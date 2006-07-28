#!/usr/bin/python
#
# Generates a series of tests from a simple test description language.

import sys
import re
import os.path
import glob
import popen2

depth=0
curline=0
curfilename=""
gen_build_type=0
gen_output_type=0
test_is_inline=0
inline_embed=0
preprocess=0
fixtures = []
tests = []
filenames = []
includes = []
libs = []
lib_paths = []
repl_inc_map={}
repl_tokens=[]
is_unix = re.search(r"linux|bsd|sunos|aix", sys.platform) != None

fixture_re =      re.compile(r"UT_FIXTURE\((?P<name>(\w)+)\)")
fixture_end_re =  re.compile(r"UT_FIXTURE_END")
test_re    =      re.compile(r"UT_TEST\((?P<name>(\w+))\)")
test_end_re =     re.compile(r"UT_TEST_END")

setup_fixture_re =         re.compile(r"UT_SETUP_FIXTURE")
teardown_fixture_re =      re.compile(r"UT_TEARDOWN_FIXTURE")
setup_fixture_end_re =     re.compile(r"UT_SETUP_FIXTURE_END")
teardown_fixture_end_re =  re.compile(r"UT_TEARDOWN_FIXTURE_END")

header_re =       re.compile("UT_INCLUDE\((?P<filename>([^)]+))\)")
def_header_re =   re.compile("UT_DEFAULT_INCLUDE\((?P<filename>([^)]+))\)")
src_include_re =  re.compile("UT_SOURCE_INCLUDE\((?P<filename>([^)]+))\)")

include_path_re = re.compile("UT_INCLUDE_PATH\((?P<path>([^)]+))\)")
replace_include_re = re.compile("UT_REPLACE_INCLUDE")
replace_token_re = re.compile("UT_REPLACE_TOKEN")

def_include_re = re.compile(r"#include\s*<\s*(?P<name>[^>]+)>")
include_re = re.compile(r'#include\s*\"\s*(?P<name>[^"]+)\"')

lib_find_version_re = re.compile(r"[0-9]+\.[0-9]+")


# Matches inline stuff like class myObject /* %startUnitTest*/ or class myObject // %startUnitTest
inline_fixture_re = re.compile(r"class (?P<name>[a-zA-Z0-9_]+)[^%]*%startFixture")
# Matches inline %endUnitTest
inline_fixture_end_re = re.compile(r"%endFixture")
# Matches inline %test(test_name)
inline_test_start_re = re.compile(r"%test\((?P<name>(\w+))\)")
# Matches inline start embed.
inline_embed_start_re = re.compile(r"%{")
# Matches inline end embed.
inline_embed_end_re = re.compile(r"%}")
# Inline setup
inline_setup_fixture_re = re.compile("%setup")
# Inline teardown
inline_teardown_fixture_re = re.compile("%teardown")

class LineCountingFile:
   def __init__(self):
      self.curline=0;
      self.file=None
      self.filename=None
   
   def open(self, name, mode):
      self.file = open(name, mode)
      self.filename=name
      
   def close(self):
      self.file.close()
      
   def write(self, msg):
      for c in msg:
         if c=='\n': self.curline+=1
         
      self.file.write(msg) 
      
class MutableFileStore:
   def __init__(self):
      self.curline=-1;
      self.lines = []
      
   def setLines(self, _lines):
      self.lines = _lines
      
   def replaceRe(self, match_re, replacement):
      for i in range(self.curline, len(self.lines)):           
         self.lines[i] =  match_re.sub(replacement, self.lines[i])
         
         
   def readline(self):     
      self.curline+=1
      if (self.curline>=len(self.lines)): return ""
      return self.lines[self.curline]
      
   def eof(self):
      if self.curline>=len(self.lines): return 1
      return 0

def print_error(lineno, msg):
   print "error:%d:%s" % (lineno, msg)
   
# Parses out some group of characters, obeying escapes.
def parse_group(line, start_pos, start_grp, end_grp):
   started=0
   escaped=0
   group=""
   i=0
   for i in range(start_pos, len(line)-1):      
      if escaped: 
         if started: group+=line[i]
         escaped=0
         continue
      
      if started==0:
         if line[i] == start_grp:            
            started=1
            
      else:
         if line[i] == end_grp:           
            return (group, i)
         else:
            group+=line[i]
            
   return (group, i)       
   
   
def check_errors():
   "Check to see if there are syntax errors."
   global depth, curline
   
   if depth<0: print_error(curline, "Missing a '{'")
   
def check_depth(line):
   "Assumes a stripped line, checks the depth."
   global depth, curline
   
   if len(line) == 0: return
   
   if line[0] == '{': depth+=1
   if line[0] == '}': depth-=1
   
def find_versioned_package(item, search):
   "Finds a package with version information."
   global includes
   
   found=0
   
   for path in search:
      results = glob.glob(os.path.join(path, item))
      if len(results)==0:
         continue
      else:         
         newest_package = ""
         version = 0.0
         for sample in results:
            # On some platforms, these files will exist. Ignore them for includes.
            if os.path.splitext(sample)[1] ==".lib": continue
           
            found=1 
            parts = sample.split("-")
            match =  lib_find_version_re.search(parts[len(parts)-1])
            if match!=None:
               this_version = float(match.group())
               if this_version > version:
                  version = this_version
                  newest_package = sample
         
         sub_include = os.path.join(newest_package, "include")
         includes.append(newest_package) 
         
         # Some packages have another include directory below them.
         if os.path.exists(sub_include):
            includes.append(sub_include) 
            
            
  
   if found==0:
      print "unittest warning: Could not find package '%s' in [%s]" % (item, ", ".join(search))


def find_package(item, search):
   "Finds a package without version information."
   global includes
   
   found=0
   
   for path in search:
      results = glob.glob(os.path.join(path, item))
      if len(results)==0:
         continue
      else:         
         for sample in results:
            # On some platforms, these files will exist. Ignore them for includes.
            if os.path.splitext(sample)[1] ==".lib": continue
            
            found=1         
            newest_package = sample
            sub_include = os.path.join(newest_package, "include")
            includes.append(newest_package) 
            
            # Some packages have another include directory below them.
            if os.path.exists(sub_include):
               includes.append(sub_include)                   
            
  
   if found==0:
      print "unittest warning: Could not find package '%s'" % item
      
def find_versioned_lib(item, search):
   "Finds a library with version information."
   global includes
   
   found=0
   
   for path in search:
      results = glob.glob(os.path.join(path, "lib"+item+".so"))
      results += glob.glob(os.path.join(path, "lib"+item+".dll"))
      results += glob.glob(os.path.join(path, "lib"+item+".a"))      
      if len(results)==0:
         continue
      else:
         found=1
         newest_package = ""
         version = 0.0
         for sample in results:
            parts = sample.split("-")
            match =  lib_find_version_re.search(parts[len(parts)-1])
            if match!=None:
               this_version = float(match.group())
               if this_version > version:
                  version = this_version
                  tmp = os.path.split(sample)[1][3:] 
                  newest_package = os.path.splitext(tmp)[0]       
               
         libs.append(newest_package) 
         
   if found==0:
      print "unittest warning: Could not find library '%s' in [%s] " % (item, ", ".join(search))
        
def find_lib(item, search):
   "Finds a library without version information."
   global libs
   
   found=0
   
   for path in search:
      results = glob.glob(os.path.join(path, "lib"+item+".so"))
      results += glob.glob(os.path.join(path, "lib"+item+".dll"))
      results += glob.glob(os.path.join(path, "lib"+item+".a"))      
      if len(results)==0:
         continue
      else:
         found=1         
         if len(results):
            tmp = os.path.split(results[0])[1][3:]
            tmp = os.path.splitext(tmp)[0]           
            libs.append(tmp)           
  
   if found==0:
      print "unittest warning: Could not find library '%s'" % item
 
   
def find_gtk():
   "Finds out what GTK packages are installed, and where."
   global libs, lib_paths
               
   # These are places we will search for the files
   if is_unix:
      search = ["/usr/lib/",
                "/usr/include", 
                "/usr/local/include"]
   else: # windows: msys
      base_path=os.environ["GTK_BASEPATH"]
      search = [os.path.normpath(os.path.join(base_path, "lib/")),
                os.path.normpath(os.path.join(base_path, "include/"))]                      
             
   # These are the directories we will search for.
   targets = ["atk-*",
              "gtk-*",
              #"gtkmm-*",
              #"gdkmm-*",
              "glib-*",
              #"glibmm-*",
              "pango-*",
              #"pangomm-*",
              #"sigc++-*"
              ]
                 
   for item in targets:
      find_versioned_package(item, search)
     
   # These are other directories that we search for, but need no version. 
   targets = ["cairo"]
   
   for item in targets:
      find_package(item, search)
      
   ##### Libraries #####
   if is_unix:
      search = ["/usr/lib/", "/usr/local/lib"]
   else: # windows: mingw
      base_path=os.environ["GTK_BASEPATH"]
      search = [os.path.join(base_path, "lib/")]      
   
   for item in search: lib_paths.append(item)
      
               
   if is_unix: 
      gtk_libs = ["gtk-x11-*",
                  "gdk-x11-*",
                  "atk-*",
                  "gdk_pixbuf-*",               
                  "pangocairo-*",               
                  "pango-*",              
                  "gobject-*",
                  "gmodule-*",               
                  "glib-*" ]
      
   else: # windows: mingw   
      gtk_libs = ["gtk-win32-*",
                  "gdk-win32-*",
                  "atk-*",
                  "gdk_pixbuf-*",
                  "pangowin32-*",
                  "pango-*",              
                  "gobject-*",
                  "gmodule-*",               
                  "glib-*" ] 
               
   for item in gtk_libs:
      find_versioned_lib(item, search)  
      
   # Now for the "other" libraries 
   if is_unix:
      gtk_libs = ["fontconfig",
               "Xext",
               "Xrender",
               "Xinerama",
               "Xi",
               "Xrandr",
               "Xcursor",
               "Xfixes",
               "m",
               "cairo", 
               "X11",
               "dl"]
               
   else: #windows: mingw
      gtk_libs = ["fontconfig*",
                  "art_lgpl_*",
                  "freetype*",
                  "iconv*",
                  "intl*",
                  "cairo*",
                  "png*",
                  "jpeg*",
                  "bz2*"]      
               
   for item in gtk_libs:
     find_lib(item, search)
         
           
   
def gen_fixture(name, outf):
   # Special support for Gtk output
   if gen_output_type==3:
      outf.write("#include <gtk/gtk.h>\n#include <gtk/gtkmain.h>\n")
   
   # The rest of the fixture.   
   outf.write("#include <unittest/assertions.h>\n")
   outf.write("#include <unittest/suite.h>\n\n")   
   outf.write("class test_%s : public unittest::Fixture\n" % name)
   outf.write("{\n");
   outf.write("\tunsigned int __total_tests__, __current_test__;\n")
   outf.write("public:\n\n")
   outf.write('\ttest_%s():Fixture("%s") {}\n' % (name, name));
   outf.write("\t~test_%s() {}\n" %name);
   outf.write("\n");
   
def gen_fixture_end(name, outf):
   global suite_f, fixtures, tests, gen_output_type
   
   outf.write("\tvirtual void runTests()\n")
   outf.write("\t{\n")
   
   # Save the number of tests in this fixture.
   outf.write("\t\t__total_tests__ = %d;\n" % len(tests))
   outf.write("\t\t__current_test__ = 0;\n")
   
   outf.write("\t\tsetup();\n\n")
   
   for testname in tests:         
      # Call test
      outf.write("\t\ttest_%s();\n" % testname)
      outf.write("\t\t++__current_test__;\n")
      
      
   outf.write("\n\t\tteardown();\n")
   outf.write("\t}\n\n")
   
   outf.write("};\n\n")
   outf.write("static test_%s the_fixture;\n\n" % name)
   outf.write("void registerFixture_%s()\n" % name)
   outf.write("{\n")
   outf.write("\tunittest::getSuite().registerFixture(&the_fixture);\n")
   outf.write("}\n\n")
   
   fixtures.append(name)      
   
def gen_test_main(fix_name, test_name, outf):   
   global tests
   
   outf.write("\tvoid test_%s()\n" % test_name);
   outf.write("\t{\n");
   outf.write("\t\tunittest::Event __event__;\n")
   outf.write('\t\t#define __TEST_NAME__ "%s::%s";\n\n' % (fix_name, test_name))
   
   tests.append(test_name)
   
def gen_test_end(name, outf):
   outf.write('\t\t#undef __TEST_NAME__\n')
   outf.write("\t}\n\n");
   
def gen_header(filename, defheader, outf):
   if defheader!=0: outf.write('#include <%s>\n' % filename)
   else:           outf.write('#include "%s"\n' % filename)
   
def gen_setup_teardown(func_name, outf):
   outf.write("\tvirtual void %s()\n" % func_name)
   outf.write("\t{\n")
   
def gen_setup_teardown_end(outf):
   outf.write("\t}\n")
   
def scan_in_setup_teardown_fixture(fixture_name, func_name, inf, outf):
   "Scans for valid macros in a setup or teardown."
   
   global curline, cur_filename, test_is_inline
         
   while not inf.eof(): 
      line=inf.readline()
      curline+=1;
            
      line=line.strip()
      
      check_depth(line)    
      check_errors()    
      
      if test_is_inline==1:
         if inline_embed_start_re.search(line)!=None: continue 
         
         if inline_embed_end_re.search(line)!=None: 
            gen_setup_teardown_end(outf)  
            return;
      
      if setup_fixture_end_re.match(line)!=None: 
         gen_setup_teardown_end(outf)
         if func_name=="teardown":
            print_error(curline, "UT_TEARDOWN_FIXTURE_END in UT_SETUP_FIXTURE")
         return
         
      if teardown_fixture_end_re.match(line)!=None: 
         gen_setup_teardown_end(outf)
         if func_name=="setup":
            print_error(curline, "UT_SETUP_FIXTURE_END in UT_TEARDOWN_FIXTURE")
         return
         
            
      
         
      # Just shovel the code into the output
      
      outf.write('#line %d "%s"\n' % (curline, curfilename))
      outf.write("\t\t%s\n" % line);      
      outf.write('#line %d "%s"\n' % (outf.curline+2, outf.filename))
      
      
      
   print_error(curline, "End of file without UT_%s_FIXTURE_END" % func_name.upper())   

         
def scan_in_test(fixture_name, test_name, inf, outf):
   "Scans for valid macros in a test."
   
   global curline
         
   while not inf.eof(): 
      line=inf.readline()
      curline+=1;
            
      line=line.strip()
      
      check_depth(line)    
      check_errors()    
      
      if test_is_inline:
         if inline_embed_start_re.search(line)!=None: continue 
      
      if test_end_re.match(line)!=None or inline_embed_end_re.search(line)!=None: 
         gen_test_end(test_name, outf)
         return
         
      # Just shovel the code into the output
      outf.write('#line %d "%s"\n' % (curline, curfilename))
      outf.write("\t\t%s\n" % line);      
      outf.write('#line %d "%s"\n' % (outf.curline+2, outf.filename))      
      
      
   print_error(curline, "End of file without UT_TEST_END")  
   
   
def scan_in_fixture(fixture_name, inf, outf):
   "Scans for valid macros in a fixture."
   
   global curline, tests, test_is_inline, inline_embed
   
   tests = []
   
   while not inf.eof(): 
      line=inf.readline()
      curline+=1;
            
      line=line.strip()
      
      check_depth(line)    
      check_errors()    
      
      if test_is_inline:
         r = inline_test_start_re.search(line)
         if r != None:
            tname = r.group('name')       
            gen_test_main(fixture_name, tname, outf)
            scan_in_test(fixture_name, tname, inf, outf)
            continue
            
         r = inline_setup_fixture_re.search(line)
         if r != None:        
            gen_setup_teardown("setup", outf)
            scan_in_setup_teardown_fixture(fixture_name, "setup", inf, outf)
            continue
            
         r = inline_teardown_fixture_re.search(line)
         if r != None:        
            gen_setup_teardown("teardown", outf)
            scan_in_setup_teardown_fixture(fixture_name, "teardown", inf, outf)                          
            continue    
            
         r = inline_embed_start_re.search(line)
         if r!=None: 
            inline_embed=1
            continue
            
         r = inline_embed_end_re.search(line)
         if r!=None: 
            inline_embed=0
            continue          
      
      else:    
         r = test_re.search(line)
         if r != None:
            tname = r.group('name')       
            gen_test_main(fixture_name, tname, outf)
            scan_in_test(fixture_name, tname, inf, outf)
            continue
            
         r = setup_fixture_re.search(line)
         if r != None:        
            gen_setup_teardown("setup", outf)
            scan_in_setup_teardown_fixture(fixture_name, "setup", inf, outf)
            continue
            
         r = teardown_fixture_re.search(line)
         if r != None:        
            gen_setup_teardown("teardown", outf)
            scan_in_setup_teardown_fixture(fixture_name, "teardown", inf, outf)                          
            continue    
            
      if fixture_end_re.search(line)!=None or inline_fixture_end_re.search(line)!=None:
         gen_fixture_end(fixture_name, outf)
         return      
               
      
      # Just shovel the code into the output
      if test_is_inline==0 or inline_embed==1:
         outf.write('#line %d "%s"\n' % (curline, curfilename))
         outf.write("\t\t%s\n" % line);      
         outf.write('#line %d "%s"\n' % (outf.curline+2, outf.filename))
      
      
   print_error(curline, "End of file without UT_FIXTURE_END")
   
   

def gen_test(inf, outf):
   "Generates the test code for a test."
   global depth, curline, includes, test_is_inline, inline_embed
   
   while not inf.eof():
      line=inf.readline()
      curline+=1;
                  
      line=line.strip() 
   
      check_errors()    
   
      if test_is_inline:            
         r = inline_embed_start_re.search(line)
         if r!=None: 
            inline_embed=1
            continue
            
         r = inline_embed_end_re.search(line)
         if r!=None: 
            inline_embed=0
            continue 
      
      # Check for a fixture declaration
      r = fixture_re.search(line)
      if r != None:
         gen_fixture(r.group('name'), outf)
         scan_in_fixture(r.group('name'), inf, outf)
         continue
         
      # Check for an inline fixture declaration
      r = inline_fixture_re.search(line)
      if r != None:
         test_is_inline=1
         gen_fixture(r.group('name'), outf)
         scan_in_fixture(r.group('name'), inf, outf)
         continue
         
      # Check for a header declaration
      r = header_re.search(line)
      if r!=None:
         gen_header(r.group('filename'), 0, outf)
         continue
         
      # Check for a default header declaration
      r = def_header_re.search(line)
      if r!=None:
         gen_header(r.group('filename'), 1, outf)
         continue
         
      # Check for a default header path declaration
      r = include_path_re.search(line)
      if r!=None:
         includes.append(r.group('path'))
         continue    
         
      # Check for a source include path declaration
      r = src_include_re.search(line)
      if r!=None:
         filenames.append(r.group('filename'))
         continue    
         
      # Just shovel the code into the output
      if inline_embed==1:
         outf.write('#line %d "%s"\n' % (curline, curfilename))
         outf.write("\t\t%s\n" % line);      
         outf.write('#line %d "%s"\n' % (outf.curline+2, outf.filename))
      
         
      
      
   if depth!=0: print_error(curline, "Missing a right brace '}'")

def gen_outputter():
   global gen_output_type, suite_f
   
   if gen_output_type==0:
      suite_f.write("CompilerOutputter");
   elif gen_output_type==1:
      suite_f.write("XmlOutputter");
   elif gen_output_type==2:
      suite_f.write("HtmlOutputter");
   elif gen_output_type==3:
      suite_f.write("GtkOutputter");
      
def gen_outputter_include():
   global gen_output_type, suite_f
   
   if gen_output_type==0:
      suite_f.write("compiler_outputter.h");
   elif gen_output_type==1:
      suite_f.write("xml_outputter.h");
   elif gen_output_type==2:
      suite_f.write("html_outputter.h");
   elif gen_output_type==3:
      suite_f.write("gtk_outputter.h");


   
def gen_suite():
   global suite_f
   
   suite_f = open("test_suite.cpp", "w");
   
   suite_f.write("#include <unittest/suite.h>\n")
   suite_f.write("#include <unittest/"); gen_outputter_include(); suite_f.write(">\n\n")
   
   suite_f.write("unittest::"); gen_outputter(); suite_f.write(" outputter;\n")
   suite_f.write("unittest::Suite test_suite(outputter);\n")
   
   suite_f.write("unittest::Suite& unittest::getSuite() { return test_suite; }\n\n")
      
   
def gen_suite_end():
   global suite_f, fixtures
   
   # Write out externs for fixtures.
   for f in fixtures:
      suite_f.write("extern void registerFixture_%s();\n" % f)
   
   # Create fixture registration function.
   suite_f.write("static void\nregister_fixtures()\n{\n");
   
   for f in fixtures:
      suite_f.write("\tregisterFixture_%s();\n" % f)
   
   suite_f.write("}\n\n")
   
   # Main function
   suite_f.write("int\nmain(int argc, char *argv[])\n")
   suite_f.write("{\n")
   
   # Initialize any GUI stuff
   if gen_output_type==3: #gtk
      suite_f.write("\tgtk_init(&argc, &argv);\n")
      
   suite_f.write("\tregister_fixtures();\n")
   suite_f.write("\tunittest::getSuite().runTests();\n")
   
   # Some GUIs require a loop at the end
   if gen_output_type==3: #gtk
      suite_f.write("\tgtk_main();\n")
   
   suite_f.write("}\n\n")
   
   suite_f.close()
   
def gen_include_paths(bf):
   global includes
   
   for i in includes:
      bf.write('-I%s ' % os.path.normpath(i))
   
      
def gen_libraries(bf):
   global libs
   
   for l in libs:
      bf.write('-l%s ' % os.path.normpath(l))

def gen_library_paths(bf):
   global lib_paths
   
   for l in lib_paths:
      bf.write('-L%s ' % os.path.normpath(l))
      
      
def gen_gcc_filenames(bf):
   global filenames
   
   for f in filenames:
      bf.write("%s " % f)

def gen_buildfile_filenames(bf):
   global filenames
   
   bf.write("sources = [\n")
   
   for f in filenames:
      bf.write("\t'%s',\n" % f)
      
   bf.write("]\n\n")


def gen_build_file():
   global gen_build_type, gen_output_type
   
   if gen_output_type==3:
      find_gtk()
   
   if gen_build_type==0:
      bf = open("build_tests.bat", "w")
      bf.write("@echo off\n")
      bf.write('set TEST_INCLUDES=')
      gen_include_paths(bf)
      bf.write('\n');
      
      bf.write("set TEST_LIBRARIES=")
      gen_libraries(bf)
      bf.write("\n");
      
      bf.write("set TEST_LIB_PATHS=")
      gen_library_paths(bf)
      bf.write("\n");
      
      bf.write("@g++ -mms-bitfields %TEST_INCLUDES% %TEST_LIB_PATHS% ")      
      gen_gcc_filenames(bf)
      bf.write(" %TEST_LIBRARIES% -o run_tests.exe\n")
      bf.close()
      
   elif gen_build_type==1:
      bf = open("build_tests.sh", "w") 
      bf.write('TEST_INCLUDES="')   
      gen_include_paths(bf)      
      bf.write('"\n')
      
      bf.write('TEST_LIBRARIES="')   
      gen_libraries(bf)      
      bf.write('"\n')
      
      bf.write("g++ -mms-bitfields $TEST_INCLUDES ")
      
      gen_gcc_filenames(bf)
      bf.write(" $TEST_LIBRARIES -o run_tests\n")
      bf.close()  
      
   elif gen_build_type==2:
      local_includes=0
      
      bf = open("Buildfile", "w")      
      bf.write("#!/usr/bin/python\n\n")
      bf.write("import build\n\n")
      
      if os.path.exists("includes.local"): 
         local_includes=1
         bf.write('build.include("includes.local")\n')      
         
      bf.write('build.include("includes.test")\n\n')
      bf.write('target="run_tests"\n\n')
      gen_buildfile_filenames(bf)
      bf.write('compile_cmd="g++ $(TEST_CFLAGS) -c $(IN) -o $(OUT)"\n')       
      bf.write('link_cmd   ="g++ $(TEST_LIBRARIES) $(IN) -o %s" % target\n')
      bf.write('other_build_cmds = []\n')
      bf.write('clean_cmds =["build/*.o", target]\n')
      bf.write('install_cmds = {}\n')
      bf.write('\nbuild.Build2(compile_cmd, sources, link_cmd, other_build_cmds, install_cmds, clean_cmds)\n\n')
      
      bf.close()
      
      bf = open("includes.test", "w")
      bf.write("TEST_LIBRARIES=")
      gen_libraries(bf)
      bf.write("\nTEST_CFLAGS=")
      if local_includes: bf.write("$(CFLAGS) ")
      gen_include_paths(bf)
      bf.write("\n")
      bf.close()
      
def find_include(filename, searchpath=1):
   "Returns the actual path of an include file."
   global includes, repl_inc_map
   
   # Perform include file replacement.
   for key in repl_inc_map.keys():
      pattern, repl = repl_inc_map[key]
      r=pattern.search(filename)
      if r!=None:
         #print key, ": ", filename, " -> ", repl
         filename = pattern.sub(repl, filename)
         break       
      
   if os.path.exists(filename): return (filename, 1)
   if searchpath==0: return (filename, 0)
   
   for path in includes:
      tmp = os.path.join(path, filename)
      if os.path.exists(tmp):
         return (tmp, 1)
         
   return (filename, 0)
   
def include_file(filename, output):
   print "processing: '%s'" % filename
   output.append('#line %d "%s"' % (1, filename))
   
   tmp = open(filename, "r")
   data = tmp.read()
   tmp.close()
   
   results = preprocess_data(data, filename)
   output.extend(results.split('\n'))
   

#Runs a *very* minimalist preprocessor       
def preprocess_data(data, filename):      
   global repl_inc_map, repl_tokens, includes
   
   tmp = MutableFileStore();
   tmp.setLines(data.split('\n'));
   
   output = []
      
   while not tmp.eof():
      line = tmp.readline()      
      line=line.strip()
      
      # Check for an default header path declaration
      r = include_path_re.search(line)
      if r!=None:
         includes.append(r.group('path'))
         continue    
      
      r = def_include_re.search(line)
      if r!=None:
         # Process default include file paths.
         incfile, found=find_include(r.group('name').strip())
         if found == 0:
            print "warning: UT_REPLACE_XXX will not affect contents of '%s' (file not found)" % incfile
            output.append('#include <%s>' % incfile)
            continue
         else:          
            include_file(incfile, output)
            continue
            
      r = include_re.search(line)
      if r!=None:
         incfile, found=find_include(r.group('name').strip(), searchpath=0)
         if not os.path.exists(incfile):
            print "warning: UT_REPLACE_XXX will not affect contents of '%s' (file not found)." % incfile
            output.append('#include "%s"' % incfile)
            continue
         else:
            include_file(incfile, output)
            continue          
      
      r = replace_include_re.search(line)
      if r!=None:                
         start_pos = r.span()[1]
         search = parse_group(line, start_pos, '"', '"')
         replace = parse_group(line, search[1]+1, '"', '"')                   
         
         repl_inc_map[search[0]] = ( re.compile(search[0]), replace[0] )
         continue
         
      r = replace_token_re.search(line)
      if r!=None:                
         start_pos = r.span()[1]
         search = parse_group(line, start_pos, '"', '"')
         replace = parse_group(line, search[1]+1, '"', '"')                   
         
         repl_tokens.append(( re.compile(search[0]), replace[0] ))
         continue
      
      # Put it in the output.
      output.append(line)
      
      
   # Return the data
   return "\n".join(output)
   
# Replace tokens in the given data buffer.
def replace_tokens1(data):
   global repl_tokens
   
   for search, replace in repl_tokens:
      data = search.sub(replace, data)
      
   return data

# Replace tokens in the given mutable data source.
def replace_tokens2(inf):
   global repl_tokens
   
   for search, replace in repl_tokens:
      inf.replaceRe(search, replace)
   
def print_help():
   print "\nunittest generator (C)opyright 2006 Christopher Nelson\n"
   print "\t--build=n     where n=unix, windows, or buildfile"
   print "\t--include-path=n where n is the path. e.g. ../../include"
   print "\t--output=n       where n=compiler, xml, or html"
   print "\t--preprocess     causes the generator preprocess the input so that"
   print "\t                 UT_REPLACE_XXX can be used.  Without this flag,"
   print "\t                 that directive will cause an error." 
   print
   
   
# Version of unittest
version = "0.2";  
suite_f = None;

sys.argv.pop(0)
input_files = sys.argv

filenames.append("test_suite.cpp")

for filename in input_files:  
   if len(filename)>2:
      if filename[0:2]=="--":       
         switch = filename.split("=")        
         if len(switch)<2 and switch[0]!="--preprocess":
            print "Bad switch: ", filename, " requires value after the '='"
            print_help()
            sys.exit(0)
            
         switch[0] = switch[0].strip();
         if (len(switch)>1): switch[1] = switch[1].strip();
         
            
         if switch[0]=="--include-path":
            includes.append(switch[1])
            continue
         elif switch[0]=="--build":
            if switch[1] == "unix": gen_build_type=1
            elif switch[1] == "windows": gen_build_type=0
            elif switch[1] == "buildfile": gen_build_type=2
            else:
               print "error: Unknown build type for --build"
               print_help()
               sys.exit(0)
               
            continue
            
         elif switch[0]=="--output":
            if switch[1] == "compiler": gen_output_type=0
            elif switch[1] == "xml": gen_output_type=1
            elif switch[1] == "html": gen_output_type=2
            elif switch[1] == "gtk": gen_output_type=3
            else:
               print "error: Unknown output type for --output"
               print_help()
               sys.exit(0)
            
            continue
            
         elif switch[0]=="--preprocess":
            preprocess=1
            continue
            
   
   files = glob.glob(filename)
   if len(files)>1:
      for file in files:
         input_files.append(file)
      continue
      
   filename = files[0]
   
   curfilename=filename
   curline=0
   test_is_inline=0
   inline_embed=0
   repl_inc_map={}
   repl_tokens=[]
   
   tmp = os.path.splitext(filename)
   out_name = os.path.split(tmp[0])[1] + "_test.cpp";
   filenames.append(out_name)
   
   inf_tmp=open(filename, "r"); 
   data = inf_tmp.read()
   inf_tmp.close()
   
   # Check for replacement directives.
   r = replace_include_re.search(data)
   if r!=None:
      if preprocess==0:
         print "error: a UT_REPLACE_INCLUDE directive was found without the --preprocess switch."
         sys.exit(1)
      
   if preprocess==1: 
      cooked_data=preprocess_data(data, filename)           
   
   inf=MutableFileStore(); inf.setLines(data.split('\n'));
   outf=LineCountingFile(); outf.open(out_name, "w");    
   
   outf.write("// Test framework generated by unittest v%s\n" % version)
   outf.write("// Source file: %s\n\n" % filename)
   outf.write("#define _UNIT_TEST_ 1\n")
      
   # If we preprocessed the stuff, then write it into the fixture's output file.
   if preprocess==1:
      cooked_data = replace_tokens1(cooked_data)
      outf.write(cooked_data)
      outf.write("\n\n")   
      outf.write('#line %d "%s"\n' % (outf.curline+2, outf.filename))      
   
   # If it's an include file, then we'll need to include it!
   if preprocess==0 and tmp[1] == ".h":
      gen_header(curfilename, 0, outf)
         
   # Make sure the replace tokens applies to our input as well.
   replace_tokens2(inf)
      
   gen_test(inf,outf)
         
   outf.close()
         
gen_suite()
gen_suite_end()   
gen_build_file()

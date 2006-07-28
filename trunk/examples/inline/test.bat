@echo off
del run_tests.exe
python ../../scripts/unittest.py --build=windows --output=gtk FixedPoint.h 
call build_tests.bat
run_tests.exe
rem ..\bin\doxygen Doxyfile
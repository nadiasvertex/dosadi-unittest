#ifndef __TEST_ASSERTIONS_H__
#define __TEST_ASSERTIONS_H__

/** \mainpage unittest Testing Framework Source Documentation
 *  \section intro_sec Introduction
 *    
 *   unittest is a C++ Automated Unit Test framework.  It is still in it's 
 * infancy, but is very useable.  It lacks some bells and whistles (like
 * a graphical interface), but is reasonably feature complete. There is
 * a short tutorial at http://dosadi.bbhc.org/darcs/unittest/docs/tutorial/index.html
 *
 *  \section usage_sec Usage
 *
 *  The tutorial covers installation and usage.  It is important to note that this
 * framework was designed to be used with a preprocessing python script.  It allows
 * a brevity of syntax that is otherwise impossible to obtain.
 *
 *  For a list of directives available in preprocessing, please see the \ref Directives section.
 *  For a list of assertions available for testing, please see the \ref Assertions section.
 *
 * \section support_sec Support
 *
 *  This framework was developed by Christopher Nelson.  If someone is interested in commercial support,
 * contact me and we can discuss terms.  If you just need help real quick, feel free to drop me an e-mail.
 * The same is true for anyone wishing to submit a patch (darcs patches are most welcome).  
 *
 *  Please write to: nadiasvertex at gmail dot com, and mention that you have questions or patches for the
 * unittest framework.  Please also realize that it may take me some time to respond, depending on how busy
 * I am.
 *
 */
 
/**  
 * \example simple.test
 * This is a simple example test that uses a number of directives and assertions.
 */


/**
 * \example simplest.test
 * This is the simplest possible test that can be developed. */


#include <cmath>
#include <cstdlib>

/** \defgroup Assertions 
    @brief Assertions that can be used in tests.
 * @{
 *
 */

#ifdef UT_DONT_CATCH

#define UT_EVALUATE_EXPR(expr, exception_expected) expr
#define UT_EVALUATE_EXPR_NORESULT(expr, exception_expected) expr, true

#else

#include <exception>

#define UT_EVENT_BODY(passed, the_expr) 		\
	__event__.pass = passed;  	 			    \
 	__event__.expr = #the_expr;	    			\
 	__event__.filename = __FILE__ "::" __TEST_NAME__;	    \
 	__event__.line_num = __LINE__; \
 	__event__.total_tests =  __total_tests__; \
 	__event__.current_test = __current_test__;			    
 	

#ifdef UT_NO_RTTI

/** Evaluates an expression, and evaluates the result.  Will
 * fail if an exception is expected and none is thrown.  Will also
 * fail if an exception is thrown where none is expected. Will also
 * fail if the expression fails. */
#define UT_EVALUATE_EXPR(the_expr, exception_expected) \
({ 												\
  bool _ut_expr_result_; 									\
  			   									\
  try 		   									\
  {												\
	  _ut_expr_result_ = (the_expr); 						\
	  	   										\
	if (exception_expected)					    \
	{											\
	  UT_EVENT_BODY(false, the_expr);						\
      __event__.msg  =  "Exception expected, but none recieved."; \
 												\
 	  listener->OnEvent(__event__);				\
	} 											\
  }												\
  catch(std::exception &e)						\
  {												\
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  std::string("Standard Exception: ") + e.what(); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  "Unknown exception.";     \
 												\
   listener->OnEvent(__event__);				\
  }						    					\
  _ut_expr_result_; })

/** Evaluates an expression, but does not expect a result.  Will
 * fail if an exception is expected and none is thrown.  Will also
 * fail if an exception is thrown where none is expected. */    
#define UT_EVALUATE_EXPR_NORESULT(the_expr, exception_expected) \
({ 												\
  bool _ut_expr_result_; 									\
  			   									\
  try 		   									\
  {												\
	  _ut_expr_result_ = true;							\
	  the_expr; 								\
	  	   										\
	if (exception_expected)					    \
	{											\
	  UT_EVENT_BODY(false, the_expr);						\
      __event__.msg  =  "Exception expected, but none recieved."; \
 												\
 	  listener->OnEvent(__event__);				\
	} 											\
  }												\
  catch(std::exception &e)						\
  {												\
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  std::string("Standard Exception: ") + e.what(); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  "Unknown exception.";     \
 												\
   listener->OnEvent(__event__);				\
  }						    					\
  _ut_expr_result_; })
 
/** 
 * Expects an exception of a certain type. Passes if that
 * exception was caught, fails otherwise. */ 
#define UT_EXPECT_EXCEPTION(the_expr, the_exception_type) \
 try 											 \
 { 												 \
	 the_expr;									 \
	 UT_EVENT_BODY(false, the_expr);			 \
	 __event__.msg = "Expected an exception of type: \"" #the_exception_type "\" but none fired."; \
	 listener->OnEvent(__event__);				\
 }												\
 catch(the_exception_type &e)					\
 {	                                            \
	UT_EVENT_BODY(true, the_expr);			    \
   	__event__.msg  =  #the_exception_type " exception caught."); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(false, the_expr);			    \
    __event__.msg  = "Expected an exception of type: \"" #the_exception_type "\" but received some other exception."; \
 												\
   listener->OnEvent(__event__);				\
  }


#else

#include <typeinfo>

/** Evaluates an expression, and evaluates the result.  Will
 * fail if an exception is expected and none is thrown.  Will also
 * fail if an exception is thrown where none is expected. Will also
 * fail if the expression fails. */
#define UT_EVALUATE_EXPR(the_expr, exception_expected) \
({                                              \
  bool _ut_expr_result_;                                  \
  			                                    \
  try 		   									\
  {												\
 	_ut_expr_result_ = (the_expr); 						\
	if (exception_expected)					    \
	{											\
	 UT_EVENT_BODY(false, the_expr);						\
     __event__.msg  =  "Exception expected, but none recieved."; \
 												\
 	listener->OnEvent(__event__);				\
	} 											\
  }												\
  catch(std::exception &e)		                \
  {	                                            \
	UT_EVENT_BODY(exception_expected, the_expr);			\
   	__event__.msg  =  std::string(typeid(e).name()) +  std::string(" exception: ") + e.what(); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  "Unknown exception.";     \
 												\
   listener->OnEvent(__event__);				\
  }						    					\
  _ut_expr_result_; })

/** Evaluates an expression, but does not expect a result.  Will
 * fail if an exception is expected and none is thrown.  Will also
 * fail if an exception is thrown where none is expected. */
#define UT_EVALUATE_EXPR_NORESULT(the_expr, exception_expected) \
({                                              \
  bool _ut_expr_result_;                                  \
  			                                    \
  try 		   									\
  {												\
 	_ut_expr_result_ = true;								\
 	the_expr; 									\
	if (exception_expected)					    \
	{											\
	 UT_EVENT_BODY(false, the_expr);						\
     __event__.msg  =  "Exception expected, but none recieved."; \
 												\
 	listener->OnEvent(__event__);				\
	} 											\
  }												\
  catch(std::exception &e)		                \
  {	                                            \
	UT_EVENT_BODY(exception_expected, the_expr);			\
   	__event__.msg  =  std::string(typeid(e).name()) +  std::string(" exception: ") + e.what(); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(exception_expected, the_expr);			\
    __event__.msg  =  "Unknown exception.";     \
 												\
   listener->OnEvent(__event__);				\
  }						    					\
  _ut_expr_result_; })

/**  
 * Expects an exception of a certain type. Passes if that
 * exception was caught, fails otherwise. */ 
#define UT_EXPECT_EXCEPTION(the_expr, the_exception_type) \
 try 											 \
 { 												 \
	 the_expr;									 \
	 UT_EVENT_BODY(false, the_expr);			 \
	 __event__.msg = "Expected an exception of type: \"" #the_exception_type "\" but none fired."; \
	 listener->OnEvent(__event__);				\
 }												\
 catch(the_exception_type &e)					\
 {	                                            \
	UT_EVENT_BODY(true, the_expr);			    \
   	__event__.msg  =  std::string(typeid(e).name()) +  std::string(" exception caught."); \
 												\
 	listener->OnEvent(__event__);				\
  }												\
  catch(...)									\
  {											    \
	UT_EVENT_BODY(false, the_expr);			    \
    __event__.msg  = "Expected an exception of type: \"" #the_exception_type "\" but received some other exception."; \
 												\
   listener->OnEvent(__event__);				\
  }

#endif // end if don't use RTTI

#endif // end if catch exceptions

/**
 * \def UT_ASSERT_MSG
 * @brief Assert is true if expr1 is true.  msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_MSG(expr1, _msg) 	\
	__event__.pass = (UT_EVALUATE_EXPR(((expr1) == true), false)); \
 	__event__.expr = #expr1 " is true";	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);


/**
 *
 * \def UT_ASSERT
 *  @brief Assert is true if expr1 is true. */
#define UT_ASSERT(expr1) UT_ASSERT_MSG(expr1, "")

/**
 * \def UT_ASSERT_FALSE_MSG
 * @brief Assert is true if expr1 is <i>false</i>.  msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_FALSE_MSG(expr1, _msg) 	\
	__event__.pass = (UT_EVALUATE_EXPR(((expr1) == false), false)); \
 	__event__.expr = #expr1 " is true";	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);
 	
/**
 *
 * \def UT_ASSERT_FALSE
 * @brief Assert is true if expr1 is <i>false</i>. */
#define UT_ASSERT_FALSE(expr1) UT_ASSERT_FALSE_MSG(expr1, "") 	
 

/**
 * \def UT_ASSERT_EQUALS_MSG
 * @brief Assert is true if expr1 and expr2 equal each other.  msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_EQUALS_MSG(expr1, expr2, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR(((expr1) == (expr2)), false); \
 	__event__.expr = #expr1 "==" #expr2;	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/**  
 * 
 * \def UT_ASSERT_EQUALS
 * @brief Assert true if expr1 and expr2 equal each other. */
#define UT_ASSERT_EQUALS(expr1, expr2) UT_ASSERT_EQUALS_MSG(expr1, expr2, "") 	

/** 
 * \def UT_ASSERT_LESS_MSG
 * @brief Assert is true if expr1 is less than expr2.  msg is 
 * additional information you would like printed in the error if it fails. */
#define UT_ASSERT_LESS_MSG(expr1, expr2, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR((expr1) < (expr2), false); \
 	__event__.expr = #expr1 " < " #expr2;	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 * \def UT_ASSERT_LESS
 * @brief Assert true if expr1 is less than expr2. */
#define UT_ASSERT_LESS(expr1, expr2) UT_ASSERT_LESS_MSG(expr1, expr2, "") 	


/** 
 * \def UT_ASSERT_GREATER_MSG
 * @brief Assert is true if expr1 is greater than expr2.  msg is 
 * additional information you would like printed in the error if it fails. */
#define UT_ASSERT_GREATER_MSG(expr1, expr2, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR((expr1) > (expr2), false); \
 	__event__.expr = #expr1 " > " #expr2;	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/**  
 * \def UT_ASSERT_GREATER
 * @brief Assert true if expr1 is greater than expr2. */
#define UT_ASSERT_GREATER(expr1, expr2) UT_ASSERT_GREATER_MSG(expr1, expr2, "") 	

/**
 * \def UT_ASSERT_FLOAT_NEAR_MSG
 * @param expr1 The left side of the expression.
 * @param expr2 The right side of the expression.
 * @param epsilon The value that indicates how "near" expr1 and expr2 must be in order to pass.
 * @param _msg The message to include in the test event.
 *
 * @brief Assert true if floating point expr1 and expr2 are near each other by some value of epsilon. Can handle
 * floating point formats up to double. msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_FLOAT_NEAR_MSG(expr1, expr2, epsilon, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR((std::fabs((expr1)-(expr2))<(epsilon)) , false); \
 	__event__.expr = #expr1 " near " #expr2 " by " #epsilon;	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 * \def UT_ASSERT_FLOAT_NEAR
 * @param expr1 The left side of the expression.
 * @param expr2 The right side of the expression.
 * @param epsilon The value that indicates how "near" expr1 and expr2 must be in order to pass.
 *
 * @brief Assert true if floating point expr1 and expr2 are near each other by some value of epsilon.
 * Can handle floating point formats up to double.  */
#define UT_ASSERT_FLOAT_NEAR(expr1, expr2, epsilon) UT_ASSERT_FLOAT_NEAR_MSG(expr1, expr2, epsilon, "") 

/** 
 * \def UT_ASSERT_NEAR_MSG
 * @param expr1 The left side of the expression.
 * @param expr2 The right side of the expression.
 * @param epsilon The value that indicates how "near" expr1 and expr2 must be in order to pass.
 * @param _msg The message to include in the test event.
 *
 * @brief Assert true if integer expr1 and expr2 are near each other by some value of epsilon. Can handle
 * integer formats up to the native int size. msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_NEAR_MSG(expr1, expr2, epsilon, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR((std::abs((expr1)-(expr2))<(epsilon)) , false); \
 	__event__.expr = #expr1 " near " #expr2 " by " #epsilon;	    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 * \def UT_ASSERT_NEAR
 * @param expr1 The left side of the expression.
 * @param expr2 The right side of the expression.
 * @param epsilon The value that indicates how "near" expr1 and expr2 must be in order to pass.
 *
 * @brief Assert true if integer expr1 and expr2 are near each other by some value of epsilon.
 * Can handle integer formats up to the native int size. */
#define UT_ASSERT_NEAR(expr1, expr2, epsilon) UT_ASSERT_NEAR_MSG(expr1, expr2, epsilon, "") 	

/**
 * \def UT_ASSERT_EXCEPTION_MSG
 * @param expr1 The expression to evaluate for exceptions. The result is also considered a test and is passed or failed.
 * @param _msg  The message to include in the test event.
 *
 * @brief Assert true for the expression, and expect an exception. Will cause a
 * fail event to occur if an exception is expected and none occurs. msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_EXCEPTION_MSG(expr1, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR((expr1), true); \
 	__event__.expr = #expr1;					    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 * \def UT_ASSERT_EXCEPTION
 * @param expr1 The expression to evaluate for exceptions. The result is also considered a test and is passed or failed.
 * 
 * @brief Assert true for the expression, and expect an exception. Will cause a
 * fail event to occur if an exception is expected and none occurs. */ 	
#define UT_ASSERT_EXCEPTION(expr1) UT_ASSERT_EXCEPTION_MSG(expr1, "") 

/**
 * \def UT_ASSERT_NORESULT_EXCEPTION_MSG
 * @param expr1 The expression to evaluate for exceptions. The result is ignored.
 * @param _msg  The message to include in the test event.
 *
 * @brief Evaluate expression but do not evaluate the result, and expect an exception. 
 * Will cause a fail event to occur if an exception is expected and none occurs. msg is 
 * additional information you would like printed in the error if it
 * fails. */
#define UT_ASSERT_NORESULT_EXCEPTION_MSG(expr1, _msg) 	\
	__event__.pass = UT_EVALUATE_EXPR_NORESULT((expr1), true); \
 	__event__.expr = #expr1;					    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 *
 * \def UT_ASSERT_NORESULT_EXCEPTION 
 * @param expr1 The expression to evaluate for exceptions. The result is ignored.
 * @example simplest.test
 *
 * @brief Evaluate expression but do not evaluate the result, and expect an exception. 
 * Will cause a fail event to occur if an exception is expected and none occurs. */
#define UT_ASSERT_NORESULT_EXCEPTION(expr1) UT_ASSERT_NORESULT_EXCEPTION_MSG(expr1, "")

/**
 * \def UT_ASSERT_BUG_MSG
  * @param _msg  The message to include in the test event.
 *
 * @brief This assertion always fails.  It is used to note that a known bug exists in the code, and
 * should be remedied. */
#define UT_ASSERT_BUG_MSG(_msg) 	\
	__event__.pass = false; \
 	__event__.expr = "--BUG--";					    \
 	__event__.msg =  _msg;					    \
 	__event__.filename = __FILE__ "::" __TEST_NAME__;			    \
 	__event__.line_num = __LINE__;			    \
 												\
 	listener->OnEvent(__event__);

/** 
 * @}
 */

////////////////////////////////////////////////////////////////////////////////////////////

/** \defgroup Directives 
 *  @brief unittest.py script directives
 * @{
 *
 */

/**
 * \def UT_FIXTURE
 * \param name The name of the fixture 
 * 
 * Defines the start of a fixture.  name is the name of the fixture. 
 * The fixture will automatically be registered with the test suite. Any
 * text that is not part of a fixture will be copied straight into the 
 * resulting C++ fixture class. */
#define UT_FIXTURE(name)

/** 
 * \def UT_FIXTURE_END 
 * Defines the end of a fixture. */
#define UT_FIXTURE_END

/** 
 * \def UT_SETUP_FIXTURE 
 *
 * Defines the start of a fixture setup.  This will be run once
 * before any tests are run.  There may be only one setup per
 * fixture. This declaration may only
 * appear between UT_FIXTURE and UT_FIXTURE_END. */
#define UT_SETUP_FIXTURE

 
/** 
 * \def UT_SETUP_FIXTURE_END 
 * Defines the end of the fixture setup routine. */
#define UT_SETUP_FIXTURE_END
 

/**
 * \def UT_TEARDOWN_FIXTURE 
 * Defines the start of a fixture teardown.  This will be run once
 * after all tests are run.  There may be only one teardown per
 * fixture. This declaration may only
 * appear between UT_FIXTURE and UT_FIXTURE_END. */
#define UT_TEARDOWN_FIXTURE


/**
 * \def UT_TEARDOWN_FIXTURE_END	 
 * Defines the end of the fixture teardown routine. */
#define UT_TEARDOWN_FIXTURE_END


/** 
 * \def UT_TEST
 * @param name The name of the test. 
 *
 * Defines the start of a test.  This declaration may only
 * appear between UT_FIXTURE and UT_FIXTURE_END. Tests are
 * automatically registered with the fixture. */
#define UT_TEST(name)

/** 
 * \def UT_TEST_END 
 * Defines the end of a test. */
#define UT_TEST_END

/** 
 * \def UT_INCLUDE
 * @param filename The name of the file to be included. 
 * @see UT_DEFAULT_INCLUDE 
 *
 * Causes the filename to be included. May contain any valid path.
 * Generates a #include "filename" */
#define UT_INCLUDE(filename)

/** 
 * \def UT_DEFAULT_INCLUDE
 * @param filename The name of the file to include. 
 * @see UT_INCLUDE 
 *
 * Causes the filename to be included relative to the default include paths. 
 * May contain any valid path.
 * Generates a #include <filename> */
#define UT_DEFAULT_INCLUDE(filename)

/**
 * \def UT_INCLUDE_PATH
 * @param path The path to be added to the default include path. 
 *
 * Adds the given path to the default include path. */
#define UT_INCLUDE_PATH(path)

/**
 * \def UT_SOURCE_INCLUDE
 * @param filename A source file that should be compiled along with the test. 
 *
 *  Adds a source file that needs to be included as a dependency.  Does not parse
 * the file in any way. */
#define UT_SOURCE_INCLUDE(filename)

/** @} */

#endif

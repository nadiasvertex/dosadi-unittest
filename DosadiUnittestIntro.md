# Introduction #

This unit testing package makes writing unit tests for C++ code much simpler than other similar packages.

It also comes with extensive documentation and a tutorial.

# Details #

You can access the tutorial on http://www.dosadi.ws/docs/unittest/tutorial/

Complete documentation is available at http://www.dosadi.ws/docs/unittest/html/

A quick example of what a simple unittest looks like:

```
UT_FIXTURE(myFixture)
 UT_TEST(myTest)
 	UT_ASSERT_EQUALS(0,1);
 UT_TEST_END
UT_FIXTURE_END 
```

Between the UT\_TEST and UT\_TEST\_END markers, any valid C++ code is allowed.  For more information on features, please see the documentation above.
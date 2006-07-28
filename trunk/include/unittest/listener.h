#ifndef __TEST_LISTENER_H__
#define __TEST_LISTENER_H__

#include "config.h"
#include <string>

namespace unittest
{
	/** Contains information about test events. */
	struct Event
	{
		/** True if it passed, false if it failed. */
		bool pass;
		
		/** The message included with the event. */
		std::string msg;
		
		/** The filename where the event occurred. */
		std::string filename;
		
		/** The line number where the event occurred. */
		uint32_t line_num;
		
		/** The expression that generated the event. */
		std::string expr;		
		
		/** The total number of tests in the fixture. */
		uint32_t total_tests;
		
		/** The the test number that generated this event. */
		uint32_t current_test;		
	};
	
	struct Summary
	{
		/** The number of failing events. */
		uint32_t failures;
		
		/** The number of passing events. */
		uint32_t passes;
		
		/** The total number of events. */
		uint32_t event_count;		
	};
	
	/** Listens for test events.  Test events are essentially pass/fail
	 * with additional information about what passed and failed. */
	class Listener
	{
	protected:
		/** A simple summary of information. */
		Summary s;
		
	public:
		Listener() {}
		~Listener() {}
		
		/** Events are received here.  You may do anything you like with the event. Subclasses
		 * should call this to maintain summary information. */
		virtual void OnEvent(Event &e)
		{
			if (e.pass) ++s.passes;
			else		++s.failures;
			
			++s.event_count;
		}
		
		/** Called when the suite is about to start. */
		virtual void OnSuiteStart()
		{
			
		}
		
		/** When a fixture is starting, this is called. */
		virtual void OnFixtureStart(const std::string &name)
		{
			
		}
		
		/** When a fixture is done with all it's tests, this is called. */
		virtual void OnFixtureDone()
		{
			
		}
		
		/** When a suite is done with all it's tests, this is called. */
		virtual void OnSuiteDone()
		{
				
		}
				
		/** Returns the summary information for this listener. */
		virtual Summary getSummary() { return s; }
	};
	
};

#endif

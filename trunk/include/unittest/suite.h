#ifndef __TEST_SUITE_H__
#define __TEST_SUITE_H__

#include <vector>
#include "fixture.h"

namespace unittest
{	
	/** Collects all fixtures and runs all tests in each fixture. */
	class Suite
	{
		/** The listener for this suite. */
		Listener &listener;
		
		/** The type of container for fixtures. */
		typedef std::vector<Fixture *> fixture_list;
		
		/** The list of all fixtures. */
		fixture_list fixtures;
		
	public:
		/** Constructs a suite, requires a listener. */
		Suite(Listener &l):listener(l) {}
		
		~Suite() {}
		
		/** Run all tests from all fixtures. */
		void runTests()
		{			
			listener.OnSuiteStart();
						
			for(fixture_list::iterator pos = fixtures.begin(); pos!=fixtures.end(); ++pos)
			{
				Fixture *f = *pos;
				
				listener.OnFixtureStart(f->Name());
				
				f->setListener(&listener);
				f->runTests();
				
				listener.OnFixtureDone();
			}		
			
			listener.OnSuiteDone();				
		}		
		
		/** Register a fixture to be run. */
		void registerFixture(Fixture *f)
		{
			fixtures.push_back(f);	
		}
	};
	
	
	/** Returns the global suite. */
	extern Suite& getSuite();
	
}

#endif

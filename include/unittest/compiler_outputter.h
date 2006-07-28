#ifndef __COMPILER_OUTPUTTER_H__
#define __COMPILER_OUTPUTTER_H__

#include <vector>
#include <iostream>
#include "listener.h"

namespace unittest
{
	/** Outputs events in a way that makes it easier for an IDE to use them. */
	class CompilerOutputter : public Listener
	{
		/** The type for event lists. */
		typedef std::vector<Event> event_list;
		
		/** The list of events processed. */
		event_list events;
		
	public:
	
		/** Handles event catches. */		
		virtual void OnEvent(Event &e)
		{
			Listener::OnEvent(e);	
			
			events.push_back(e);
			
			if (e.pass) std::cout << ".";
			else		std::cout << "f";						
		}
		
		/** When a fixture is done with all it's tests, this is called. */
		virtual void OnFixtureDone()
		{
			Listener::OnFixtureDone();	
			
			std::cout << std::endl;
		}
		
		/** When a suite is done with all it's tests, this is called. */
		virtual void OnSuiteDone()
		{
			Listener::OnSuiteDone();				
			
			std::cout << std::endl << std::endl;
			
			for(event_list::iterator pos=events.begin(); pos!=events.end(); ++pos)
			{
				if (!(pos->pass))
				{
					std::cout << pos->filename.substr(0, pos->filename.find(':')) << ":" << pos->line_num << ":1: " 
						      << pos->expr << " failed";
					
					if (pos->msg.size()>0) std::cout << " : " << pos->msg; 
					
					std::cout << std::endl;
				}					 
			}
			
			std::cout << std::endl 
				 << s.failures << " failures" << std::endl
				 << s.passes   << " passes" << std::endl
				 << s.event_count << " total events" << std::endl << std::endl;			
		}
		
	};
		
}

#endif

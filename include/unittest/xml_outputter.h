#ifndef __XML_OUTPUTTER_H__
#define __XML_OUTPUTTER_H__

#include <vector>
#include <iostream>
#include "listener.h"

namespace unittest
{
	/** Outputs events in a way that makes it easier for programs to parse them. */
	class XmlOutputter : public Listener
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
					std::cout << "<error filename=\""pos->filename << "\" line=\"" << pos->line_num << "\">" << std::endl 
						      << "  <expr>" << pos->expr << "</expr>" << std::endl
						      << "  <msg>"  << pos->msg  << "</msg>" << std::endl
						      << "</error>" << std::endl;
				}					 
			}
			
			std::cout << std::endl << "<summary "
				 << "failures=\""  << s.failures << "\" " << std::endl
				 << "passes=\""    << s.passes   << "\" " << std::endl
				 << "event_count=\"" << s.event_count << "\" />" << std::endl << std::endl;			
		}
		
	};
		
}

#endif

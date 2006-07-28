#ifndef __HTML_OUTPUTTER_H__
#define __HTML_OUTPUTTER_H__

#include <vector>
#include <iostream>
#include "listener.h"

namespace unittest
{
	/** @brief Outputs events to html.
	 *
	 *  In order to display the output in the ideal way, it is important 
	 * to copy the res/html/results.css file and the res/html/img directory
	 * to the <em>same</em> folder as your results will be put into. */
	class HtmlOutputter : public Listener
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
			
			if (e.pass) std::cout << "<img src=\"img/pass.png\" />";
			else		std::cout << "<img src=\"img/fail.png\" />";
			
			if (events.size() % 20 == 0) std::cout << "<br>" << std::endl;						
		}
		
		/** Called when the fixture is about to start. */
		virtual void OnSuiteStart()
		{
			std::cout << "<html><head><title>Unit Test Results</title>" << std::endl
			          << "<link href=\"results.css\" rel=\"stylesheet\" type=\"text/css\"></head><body>" << std::endl;	
			          
			std::cout << "<h1>Unit Test Suite Results</h1>" << std::endl;
			std::cout << "<p class=\"intro\">The following contains the results of running the test suite. "
					  << "Each fixture has it's own section.  At the top of that section you will see a "
					  << "horizontal rule, followed by the name of the fixture in large, bold type. "
					  << "Below the fixture name will be a line or grid (for many tests) of images. "
					  << "Green images mean that the test passed, while red images mean that the test failed. "
					  << "For each failing test there will be a message below the line (or grid) describing "
					  << "the failure.</p>" << std::endl;
		}
		
		/** When a fixture is starting, this is called. */
		virtual void OnFixtureStart(const std::string &name)
		{
			std::cout << "<br><hr><h2>" << name << "</h2>" << std::endl;	
		}
		
		/** When a fixture is done with all it's tests, this is called. */
		virtual void OnFixtureDone()
		{
			Listener::OnFixtureDone();	
			
			for(event_list::iterator pos=events.begin(); pos!=events.end(); ++pos)
			{
				if (!(pos->pass))
				{
					std::cout << "<table width=\"100%\">" << std::endl;
					std::cout << "<tr><td width=\"25%\" class=\"fn_cell\">" << std::endl;
					std::cout << pos->filename << "</td><td width=\"5%\" class=\"info_cell\">line " << pos->line_num << "</td>" << std::endl 
						      << "<td width=\"60%\" class=\"info_cell\">" << pos->expr << "</td></tr>" << std::endl
						      << "<tr><td colspan=\"3\" valign=\"top\" class=\"msg_cell\">"  << pos->msg  << "</td></tr>" << std::endl;
						      
				    std::cout << "</table>" << std::endl;
				}					 
			}
			
			events.clear();					
		}
		
		/** When a suite is done with all it's tests, this is called. */
		virtual void OnSuiteDone()
		{
			Listener::OnSuiteDone();				
			
			std::cout << std::endl << std::endl;						
			
			std::cout << std::endl << "<br><hr><h2>Summary</h2><br>"
				 << "failures="  << s.failures << "<br>" << std::endl
				 << "passes="    << s.passes   << "<br>" << std::endl
				 << "event_count=" << s.event_count << "<br>" << std::endl << std::endl;			
				 
		    std::cout << "</body></html>" << std::endl;
		}
		
	};
		
}

#endif

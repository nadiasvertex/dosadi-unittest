#ifndef __GTK_OUTPUTTER_H__
#define __GTK_OUTPUTTER_H__

#include <vector>
#include <iostream>
#include <sstream>
#include "listener.h"

#include <gtk/gtk.h>

#ifndef _
 #define _(txt) txt
#endif



namespace unittest
{

   static gint close_window_cb(GtkWidget* w, GdkEventAny *e, gpointer data);
   static gboolean test_results_expose_cb(GtkWidget *w, GdkEventExpose *event,
                                          gpointer data);
                                          
   static gboolean pass_fail_expose_cb(GtkWidget *w, GdkEventExpose *event,
                                          gpointer data);                                          

	/** Outputs events in a way that makes it easier for an IDE to use them. */
	class GtkOutputter : public Listener
	{
		/** The type for event lists. */
		typedef std::vector<Event> event_list;
		
		/** The list of events processed. */
		event_list events;
		
		/** The number of passed events. */
		int nPassed;
		
		/** The number of failed events. */
		int nFailed;
		
	public:
	
		/** Handles event catches. */		
		virtual void OnEvent(Event &e)
		{
		   std::ostringstream tmp;
		   
			Listener::OnEvent(e);	
			
			events.push_back(e);
			
			if (e.pass)
			{			   
			   ++nPassed;
			   
			   tmp << "<span foreground=\"#008800\" weight=\"bold\">" << nPassed
			       << "</span>";
			       
			   gtk_label_set_markup(GTK_LABEL(passed), tmp.str().c_str());
			   
			   // Update the list
   			GtkTreeIter iter;
   			size_t split_pos = e.filename.find(':');
   		   
   		   gtk_list_store_append(passstore, &iter);
   		   gtk_list_store_set(passstore, &iter,
   		                      0, e.filename.substr(0, split_pos).c_str(),
   		                      1, e.filename.substr(split_pos+2, e.filename.size()).c_str(),
   		                      2, e.line_num,
   		                      3, e.expr.c_str(),   		                      
   		                      -1); 
			   
			}
			else
			{
			   ++nFailed;
			   
			   tmp  << "<span foreground=\"#880000\" weight=\"bold\">" << nFailed
			        << "</span>";
			        
			   gtk_label_set_markup(GTK_LABEL(failed), tmp.str().c_str());
			   
			   // Update the list
   			GtkTreeIter iter;
   			size_t split_pos = e.filename.find(':');
   		   
   		   gtk_list_store_append(infostore, &iter);
   		   gtk_list_store_set(infostore, &iter,
   		                      0, e.filename.substr(0, split_pos).c_str(),
   		                      1, e.filename.substr(split_pos+2, e.filename.size()).c_str(),
   		                      2, e.line_num,
   		                      3, e.expr.c_str(),
   		                      4, e.msg.c_str(),
   		                      -1);   			
			}
			
			
			// Invalidate the widget area so that it will get redrawn.			
			gtk_widget_queue_draw_area(test_output, 0,0, 
			                           test_output->allocation.width, 
			                           test_output->allocation.height);	
			                           
			                           
			// Update the progress bar
			{
   			float cur = e.current_test, total = e.total_tests;
   			
   		   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(tests_done), cur/total);
   		   gtk_progress_bar_pulse(GTK_PROGRESS_BAR(fixtures_done));
			}	
			                           
			                           
		   // Run a partial GTK+ event loop to handle events.
		   while (gtk_events_pending()) { gtk_main_iteration(); }			
		}
		
		/** When a fixture is starting, this is called. */
		virtual void OnFixtureStart(const std::string &name)
		{
   		std::string title = std::string("Unit Test :: ") + name; 
   		
			// Set the name of the fixture being tested.
			gtk_window_set_title(GTK_WINDOW(runtests), title.c_str());			
			
			// Update the progress bar.
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(tests_done), 0.0);						
		}
		
		/** When a fixture is done with all it's tests, this is called. */
		virtual void OnFixtureDone()
		{
			Listener::OnFixtureDone();	
			
			// Update the progress bar
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(tests_done), 1.0);			
		}
		
		/** Called when the suite is about to start. */
		virtual void OnSuiteStart()
		{
		    CreateWindow();
		    
		    // Update the progress bar.
			 gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fixtures_done), 0.0);
		    
		    gtk_widget_show (runtests);
		    
		    nPassed=0;
		    nFailed=0;
		}
		
		/** When a suite is done with all it's tests, this is called. */
		virtual void OnSuiteDone()
		{
			Listener::OnSuiteDone();	
			
			std::string title = std::string("Unit Test :: Finished"); 
   		
			// Set the name of the fixture being tested.
			gtk_window_set_title(GTK_WINDOW(runtests), title.c_str());						
			
			// Update the progress bar.
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fixtures_done), 1.0);
		}
		
	  // Draws the test results widget.
	  void DrawResults()
	  {
	     int x=2, y=2;
	     GdkColor pass_col, fail_col, black_col;
	     GtkWidget* widget = test_output;
	     GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	     
	     GdkGCValues old_vals;
	     
	     // Save context
	     gdk_gc_get_values(gc, &old_vals);
	     
	     gdk_color_parse("red", &fail_col);
	     gdk_color_parse("green", &pass_col);
	     gdk_color_parse("black", &black_col);
	     
	     
	     for(event_list::iterator pos=events.begin(); pos!=events.end(); ++pos)
			{
			   // Set the approptiate color
				if (!(pos->pass))
				{
				  gdk_gc_set_rgb_fg_color(gc, &fail_col);
				}
				else
				{				                          
				  gdk_gc_set_rgb_fg_color(gc, &pass_col);
				}
				
				// Draw the filling
				gdk_draw_rectangle(test_output->window, 
				                     gc,
				                     TRUE,
				                     x, y,
				                     5, 5);
				 
				// Draw a border                    
				gdk_gc_set_rgb_fg_color(gc, &black_col); 
				gdk_draw_rectangle(test_output->window, 
				                     gc,
				                     FALSE,
				                     x, y,
				                     5, 5);                    
				                     
				
				// Adjust for next box                     
				x+=7;
				
				if (x>=widget->allocation.width)
				{
				  x=2;
				  y+=7;
				}
				
			}
			
			// Restore context
			gdk_gc_set_values(gc, &old_vals, GDK_GC_FOREGROUND);
	  
	  }
	  
	  // Draws the pass/fail widget
	  void DrawPassFail()
	  {
	     int x=2, y=2;
	     GdkColor pass_col, fail_col, black_col;
	     GtkWidget* widget = pass_fail;
	     GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
	     
	     GdkGCValues old_vals;
	     
	     // Save context
	     gdk_gc_get_values(gc, &old_vals);
	     
	     gdk_color_parse("red", &fail_col);
	     gdk_color_parse("green", &pass_col);
	     gdk_color_parse("black", &black_col);
	     
	      // Set the appropriate color
   		if (nFailed)
   		{
   		  gdk_gc_set_rgb_fg_color(gc, &fail_col);
   		}
   		else
   		{				                          
   		  gdk_gc_set_rgb_fg_color(gc, &pass_col);
   		}
	     
	     // Draw the filling
		  gdk_draw_rectangle(widget->window, 
				                     gc,
				                     TRUE,
				                     5, 5,
				                     widget->allocation.width-10, 
				                     widget->allocation.height-10);
			
		  // Draw frame
		  gdk_gc_set_rgb_fg_color(gc, &black_col);	                     
        gdk_draw_rectangle(widget->window, 
				                     gc,
				                     FALSE,
				                     5, 5,
				                     widget->allocation.width-10, 
				                     widget->allocation.height-10);				                     
	     
	     // Restore context
		  gdk_gc_set_values(gc, &old_vals, GDK_GC_FOREGROUND);	     
     }
		
	private:
	  void CreateWindow()
	  {
        runtests = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title (GTK_WINDOW (runtests), _("Unit Test - Running Tests"));
        gtk_window_set_default_size (GTK_WINDOW (runtests), 600, 400);
      
        viewport1 = gtk_viewport_new (NULL, NULL);
        gtk_widget_show (viewport1);
        gtk_container_add (GTK_CONTAINER (runtests), viewport1);
      
        eventbox1 = gtk_event_box_new ();
        gtk_widget_show (eventbox1);
        gtk_container_add (GTK_CONTAINER (viewport1), eventbox1);
      
        vpaned1 = gtk_vpaned_new ();
        gtk_widget_show (vpaned1);
        gtk_container_add (GTK_CONTAINER (eventbox1), vpaned1);
        gtk_paned_set_position (GTK_PANED (vpaned1), 200);
      
        vbox1 = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vbox1);
        gtk_paned_pack1 (GTK_PANED (vpaned1), vbox1, FALSE, TRUE);
      
        fixed1 = gtk_fixed_new ();
        gtk_widget_show (fixed1);
        gtk_box_pack_start (GTK_BOX (vbox1), fixed1, FALSE, TRUE, 0);
      
        label4 = gtk_label_new (_("Passed:"));
        gtk_widget_show (label4);
        gtk_fixed_put (GTK_FIXED (fixed1), label4, 0, 0);
        gtk_widget_set_size_request (label4, 56, 16);
      
        label5 = gtk_label_new (_("Failed:"));
        gtk_widget_show (label5);
        gtk_fixed_put (GTK_FIXED (fixed1), label5, 8, 16);
        gtk_widget_set_size_request (label5, 48, 16);
      
        tests_done = gtk_progress_bar_new ();
        gtk_widget_show (tests_done);
        gtk_fixed_put (GTK_FIXED (fixed1), tests_done, 312, 18);
        gtk_widget_set_size_request (tests_done, 320, 12);
      
        fixtures_done = gtk_progress_bar_new ();
        gtk_widget_show (fixtures_done);
        gtk_fixed_put (GTK_FIXED (fixed1), fixtures_done, 312, 2);
        gtk_widget_set_size_request (fixtures_done, 320, 12);
      
        pass_fail = gtk_drawing_area_new ();
        gtk_widget_show (pass_fail);
        gtk_fixed_put (GTK_FIXED (fixed1), pass_fail, 144, 0);
        gtk_widget_set_size_request (pass_fail, 120, 32);
      
        failed = gtk_label_new (_("0"));
        gtk_widget_show (failed);
        gtk_fixed_put (GTK_FIXED (fixed1), failed, 56, 16);
        gtk_widget_set_size_request (failed, 80, 16);
      
        passed = gtk_label_new (_("0"));
        gtk_widget_show (passed);
        gtk_fixed_put (GTK_FIXED (fixed1), passed, 56, 0);
        gtk_widget_set_size_request (passed, 80, 16);
      
        label10 = gtk_label_new (_("Tests"));
        gtk_widget_show (label10);
        gtk_fixed_put (GTK_FIXED (fixed1), label10, 280, 16);
        gtk_widget_set_size_request (label10, 34, 16);
      
        label9 = gtk_label_new (_("Fixtures"));
        gtk_widget_show (label9);
        gtk_fixed_put (GTK_FIXED (fixed1), label9, 272, 0);
        gtk_widget_set_size_request (label9, 40, 16);
      
        hseparator1 = gtk_hseparator_new ();
        gtk_widget_show (hseparator1);
        gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, FALSE, TRUE, 0);
      
        test_output = gtk_drawing_area_new ();
        gtk_widget_show (test_output);
        gtk_box_pack_start (GTK_BOX (vbox1), test_output, TRUE, TRUE, 0);
      
        notebook1 = gtk_notebook_new ();
        gtk_widget_show (notebook1);
        gtk_paned_pack2 (GTK_PANED (vpaned1), notebook1, TRUE, TRUE);
      
        scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow4);
        gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow4);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_SHADOW_IN);
      
        testinfo = gtk_tree_view_new ();
        gtk_widget_show (testinfo);
        gtk_container_add (GTK_CONTAINER (scrolledwindow4), testinfo);
      
        label7 = gtk_label_new (_("Failed"));
        gtk_widget_show (label7);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label7);
      
        scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_show (scrolledwindow5);
        gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow5);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_SHADOW_IN);
      
        passinfo = gtk_tree_view_new ();
        gtk_widget_show (passinfo);
        gtk_container_add (GTK_CONTAINER (scrolledwindow5), passinfo);
      
        label8 = gtk_label_new (_("Passed"));
        gtk_widget_show (label8);
        gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label8);
        
        // Setup the treeview
        GtkCellRenderer   *cell_render; 
        
        /////// Fail List View //////
        // Create a new store
        infostore = gtk_list_store_new(5, G_TYPE_STRING, // filename 
                                          G_TYPE_STRING, // testname
                                          G_TYPE_UINT,   // line number
                                          G_TYPE_STRING, // expression
                                          G_TYPE_STRING); // message
                
        // Create the columns
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(testinfo), 0,
                                    "Filename", cell_render,
                                    "text", 0, NULL);
        
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(testinfo), 1,
                                    "Test", cell_render,
                                    "text", 1, NULL);
        
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(testinfo), 2,
                                    "Line", cell_render,
                                    "text", 2, NULL);
        
        cell_render = gtk_cell_renderer_text_new();                            
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(testinfo), 3,
                                    "Expression", cell_render,
                                    "text", 3, NULL);
        
        cell_render = gtk_cell_renderer_text_new();                            
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(testinfo), 4,
                                    "Message", cell_render,
                                    "text", 4, NULL);  
                                    
        // Set the model                            
        gtk_tree_view_set_model(GTK_TREE_VIEW(testinfo), 
                                GTK_TREE_MODEL(infostore));                                  
        
                                         
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(testinfo), TRUE);  
        
        
        /////// Pass List View //////
        
        // Create a new store
        passstore = gtk_list_store_new(4, G_TYPE_STRING, // filename 
                                          G_TYPE_STRING, // testname
                                          G_TYPE_UINT,   // line number
                                          G_TYPE_STRING); // expression
                                          
                                          
       // Create the columns
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(passinfo), 0,
                                    "Filename", cell_render,
                                    "text", 0, NULL);
        
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(passinfo), 1,
                                    "Test", cell_render,
                                    "text", 1, NULL);
        
        cell_render = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(passinfo), 2,
                                    "Line", cell_render,
                                    "text", 2, NULL);
        
        cell_render = gtk_cell_renderer_text_new();                            
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(passinfo), 3,
                                    "Expression", cell_render,
                                    "text", 3, NULL);
                                          
        // Set the model                            
        gtk_tree_view_set_model(GTK_TREE_VIEW(passinfo), 
                                GTK_TREE_MODEL(passstore));                                  
        
                                         
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(passinfo), TRUE);
           
           
        // When the close button is pressed, run this     
        gtk_signal_connect(GTK_OBJECT(runtests),
                     "delete_event",
                     GTK_SIGNAL_FUNC(close_window_cb),
                     NULL);  
                     
        // Redraw callback for the red/green/refactor window             
        gtk_signal_connect(GTK_OBJECT(test_output), "expose_event",
                           G_CALLBACK(test_results_expose_cb), this);
                           
        // Redraw callback for the pass/fail window
        gtk_signal_connect(GTK_OBJECT(pass_fail), "expose_event",
                           G_CALLBACK(pass_fail_expose_cb), this);
                           
	  }
		
	private:
	  GtkWidget *runtests;
     GtkWidget *viewport1;
     GtkWidget *eventbox1;
     GtkWidget *vpaned1;
     GtkWidget *vbox1;
     GtkWidget *fixed1;
     GtkWidget *label4;
     GtkWidget *label5;
     GtkWidget *tests_done;
     GtkWidget *fixtures_done;
     GtkWidget *pass_fail;
     GtkWidget *failed;
     GtkWidget *passed;
     GtkWidget *label10;
     GtkWidget *label9;
     GtkWidget *hseparator1;
     GtkWidget *test_output;
     GtkWidget *notebook1;
     GtkWidget *scrolledwindow4;
     GtkWidget *testinfo;
     GtkWidget *label7;
     GtkWidget *scrolledwindow5;
     GtkWidget *passinfo;
     GtkWidget *label8;
        
     GtkListStore *infostore;
     GtkListStore *passstore;
   	  	             
	};
	
// Close the window.
static gint close_window_cb(GtkWidget* w, GdkEventAny *e, gpointer data)
{
   gtk_main_quit();
   return FALSE;  
}

// Redraw the test results
static gboolean test_results_expose_cb(GtkWidget *w, GdkEventExpose *event,
                                       gpointer data)
{
  GtkOutputter *outputter = static_cast<GtkOutputter *>(data);
  
  outputter->DrawResults();

  return TRUE; 
}

// Redraw the test results
static gboolean pass_fail_expose_cb(GtkWidget *w, GdkEventExpose *event,
                                       gpointer data)
{
  GtkOutputter *outputter = static_cast<GtkOutputter *>(data);
  
  outputter->DrawPassFail();

  return TRUE; 
}

		
} // end namespace


#endif

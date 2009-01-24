/***************************************************************************
                          toolkit_dialog.cpp  -  description
                             -------------------

  Handle simple dialog (alert, yes./no)


    begin                : Fri Dec 14 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_toolkitGtk.h"
#include "prefs.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)


static GtkWidget *widgetStack[10];
static int	  widgetCount=0;
void GUI_detransient(void )
{
        gtk_window_set_modal(GTK_WINDOW(widgetStack[0]), 0);
}
void GUI_retransient(void )
{
        gtk_window_set_modal(GTK_WINDOW(widgetStack[0]), 1);
}
void gtk_register_dialog(GtkWidget *newdialog)
{

        widgetStack[widgetCount]=newdialog;
        // old one is no longer modal
        if(widgetCount)
        {
#ifndef __WIN32                
                gtk_window_set_modal(GTK_WINDOW(widgetStack[widgetCount-1]), 0);
#endif
                gtk_window_set_transient_for (GTK_WINDOW(newdialog),GTK_WINDOW(widgetStack[widgetCount-1]));
        }
        gtk_window_set_modal(GTK_WINDOW(widgetStack[widgetCount]), 1);
#ifdef __WIN32
        gtk_window_set_icon(GTK_WINDOW(widgetStack[widgetCount]), gtk_window_get_icon (GTK_WINDOW(widgetStack[0])));
#endif
        widgetCount++;
}
void gtk_unregister_dialog(GtkWidget *newdialog)
{
	ADM_assert(widgetCount);
	ADM_assert(widgetStack[widgetCount-1]==newdialog);
	widgetCount--;
	if(widgetCount)
	{
		// Reset the old one modal
		gtk_window_set_modal(GTK_WINDOW(widgetStack[widgetCount-1]), 1);
#ifdef __WIN32
		gtk_window_present(GTK_WINDOW(widgetStack[widgetCount-1]));
#endif
	}

}
void		gtk_transient(GtkWidget *widget)
{
GtkWidget *top;
    return;
	assert(widgetCount);
	top=widgetStack[widgetCount-1];
		
	// The father is no longer modal
	gtk_window_set_modal(GTK_WINDOW(top), 0);
	// But we are
	gtk_window_set_modal(GTK_WINDOW(widget), 1);
	
	gtk_window_set_transient_for (GTK_WINDOW(widget),GTK_WINDOW(top));
	
	
	


}


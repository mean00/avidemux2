/***************************************************************************
                          GUI_binding.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by mean
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
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/
/*******************THIS IS NOT USED ANYMORE********************/

#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <math.h>
#include "ADM_default.h"


#include "gui_action.hxx"
extern void HandleAction(Action act);  
// when keys are pressed
// We have to duplicate the ALT ... shortcut
// because of change of focus ?

gboolean UI_on_key_press(GtkWidget *widget, GdkEventKey* event, gpointer user_data)
{
    UNUSED_ARG(widget);
    UNUSED_ARG(user_data);
	gboolean shift = FALSE;
	gboolean ctrl = FALSE;
	gboolean alt = FALSE;
	Action action;

	//printf("key : %d (%c) \n",event->keyval,event->keyval);
	
	if (event->state & GDK_CONTROL_MASK)
	{
		ctrl = TRUE;
	}
	if (event->state & GDK_SHIFT_MASK)
	{
		shift = TRUE;
	}
	if(event->state & GDK_MOD1_MASK)
	{
		alt = TRUE;
	}
	// ALT+x
	//_____________
	
	action=ACT_DUMMY;
	

    switch (event->keyval)
	{	
	case GDK_Up:
		action=ACT_NextKFrame;
		break;
	case GDK_Down:
		action=ACT_PreviousKFrame;
		break;
			
		// Position advance
       	case GDK_Left: case GDK_KP_Left:

		// One frame
		if((shift == FALSE) && (ctrl == FALSE))
		{
			action = ACT_PreviousFrame;
		}
                // 100 frames
                else if((shift == TRUE) && (ctrl == TRUE))
                {
                        action = ACT_Back100Frames;
                }
		// 50 frames
		else if(ctrl == TRUE)
		{
			action = ACT_Back50Frames;
		}
		// 25 frames
		else
		{
			action = ACT_Back25Frames;
		}
		break;

		// Position reverse
	case GDK_Right: case GDK_KP_Right:
		if((shift == FALSE) && (ctrl == FALSE))
		{
			action = ACT_NextFrame;
		}
                // 100 frames
                else if((shift == TRUE) && (ctrl == TRUE))
                {
                        action = ACT_Forward100Frames;
                }

		else if(ctrl == TRUE)
		{
			action = ACT_Forward50Frames;
		}
		else
		{
			action = ACT_Forward25Frames;
		}
		break;
		
        }
        if(action!=ACT_DUMMY) // For me to handle
        {
	       HandleAction(action);
               return TRUE;
        }
        return FALSE;
}

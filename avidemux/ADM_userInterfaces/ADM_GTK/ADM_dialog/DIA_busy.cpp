/***************************************************************************
                          DIA_Busy.cpp  -  description
                             -------------------
    begin                : Thu Apr 21 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

	This function displays a busy box

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

#include "DIA_busy.h"

static GtkWidget	*create_dialog1 (void);
static GtkWidget *busy=NULL;
static gint on_destroy_abort(GtkObject * object, gpointer user_data);
extern  void UI_BusyCursor( void );
extern void UI_NormalCursor( void );

gint on_destroy_abort(GtkObject * object, gpointer user_data)
{
      UNUSED_ARG(object);
      UNUSED_ARG(user_data);
      return TRUE;

};

void DIA_StartBusy( void )
{

	UI_BusyCursor();
        //UI_purge();

	
}

void DIA_StopBusy( void )
{
	UI_NormalCursor();
}


void DIA_StartBusyDialog( void )
{
        
        busy=create_dialog1();
        gtk_signal_connect(GTK_OBJECT(busy), "delete_event",
                      GTK_SIGNAL_FUNC(on_destroy_abort), (void *) NULL);
        gtk_register_dialog(busy);
        gtk_widget_show(busy);
        
        UI_purge();
        
        ADM_usleep(500);
	
}

void DIA_StopBusyDialog( void )
{
        if(busy)
        {
                
                gtk_unregister_dialog(busy);
                gtk_widget_destroy(busy);
                
                busy=NULL;
        }
        else
        {
                printf("\n Busy was null ???\n");
        }

}

void DIA_runBusy( void )
{
  UI_purge();
}


GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *image1;
  GtkWidget *dialog_action_area1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Busy"));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  image1 = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), image1, TRUE, TRUE, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");

  return dialog1;
}


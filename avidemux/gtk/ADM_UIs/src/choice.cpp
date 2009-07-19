/***************************************************************************
                          choice.cpp  -  description
                             -------------------
    begin                : Fri Sep 20 2002
    copyright            : (C) 2002 by mean
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


static void alert_cb_ok(void);
static void alert_cb_destroyed(void);
static void alert_cb_ko(void);


extern GtkWidget *GUI_PixmapButtonDefault(GdkWindow * window, const gchar ** xpm,
				   const gchar * tooltip);
extern GtkWidget *GUI_PixmapButton(GdkWindow * window, const gchar ** xpm,
			    const gchar * tooltip, gint border);


static int wait_gui;
static int destroy_flag;


//static GtkTooltips *tooltips = NULL;

//_________________________________
// Call back for press/ destroy
//_________________________________
void alert_cb_ok(void)
{

    wait_gui = 1;

}

void alert_cb_destroyed(void)
{

    wait_gui = 0;
    destroy_flag = 1;

}

void alert_cb_ko(void)
{

    wait_gui = 2;

}



//
//_______________________________
namespace ADM_GtkCoreUIToolkit
{
int GUI_Alternate(const char *title,const char *choice1,const char *choice2)
{
    GtkWidget *window1;
    GtkWidget *vbox1;
    GtkWidget *label2;
    GtkWidget *button1;
    GtkWidget *button2;
    int ret;

    window1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data(GTK_OBJECT(window1), "window1", window1);
    gtk_window_set_title(GTK_WINDOW(window1), "Alert");
    gtk_widget_set_usize(window1, 200, 70);
    gtk_window_set_modal(GTK_WINDOW(window1), 1);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_ref(vbox1);
    gtk_object_set_data_full(GTK_OBJECT(window1), "vbox1", vbox1,
			     (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(window1), vbox1);

    label2 = gtk_label_new(title);
    gtk_widget_ref(label2);
    gtk_object_set_data_full(GTK_OBJECT(window1), "label2", label2,
			     (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show(label2);
    gtk_box_pack_start(GTK_BOX(vbox1), label2, FALSE, FALSE, 0);

    button1 = gtk_button_new_with_label(choice1);
    gtk_widget_ref(button1);
    gtk_object_set_data_full(GTK_OBJECT(window1),
			     "window1", window1,
			     (GtkDestroyNotify) alert_cb_destroyed);

    gtk_signal_connect(GTK_OBJECT(button1),
		       "button_press_event", GTK_SIGNAL_FUNC(alert_cb_ok),
		       0);

    gtk_widget_show(button1);
    gtk_box_pack_start(GTK_BOX(vbox1), button1, FALSE, FALSE, 0);
//
    button2 = gtk_button_new_with_label(choice2);
    gtk_widget_ref(button2);
    gtk_signal_connect(GTK_OBJECT(button2),
		       "button_press_event", GTK_SIGNAL_FUNC(alert_cb_ko),
		       0);

    gtk_widget_show(button2);
    gtk_box_pack_start(GTK_BOX(vbox1), button2, FALSE, FALSE, 0);



    gtk_widget_show(window1);
//  printf("\n Waiting to be unlocked");
    wait_gui = -1;
    destroy_flag = 0;
    while (wait_gui == -1)
	while (gtk_events_pending())
	  {
	      gtk_main_iteration();
	  }
    ret = wait_gui;
    if (!destroy_flag)
	gtk_widget_destroy(window1);

    return ret;
}

} // End of namespace
//EOF

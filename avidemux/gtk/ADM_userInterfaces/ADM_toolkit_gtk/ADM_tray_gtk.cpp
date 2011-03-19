/***************************************************************************
                              ADM_tray_gtk.cpp
                              ----------------

    begin                : Sun Jul 14 2002
    copyright            : (C) 2002 by mean/gruntster
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

#include "config.h"

#include <string.h>
#include <stdio.h>

#include "ADM_toolkitGtk.h"
#include "ADM_tray.h"

//extern void UI_deiconify(void);

static GdkPixbuf **pixbuf = NULL;
static int lastIcon;

static const char *animated[] = {
	"film1.png","film3.png","film5.png","film7.png","film9.png",
	"film11.png","film13.png","film15.png","film17.png","film19.png",
	"film21.png","film23.png"};

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
	gtk_window_deiconify(GTK_WINDOW(user_data));
	//UI_deiconify();
}

static void tray_menu_open_avidemux(GtkStatusIcon *status_icon, gpointer user_data)
{
	gtk_window_deiconify(GTK_WINDOW(user_data));
	//UI_deiconify();
}

static void tray_icon_popup_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
	GtkWidget *popupMenu = gtk_menu_new();
	GtkWidget *item = gtk_menu_item_new_with_label(QT_TR_NOOP("Open Avidemux"));

	gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), item);
	g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(tray_menu_open_avidemux), user_data);

	gtk_widget_show_all(popupMenu);
	gtk_menu_popup(GTK_MENU(popupMenu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

ADM_tray::ADM_tray(void* parent)
{
	_parent = parent;
	lastIcon = 0;
	int nb = sizeof(animated) / sizeof(char *);

	if (!pixbuf)
	{
		pixbuf = new GdkPixbuf*[nb];

		for (int i = 0; i < nb; i++)
		{
#warning broken
		//	pixbuf[i] = create_pixbuf(animated[i]);

			if (!pixbuf[i])
			{
				printf("Failed to create <%s>\n", animated[i]);
			//	ADM_assert(0);
			}
		}
	}

	sys = gtk_status_icon_new_from_pixbuf(pixbuf[0]);

	g_signal_connect(G_OBJECT(sys), "activate", G_CALLBACK(tray_icon_on_click), _parent);
	g_signal_connect(G_OBJECT(sys), "popup-menu", G_CALLBACK(tray_icon_popup_menu), _parent);
	gtk_status_icon_set_tooltip_text((GtkStatusIcon*)sys, "Avidemux");
	gtk_status_icon_set_visible((GtkStatusIcon*)sys, TRUE);
}

ADM_tray::~ADM_tray()
{
	if (sys)
		gtk_status_icon_set_visible((GtkStatusIcon*)sys, FALSE);

	sys = NULL;
}

uint8_t ADM_tray::setPercent(int percent)
{
	char percentS[40];

	sprintf(percentS, "Avidemux [%d%%]", percent);

	if (sys)
	{
		int maxIcons = sizeof(animated) / sizeof(char *);
		lastIcon++;

		if (lastIcon >= maxIcons)
			lastIcon = 0;

		gtk_status_icon_set_from_pixbuf((GtkStatusIcon*)sys, pixbuf[lastIcon]);
		gtk_status_icon_set_tooltip_text((GtkStatusIcon*)sys, percentS);
	}

	return 1;
}

uint8_t ADM_tray::setStatus(int working)
{
	return 1;
}

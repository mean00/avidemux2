
/***************************************************************************
copyright            : (C) 2001/2006 by mean
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
#ifndef ADM_TLK_GTK
#define ADM_TLK_GTK

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "ADM_default.h"
#include "ADM_gladeSupport.h"
#include "ADM_toolkitGtkInclude.h"

uint8_t getRangeInMenu(GtkWidget *Menu);
void changeTextInMenu(GtkWidget *menu, int range, const char *text);
void UI_purge(void);
uint8_t getSelectionNumber(uint32_t nb, GtkWidget *tree, GtkListStore *store, uint32_t *number);
uint8_t setSelectionNumber(uint32_t nb, GtkWidget *tree, GtkListStore *store, uint32_t number);
int gtk_read_entry(GtkWidget *entry);
void gtk_write_entry(GtkWidget *entry, int value);
void gtk_write_entry_string(GtkWidget *entry, char *value);
float gtk_read_entry_float(GtkWidget *entry);
void gtk_write_entry_float(GtkWidget *entry, float value);

/* Window handling: transient modal etc..*/
void gtk_register_dialog(GtkWidget *newdialog);
void gtk_unregister_dialog(GtkWidget *newdialog);
void gtk_transient(GtkWidget *widget);
/* Some resizing function helpers */
float UI_calcZoomToFitScreen(GtkWindow* window, GtkWidget* drawingArea, uint32_t imageWidth, uint32_t imageHeight);
void  UI_centreCanvasWindow(GtkWindow *window, GtkWidget *canvas, int newCanvasWidth, int newCanvasHeight);

#define WID(x) lookup_widget(dialog,#x)
#define RADIO_SET(widget_name,value) gtk_toggle_button_set_active \
	(GTK_TOGGLE_BUTTON (WID(widget_name)), value)

#define RADIO_GET(widget_name) gtk_toggle_button_get_active \
	(GTK_TOGGLE_BUTTON (WID(widget_name)))

#ifdef QT_TR_NOOP
#undef QT_TR_NOOP
#endif

#define QT_TR_NOOP(String) String // FIXME
#warning FIXME TRANSLATE


#endif

/***************************************************************************
    Hook to Gfx toolkit concerning display (RGB or accel)
                             -------------------
    
    copyright            : (C) 2007 by mean
    email                : Mean/fixounet@free.fr
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
#include <gtk/gtk.h>

#ifdef __WIN32
#include <gdk/gdkwin32.h>
#elif defined(__APPLE__)
extern "C"
{
	int getMainNSWindow(void);
}
#else
#include <gdk/gdkx.h>
#endif
#include "ADM_toolkitGtk.h"

#include "ADM_misc.h"

#include "../ADM_render/GUI_render.h"
#include "../ADM_render/GUI_accelRender.h"

#include "ADM_colorspace.h"

extern GtkWidget *guiRootWindow;
extern GtkWidget *getDrawWidget(void);
extern uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w, uint32_t *h);

#ifdef ENABLE_WINDOW_SIZING_HACK
extern int maxWindowWidth, maxWindowHeight; // from GUI_bindings.cpp
#endif


uint32_t lastW,lastH;
/**
    \brief return pointer to the drawing widget that displays video
*/
void *UI_getDrawWidget(void)
{
  return (void *) getDrawWidget();
}
/**
    \brief Display to widget in RGB32
*/
void UI_rgbDraw(void *widg,uint32_t w, uint32_t h,uint8_t *ptr)
{
    GtkWidget *widget;
    widget = (GtkWidget *)widg; 

    if(lastW>w || lastH>h)
    {
// This makes a lot of noise if you use the Crop filter...  perhaps it should
// be output only once?  Or maybe there is a real bug - I'm not sure.  [Chris MacGregor]
//      printf("[GTK] Warning window bigger than display %u x %u vs %u x %u\n",w,h,lastW,lastH);
    }

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w);
    cairo_surface_t *s = cairo_image_surface_create_for_data((unsigned char*)ptr, 
                                                             CAIRO_FORMAT_RGB24, 
                                                             w, 
                                                             h, 
                                                             stride);
    cairo_set_source_surface(cr, s, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(s);
}
/**
      \brief Resize the window
*/
void  UI_updateDrawWindowSize(void *win,uint32_t w,uint32_t h)
{
    gtk_window_set_resizable(GTK_WINDOW(guiRootWindow), FALSE);
    gtk_widget_set_size_request((GtkWidget *)win, w, h);
    UI_purge();
    gtk_window_set_resizable(GTK_WINDOW(guiRootWindow), TRUE);
    lastW=w;
    lastH=h;
    printf("[GTK] Changing size to %u %u\n",w,h);
    UI_purge();
}
/**
      \brief Retrieve info from window, needed for accel layer
*/
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
		GdkWindow *win, *parentWin;
		GtkWidget *widget=(GtkWidget *)draw;

		win = gtk_widget_get_window(widget);
		parentWin = gtk_widget_get_parent_window(widget);

#ifdef __WIN32
		xinfo->display = (void*)GDK_WINDOW_HWND(win);
#elif defined(__APPLE__)
		xinfo->display = 0;
		xinfo->window = getMainNSWindow();
#else
		xinfo->window = GDK_WINDOW_XID(win);
		xinfo->display = GDK_WINDOW_XDISPLAY(parentWin);
#endif

		int windowWidth, windowHeight;
		int x, y;

		windowWidth = gdk_window_get_width(parentWin);
		windowHeight = gdk_window_get_height(parentWin);
		gdk_window_get_position(win, &x, &y);

		xinfo->x = x;
		xinfo->y = windowHeight - (y + lastH);
		xinfo->width = lastW;
		xinfo->height = lastH;
}

// EOF

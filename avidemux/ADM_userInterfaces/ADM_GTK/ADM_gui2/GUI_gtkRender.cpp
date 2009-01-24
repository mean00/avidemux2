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
#include "ADM_libraries/ADM_ffmpeg/ADM_libswscale/ADM_mp.h"

void GUI_gtk_grow_off(int onff);

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

    gdk_draw_rgb_32_image(widget->window, widget->style->fg_gc[GTK_STATE_NORMAL], 0,    // X
                       0,       // y
                       w,       //width
                       h,       //h*2, // heigth
                       GDK_RGB_DITHER_NONE,
                       //GDK_RGB_DITHER_MAX,  // dithering
                       (guchar *) ptr,  // buffer
                       w * 4);
}
/**
      \brief Resize the window
*/
void  UI_updateDrawWindowSize(void *win,uint32_t w,uint32_t h)
{
    GUI_gtk_grow_off(0);
    gtk_widget_set_usize((GtkWidget *)win, w, h);
    UI_purge();
    GUI_gtk_grow_off(1);
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
        GdkWindow *win;
        GtkWidget *widget=(GtkWidget *)draw;
          
        win = gtk_widget_get_parent_window(widget);

#ifdef __WIN32
		xinfo->display = (void*)GDK_WINDOW_HWND(widget->window);
#elif defined(__APPLE__)
		xinfo->display = 0;
		xinfo->window = getMainNSWindow();
#else
		xinfo->window = GDK_WINDOW_XWINDOW(widget->window);
		xinfo->display = GDK_WINDOW_XDISPLAY(win);
#endif

		int windowWidth, windowHeight;
		int x, y;

		gdk_drawable_get_size(win, &windowWidth, &windowHeight);
		gdk_window_get_position(widget->window, &x, &y);

		xinfo->x = x;
		xinfo->y = windowHeight - (y + lastH);
		xinfo->width = lastW;
		xinfo->height = lastH;
}

// EOF

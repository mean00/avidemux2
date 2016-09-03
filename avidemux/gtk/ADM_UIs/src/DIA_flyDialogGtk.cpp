/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///

#include "ADM_toolkitGtk.h"
#include "DIA_flyDialogGtk.h"
#include "DIA_factory.h"


#ifdef USE_JOG
#include "gui_action.hxx"
#include "../ADM_toolkit_gtk/ADM_jogshuttle.h"

extern PhysicalJogShuttle *physical_jog_shuttle;
#endif

extern float UI_calcZoomToFitScreen(GtkWindow* window, GtkWidget* drawingArea, uint32_t imageWidth, uint32_t imageHeight);
extern void UI_centreCanvasWindow(GtkWindow *window, GtkWidget *canvas, int newCanvasWidth, int newCanvasHeight);

/**
 *      \fn flyDialogGtk constructor
 * 
 */

ADM_flyDialogGtk::ADM_flyDialogGtk(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod):
                                ADM_flyDialog(width,height,in,canvas,slider,yuv,resizeMethod)
  {
		EndConstructor();
  }
void ADM_flyDialogGtk::postInit(uint8_t reInit)
{
	if (_slider)
	{
		GtkAdjustment *adj = (GtkAdjustment*)gtk_adjustment_new(0, 0, ADM_FLY_SLIDER_MAX, 0, 1, 0);

		gtk_range_set_adjustment(GTK_RANGE(_slider), adj);
		gtk_scale_set_digits(GTK_SCALE(_slider), 0);
	}

	GtkWindow *window = (GtkWindow*)gtk_widget_get_ancestor((GtkWidget*)_canvas, GTK_TYPE_WINDOW);
	UI_centreCanvasWindow(window, (GtkWidget*)_canvas, _zoomW, _zoomH);
	gtk_widget_set_size_request((GtkWidget*)_canvas, _zoomW, _zoomH);
#ifdef USE_JOG
        if (!reInit)
            physical_jog_shuttle->registerCBs (this, PhysicalJogShuttle::NoButtonCB,
                                               jogDial, jogRing);
#endif
}

ADM_flyDialogGtk::~ADM_flyDialogGtk(void)
{
#ifdef USE_JOG
        physical_jog_shuttle->deregisterCBs (this);
#endif
}

#ifdef USE_JOG
void ADM_flyDialogGtk::jogDial (void * my_data, signed short offset)
{
    ADM_flyDialog * myFly = static_cast <ADM_flyDialog *> (my_data);
    myFly->sliderSet (myFly->sliderGet() + offset);
}

static guint jogRingTimerID = 0;
static signed short jogRingIncr = 0;

static gboolean on_jogRingTimer (gpointer data)
{
    gdk_threads_enter();

    ADM_flyDialog * myFly = static_cast <ADM_flyDialog *> (data);
    myFly->sliderSet (myFly->sliderGet() + jogRingIncr);

    gdk_threads_leave();
    return TRUE;
}

void ADM_flyDialogGtk::jogRing (void * my_data, gfloat angle)
{
    if (jogRingTimerID)
    {
        g_source_remove (jogRingTimerID);
        jogRingTimerID = 0;
    }

    if (angle > -0.0001 && angle < 0.0001)
        return;

    jogRingIncr = (angle < 0) ? -1 : +1;
    angle *= jogRingIncr; // absolute value
    jogRingTimerID = g_timeout_add (guint ((1 - angle) * 500 + 10),
                                    on_jogRingTimer, my_data);
}
#endif

float ADM_flyDialogGtk::calcZoomFactor(void)
{
	GtkWindow *window = (GtkWindow*)gtk_widget_get_ancestor((GtkWidget*)_canvas, GTK_TYPE_WINDOW);

	return UI_calcZoomToFitScreen(window, (GtkWidget*)_canvas, _w, _h);
}

uint8_t  ADM_flyDialogGtk::display(uint8_t *rgbData)
{
	ADM_assert(_canvas);
	ADM_assert(rgbData);
	GtkWidget *widget=(GtkWidget*)_canvas;

	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, _zoomW);
	cairo_surface_t *s = cairo_image_surface_create_for_data((unsigned char*)rgbData, 
	                                                         CAIRO_FORMAT_RGB24, 
	                                                         _zoomW, 
	                                                         _zoomH, 
	                                                         stride);
	cairo_set_source_surface(cr, s, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	cairo_surface_destroy(s);
	//GUI_RGBDisplay(_rgbBufferOut, _zoomW, _zoomH, _canvas);
	return 1; 
}

uint32_t ADM_flyDialogGtk::sliderGet(void)
{
	ADM_assert(_slider);
	return (uint32_t)gtk_range_get_value (GTK_RANGE(_slider));
}

uint8_t ADM_flyDialogGtk::sliderSet(uint32_t value)
{
	ADM_assert(_slider);

        gtk_range_set_value (GTK_RANGE(_slider), value);

	return 1; 
}

bool ADM_flyDialogGtk::isRgbInverted(void)
{
	return false; 
}


//EOF

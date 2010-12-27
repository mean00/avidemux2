
//
/***************************************************************************
                          DIA_Equalizer
                             -------------------

			   Ui for equalizer, ugly

    begin                : 30 Dec 2004
    copyright            : (C) 2004/5 by mean
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
#include <math.h>

#include "ADM_toolkitGtk.h"

#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_vidEqualizer.h"
#include "ADM_colorspace.h"
extern "C" {
#include "libavcodec/avcodec.h"
}

static ColYuvRgb    *rgbConv=NULL;
uint8_t DIA_getEqualizer(EqualizerParam *param, ADMImage *image);

static GtkWidget	*create_dialog1 (void);
static void  		update ( void);
static gboolean 	draw (void );
// static void 		reset( void );
static void 		upload(void);
static void  		read ( void );
static void		recalc( void );
static void 		drawCross(uint32_t x,uint32_t y);
static void 		spinner(void);
static void 		compute_histogram(void);
static void 		frame_changed( void );

extern void GUI_RGBDisplay(uint8_t * dis, uint32_t w, uint32_t h, void *widg);
extern float UI_calcZoomToFitScreen(GtkWindow* window, GtkWidget* drawingArea, uint32_t imageWidth, uint32_t imageHeight);
extern void UI_centreCanvasWindow(GtkWindow *window, GtkWidget *canvas, int newCanvasWidth, int newCanvasHeight);

static ADMImage *imgsrc,*imgdst,*imgdisplay;
static GtkWidget *dialog=NULL;
static uint32_t scaler[256];
static uint32_t w,h,zoomW,zoomH;
static uint32_t *rgbbuffer=NULL;
static ADMImageResizer *resizer=NULL;

static uint32_t *histogram=NULL;
static uint32_t *histogramout=NULL;
static AVDMGenericVideoStream *incoming;
static const int cross[8]= {0,36,73,109,
			146,182,219,255};
#define CROSS 0xFFFF0000
#define DRAW  0x0000FF00
#define LINER 0x0000FFFF

#define ZOOM_FACTOR 5

#define A_RESET 99
//
//	Video is in YV12 Colorspace
//
//
uint8_t DIA_getEqualizer(EqualizerParam *param, AVDMGenericVideoStream *in)
{
	int ret;
	uint32_t l,f;
	uint32_t max=in->getInfo()->nb_frames;
        
	incoming=in;
	// Allocate space for green-ised video
	w=in->getInfo()->width;
	h=in->getInfo()->height;
	rgbConv=new ColYuvRgb(w,h);
        rgbConv->reset(w,h);
	
	histogram=new uint32_t [256*128];
        histogramout=new uint32_t [256*128];
	
	imgdst=new ADMImage(w,h);
	imgsrc=new ADMImage(w,h);
        imgdisplay=new ADMImage(w,h);
	
        max=max/2;
        
	ADM_assert(in->getFrameNumberNoAlloc(max, &l, imgsrc,&f));
        memcpy(imgdisplay->data+w*h,imgsrc->data+w*h,(w*h)>>1);
	// init local equalizer
		
        memcpy(scaler,param->_scaler,sizeof(scaler));

	dialog=create_dialog1();
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										A_RESET,
										GTK_RESPONSE_OK,
										GTK_RESPONSE_CANCEL,
										GTK_RESPONSE_APPLY,
										-1);
	gtk_register_dialog(dialog);
	gtk_widget_set_usize(WID(drawingarea_histin), 256,128);
    gtk_widget_set_usize(WID(drawingarea_histout), 256,128);

	float zoom = UI_calcZoomToFitScreen(GTK_WINDOW(dialog), WID(drawingarea1), w, h);

	zoomW = w * zoom;
	zoomH = h * zoom;
	rgbbuffer=new uint32_t[zoomW*zoomH];

	gtk_widget_set_usize(WID(drawingarea1), zoomW, zoomH);

	if (zoom < 1)
	{
		UI_centreCanvasWindow((GtkWindow*)dialog, WID(drawingarea1), zoomW, zoomH);
		resizer = new ADMImageResizer(w, h, zoomW, zoomH, PIX_FMT_YUV420P, PIX_FMT_RGB32);
	}

	  upload();

	gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
		       GTK_SIGNAL_FUNC(draw),
		       NULL);
		       
	
	gtk_signal_connect(GTK_OBJECT(WID(gui_scale)), "value_changed",GTK_SIGNAL_FUNC(frame_changed),   NULL);
  //      gtk_signal_connect(GTK_OBJECT(WID(curve1)), "curve-type-changed",GTK_SIGNAL_FUNC(spinner),   NULL);
	
        /*

        */
        GtkWidget *curve=WID(curve1);
        gtk_curve_set_range (GTK_CURVE(curve),0,255.,0.,255.);
	
        gtk_widget_show(dialog);
        upload();
        compute_histogram();
        spinner();
	ret=0;
	int response;
_again:
	while( (response=gtk_dialog_run(GTK_DIALOG(dialog)))==GTK_RESPONSE_APPLY)
	{
		spinner();	
	}
        if(response==A_RESET)
        {
                gfloat duo[2]={0,255.};
                gtk_curve_set_curve_type(GTK_CURVE(WID(curve1)),GTK_CURVE_TYPE_SPLINE);
                gtk_curve_reset(GTK_CURVE(WID(curve1)));

                goto _again;
        }
        
	if(response==GTK_RESPONSE_OK)
        {
		printf("Accepting new values\n");
		memcpy(param->_scaler,scaler,sizeof(scaler));
		ret=1;
	}
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	
	delete imgdst;
	delete imgsrc;
        delete imgdisplay;
	delete [] rgbbuffer;
	
	delete [] histogram;
        delete [] histogramout;
    delete rgbConv;

	if (resizer)
	{
		delete resizer;
		resizer=NULL;
	}

    rgbConv=NULL;    
	histogram=NULL;
        histogramout=NULL;
	
	rgbbuffer=NULL;
	imgdst=NULL;
	imgsrc=NULL;
	dialog=NULL;
        imgdisplay=NULL;
	return ret;

}
void frame_changed( void )
{
uint32_t new_frame,max,l,f;
double   percent;
GtkWidget *wid;	
GtkAdjustment *adj;
	
	max=incoming->getInfo()->nb_frames;
	wid=WID(gui_scale);
	adj=gtk_range_get_adjustment (GTK_RANGE(wid));
	new_frame=0;
	
	percent=(double)GTK_ADJUSTMENT(adj)->value;
	percent*=max;
	percent/=100.;
	new_frame=(uint32_t)floor(percent);
	
	if(new_frame>=max) new_frame=max-1;
	
	ADM_assert(incoming->getFrameNumberNoAlloc(new_frame, &l, imgsrc,&f));
         memcpy(imgdisplay->data+w*h,imgsrc->data+w*h,(w*h)>>1);
	compute_histogram();
	update();

}
void spinner(void)
{
		read();
		recalc();
		upload();
                compute_histogram();
		update();
}
void recalc( void )
{
uint32_t y,tgt;
	// compute the in-between field & display them
	
	draw();
}
void drawCross(uint32_t x,uint32_t y)
{

}
void update( void)
{
	uint8_t *src,*dst,*disp;
	src=imgsrc->data;
	dst=imgdst->data;
	// Only do left side of target
	for(int y=0;y<h;y++)
	{
		for(int x=0;x<w;x++)
		{
			*dst=scaler[*src];
			dst++;
			src++;
		}		
	}
        // Img src = 10
        //           01
        // Img dst= 01
        //          10
        uint32_t half=w>>1;
        
        dst=imgdst->data;
        src=imgsrc->data;
        disp=imgdisplay->data;
        
        for(int y=0;y<h;y++)
        {
                if(y>h/2)
                {
                        memcpy(disp,src,half);
                        memcpy(disp+half,dst+half,half);
                
                
                }
                else
                {
                
                        memcpy(disp,dst,half);
                        memcpy(disp+half,src+half,half);
                }
                src+=w;
                dst+=w;
                disp+=w;
        
        }
	// udate u & v
	// now convert to rgb
	//COL_yv12rgb(  w,   h,imgdisplay->data,(uint8_t *)rgbbuffer );
	if (resizer)
		resizer->resize(imgdisplay->data, (uint8_t*)rgbbuffer);
	else
		rgbConv->scale(imgdisplay->data, (uint8_t*)rgbbuffer);

	draw();
}
// Compute histogram
// Top is histogram in, bottom is histogram out
void compute_histogram(void)
{
	uint32_t value[256];
        uint32_t valueout[256];
        uint8_t v;
        
	memset(value,0,256*sizeof(uint32_t));
        memset(valueout,0,256*sizeof(uint32_t));
        // In
	for(uint32_t t=0;t<w*h;t++)
	{
                v=imgsrc->data[t];
		value[v]++;	
                valueout[scaler[v]]++;
	}
	// normalize
	double d,a;
	a=w*h;
	for(uint32_t i=0;i<256;i++)
	{
		d=value[i];
		d*=256*ZOOM_FACTOR;
		d/=a;
		value[i]=(uint32_t)floor(d+0.49);
		
		if(value[i]>127) value[i]=127;
                
                d=valueout[i];
                d*=256*ZOOM_FACTOR;
                d/=a;
                valueout[i]=(uint32_t)floor(d+0.49);
                
                if(valueout[i]>127) valueout[i]=127;
                
                
	}
	// Draw
	memset(histogram,0,256*128*sizeof(uint32_t));
        memset(histogramout,0,256*128*sizeof(uint32_t));
	uint32_t y,tgt,yout;
	for(uint32_t i=0;i<256;i++)
	{
		y=value[i];
                
		for(uint32_t u=0;u<=y;u++)
		{
			tgt=i+(127-u)*256;
			histogram[tgt]=0xFFFFFFFF;
		}
                
                y=valueout[i];
                
                for(uint32_t u=0;u<=y;u++)
                {
                        tgt=i+(127-u)*256;
                        histogramout[tgt]=0xFFFFFFFF;
                }
	}

        
}
/*---------------------------------------------------------------------------
	Actually draw the working frame on screen
*/
gboolean draw (void)
{
	GtkWidget *draw=WID(drawingarea1);

	GUI_RGBDisplay((uint8_t *)rgbbuffer, zoomW, zoomH, (void *)draw);
/*	
	draw=WID(histogram);
	GUI_RGBDisplay((uint8_t *)bargraph, 256,256, (void *)draw);
*/	
	draw=WID(drawingarea_histin);
	GUI_RGBDisplay((uint8_t *)histogram, 256,128, (void *)draw);
	
        draw=WID(drawingarea_histout);
        GUI_RGBDisplay((uint8_t *)histogramout, 256,128, (void *)draw);
        
	return true;
}

void read ( void)
{
uint32_t g;

        gfloat sample[256];
        gtk_curve_get_vector(GTK_CURVE(WID(curve1)),256,sample);
        for(int i=0;i<256;i++)
                {
                        if(sample[i]<0) g=0;
                        else if(sample[i]>255) g=255;
                                else g=(uint32_t)sample[i];
                        scaler[i]=g;
                      //  printf("%u %u\n",i,scaler[i]);
                }
}

void upload(void)
{
#define SCALING 1
gfloat g;

        gfloat sample[256];
        
        for(int i=0;i<256/SCALING;i++)
                {
                       
                       sample[i]=scaler[i*SCALING];
                }
        gtk_curve_set_vector(GTK_CURVE(WID(curve1)),256/SCALING,sample);
        //gtk_curve_set_curve_type(GTK_CURVE(WID(curve1)),GTK_CURVE_TYPE_LINEAR);

}
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *drawingarea1;
  GtkWidget *gui_scale;
  GtkWidget *hbox1;
  GtkWidget *curve1;
  GtkWidget *vbox2;
  GtkWidget *drawingarea_histin;
  GtkWidget *drawingarea_histout;
  GtkWidget *dialog_action_area1;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;
  GtkWidget *button4;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Equalizer"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_box_pack_start (GTK_BOX (vbox1), drawingarea1, TRUE, TRUE, 0);

  gui_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 10, 10)));
  gtk_widget_show (gui_scale);
  gtk_box_pack_start (GTK_BOX (vbox1), gui_scale, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  curve1 = gtk_curve_new ();
  gtk_widget_show (curve1);
  gtk_box_pack_start (GTK_BOX (hbox1), curve1, TRUE, TRUE, 0);
  gtk_curve_set_range (GTK_CURVE (curve1), 0, 1, 0, 1);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  drawingarea_histin = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea_histin);
  gtk_box_pack_start (GTK_BOX (vbox2), drawingarea_histin, TRUE, TRUE, 0);

  drawingarea_histout = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea_histout);
  gtk_box_pack_start (GTK_BOX (vbox2), drawingarea_histout, TRUE, TRUE, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  button1 = gtk_button_new_from_stock ("gtk-clear");
  gtk_widget_show (button1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button1, A_RESET);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (button2);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button2, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  button3 = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (button3);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button3, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  button4 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (button4);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button4, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT (dialog1, gui_scale, "gui_scale");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, curve1, "curve1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea_histin, "drawingarea_histin");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea_histout, "drawingarea_histout");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, button1, "button1");
  GLADE_HOOKUP_OBJECT (dialog1, button2, "button2");
  GLADE_HOOKUP_OBJECT (dialog1, button3, "button3");
  GLADE_HOOKUP_OBJECT (dialog1, button4, "button4");

  return dialog1;
}

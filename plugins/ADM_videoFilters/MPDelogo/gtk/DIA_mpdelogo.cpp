/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

			    GUI for cropping including autocrop
			    +Revisted the Gtk2 way
			     +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
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

#include <math.h>

#include "ADM_toolkitGtk.h"


#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_colorspace.h"
#include "ADM_vidMPdelogo.h"

extern "C" {
#include "ADM_libraries/ADM_ffmpeg/ADM_lavcodec/avcodec.h"
}

static GtkWidget	*create_dialog1 (void);
static void  		ui_read ( void);
static void  		ui_update ( void);
static void 		draw (GtkWidget *dialog,uint32_t w,uint32_t h );
static gboolean 	gui_draw( void );
static void 		autocrop (void );
static void 		reset( void );
static void 		ui_upload(void);
static gboolean		ui_changed(void);


static void		prepare(uint32_t img);
static void 		frame_changed( void );

extern void GUI_RGBDisplay(uint8_t * dis, uint32_t w, uint32_t h, void *widg);

static ColYuvRgb    *rgbConv=NULL;
static uint8_t *working=NULL;
static uint8_t *rgbBufferDisplay=NULL;
static uint8_t *original=NULL;
static GtkWidget *dialog=NULL;
static uint32_t x,y,w,h,zoomW,zoomH,band;

static AVDMGenericVideoStream *incoming=NULL;
static ADMImage *imgsrc=NULL;
static ADMImageResizer *resizer=NULL;

static int lock=0,width,height;


//
//	Video is in YV12 Colorspace
//
//
uint8_t DIA_getMPdelogo(MPDELOGO_PARAM *param,AVDMGenericVideoStream *in)

{
	// Allocate space for green-ised video
	width=in->getInfo()->width;
	height=in->getInfo()->height;
		
	working=new uint8_t [width*height*4];	
	original=NULL;

	uint8_t ret=0;

	dialog=create_dialog1();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_OK,
										GTK_RESPONSE_CANCEL,
										-1);

	gtk_register_dialog(dialog);
	
	x=param->xoff;
	y=param->yoff;
	w=param->lw;
	h=param->lh;
	band=param->band;

	imgsrc=new ADMImage(width,height);
	incoming=in;
	
	rgbConv=new ColYuvRgb(width,height);
        rgbConv->reset(width,height);

	float zoom = UI_calcZoomToFitScreen(GTK_WINDOW(dialog), WID(drawingarea1), width, height);

	zoomW = width * zoom;
	zoomH = height * zoom;
	rgbBufferDisplay=new uint8_t[zoomW*zoomH*4];

	gtk_widget_set_usize(WID(drawingarea1), zoomW, zoomH);

	if (zoom < 1)
	{
		UI_centreCanvasWindow((GtkWindow*)dialog, WID(drawingarea1), zoomW, zoomH);
		resizer = new ADMImageResizer(width, height, zoomW, zoomH, PIX_FMT_RGB32, PIX_FMT_RGB32);
	}

	prepare(0);
	gtk_widget_show(dialog);
	
	ui_upload();
	ui_update();
	
#define CONNECT(x,y,z) 	gtk_signal_connect(GTK_OBJECT(WID(x)), #y,GTK_SIGNAL_FUNC(z),   NULL);

        CONNECT(drawingarea1,expose_event,gui_draw);
        CONNECT(hscale1,value_changed,frame_changed);	  
		      
#define CONNECT_SPIN(x) CONNECT(spinbutton##x, value_changed,ui_changed)
      	  
        CONNECT_SPIN(X);
        CONNECT_SPIN(Y);
        CONNECT_SPIN(W);
        CONNECT_SPIN(H);
        CONNECT_SPIN(Band);
	  
	draw(dialog,width,height);

	ret=0;
	int response;
	while( (response=gtk_dialog_run(GTK_DIALOG(dialog)))==GTK_RESPONSE_APPLY)
	{
		ui_changed();
		
	}
	if(response==GTK_RESPONSE_OK)
        {
		ui_read( );
		param->xoff=x;
                param->yoff=y;
                param->lw=w;
                param->lh=h;
                param->band=band;
		ret=1;
	}
        gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	delete working;	
	delete imgsrc;
	delete rgbConv;

	if (resizer)
	{
		delete resizer;
		delete[] rgbBufferDisplay;

		resizer=NULL;
		rgbBufferDisplay=NULL;
	}

	working=NULL;
	dialog=NULL;
	original=NULL;
	imgsrc=NULL;
	return ret;
}
void frame_changed( void )
{
uint32_t new_frame,max,l,f;
double   percent;
GtkWidget *wid;	
GtkAdjustment *adj;

        max=incoming->getInfo()->nb_frames;
        wid=WID(hscale1);
        adj=gtk_range_get_adjustment (GTK_RANGE(wid));
        new_frame=0;
        
        percent=(double)GTK_ADJUSTMENT(adj)->value;
        percent*=max;
        percent/=100.;
        new_frame=(uint32_t)floor(percent);
        
        if(new_frame>=max) new_frame=max-1;
        
        prepare(new_frame);
        ui_update();
        gui_draw();


}
void prepare(uint32_t img)
{
	uint32_t l,f;
	
	ADM_assert(incoming->getFrameNumberNoAlloc(img,&l,imgsrc,&f));
	original=imgsrc->data;
	

}
gboolean ui_changed(void)
{
	if(!lock)
	{
		ui_read();
		memcpy(working,original,(width*height*3)>>1);
		ui_update();
		draw(dialog,width,height);
	}
		return true;
}


/*---------------------------------------------------------------------------
	Actually draw the working frame on screen
*/
gboolean gui_draw( void )
{
	draw(dialog,width,height);
	return true;
}
void draw (GtkWidget *dialog,uint32_t w,uint32_t h )
{
	GtkWidget *draw=WID(drawingarea1);

	if (resizer)
	{
		resizer->resize(working, rgbBufferDisplay);
		GUI_RGBDisplay(rgbBufferDisplay, zoomW, zoomH, (void*)draw);
	}
	else
		GUI_RGBDisplay(working, w, h, (void*)draw);
}
/*---------------------------------------------------------------------------
	Read entried from dialog box
*/

#define SPIN_GET(x,y) {x= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(WID(spinbutton##y))) ;}
#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbutton##y)),(gfloat)x) ;}


void ui_read (void )
{
	int reject=0;
	
			SPIN_GET(x,X);
			SPIN_GET(y,Y);
			SPIN_GET(h,H);
			SPIN_GET(w,W);
                        SPIN_GET(band,Band);
			
			//printf("%d %d %d %d\n",x,y,w,h);
			
			x&=0xffffe;
			y&=0xffffe;
			h&=0xffffe;
			w&=0xffffe;
			
			if((x+w)>width)
				{
                                        if(w>=width) w=width;
                                        x=width-w;
                                        reject=1;
				}
			if((y+h)>height)
				{
                                        if(h>=height) h=height;
                                        y=height-h;
                                        reject=1;
				}
			if(reject)
				ui_upload();
}

void ui_upload(void)
{
	lock++;
	SPIN_SET(x,X);
	SPIN_SET(y,Y);
	SPIN_SET(w,W);
	SPIN_SET(h,H);
        SPIN_SET(band,Band);
	lock--;
}
//____________________________________
void reset( void )
{
	
	x=y=w=h=0;
	ui_upload();
	ui_update();
	gui_draw();
	

}
/*---------------------------------------------------------------------------
	Green-ify the displayed frame on cropped parts
*/
void ui_update( )
{

        uint8_t  *in,*in2;
        uint8_t *buffer=working;

        rgbConv->scale(original,buffer);
        // Buffer is in RGB space
        in=buffer+x*4+y*width*4;
        in2=buffer+(x+w)*4+y*width*4;
        
        for(int yy=0;yy<h;yy++)
        {
          in[0]=in[2]=0;in[1]=0xff;
          in2[0]=in2[2]=0;in2[1]=0xff;
          in+=width*4;
          in2+=width*4;
        }
        in=buffer+(y*width+x)*4;
        in2=buffer+((y+h)*width+x)*4;
        for(int yy=0;yy<w;yy++)
        {
          in[0]=in[2]=0;in[1]=0xff;
          in2[0]=in2[2]=0;in2[1]=0xff;
          in+=4;
          in2+=4;
        }

}

//--------------------------------------------
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *table2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkObject *spinbuttonX_adj;
  GtkWidget *spinbuttonX;
  GtkObject *spinbuttonY_adj;
  GtkWidget *spinbuttonY;
  GtkObject *spinbuttonW_adj;
  GtkWidget *spinbuttonW;
  GtkObject *spinbuttonH_adj;
  GtkWidget *spinbuttonH;
  GtkWidget *hseparator1;
  GtkWidget *hseparator2;
  GtkObject *spinbuttonBand_adj;
  GtkWidget *spinbuttonBand;
  GtkWidget *hseparator3;
  GtkWidget *hscale1;
  GtkWidget *drawingarea1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Mplayer Delogo"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), vbox2, FALSE, FALSE, 0);

  table2 = gtk_table_new (3, 4, FALSE);
  gtk_widget_show (table2);
  gtk_box_pack_start (GTK_BOX (vbox2), table2, TRUE, TRUE, 0);

  label3 = gtk_label_new (QT_TR_NOOP("X"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table2), label3, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label4 = gtk_label_new (QT_TR_NOOP("Y"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table2), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label5 = gtk_label_new (QT_TR_NOOP("W"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table2), label5, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  label6 = gtk_label_new (QT_TR_NOOP("H"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table2), label6, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label7 = gtk_label_new (QT_TR_NOOP("Band"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  spinbuttonX_adj = gtk_adjustment_new (1, 0, 2000, 1, 10, 10);
  spinbuttonX = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonX_adj), 1, 0);
  gtk_widget_show (spinbuttonX);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonX, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  spinbuttonY_adj = gtk_adjustment_new (1, 0, 2000, 1, 10, 10);
  spinbuttonY = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonY_adj), 1, 0);
  gtk_widget_show (spinbuttonY);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonY, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  spinbuttonW_adj = gtk_adjustment_new (1, 0, 2000, 1, 10, 10);
  spinbuttonW = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonW_adj), 1, 0);
  gtk_widget_show (spinbuttonW);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonW, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  spinbuttonH_adj = gtk_adjustment_new (1, 0, 2000, 1, 10, 10);
  spinbuttonH = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonH_adj), 1, 0);
  gtk_widget_show (spinbuttonH);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonH, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_table_attach (GTK_TABLE (table2), hseparator1, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_show (hseparator2);
  gtk_table_attach (GTK_TABLE (table2), hseparator2, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  spinbuttonBand_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
  spinbuttonBand = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBand_adj), 1, 0);
  gtk_widget_show (spinbuttonBand);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonBand, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hseparator3 = gtk_hseparator_new ();
  gtk_widget_show (hseparator3);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator3, FALSE, FALSE, 0);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 0)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, FALSE, FALSE, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_box_pack_start (GTK_BOX (vbox1), drawingarea1, TRUE, TRUE, 0);
  gtk_widget_set_size_request (drawingarea1, 100, 100);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, table2, "table2");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label4, "label4");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonX, "spinbuttonX");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonY, "spinbuttonY");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonW, "spinbuttonW");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonH, "spinbuttonH");
  GLADE_HOOKUP_OBJECT (dialog1, hseparator1, "hseparator1");
  GLADE_HOOKUP_OBJECT (dialog1, hseparator2, "hseparator2");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonBand, "spinbuttonBand");
  GLADE_HOOKUP_OBJECT (dialog1, hseparator3, "hseparator3");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}



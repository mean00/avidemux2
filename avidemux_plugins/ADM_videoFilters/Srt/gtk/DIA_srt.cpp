/***************************************************************************
                          ADM_guiSRT.cpp  -  description
                             -------------------
    begin                : Wed Dec 18 2002
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
#include "DIA_flyDialog.h"


#include "DIA_fileSel.h"

#include "ADM_videoFilter.h"

class ADMfont;
#include "ADM_vidSRT.h"
#include "DIA_flySrtPos.h"
#include "ADM_colorspace.h"



static void read( void );
static void upload ( void );
static gboolean slider_update( void );
static gboolean gui_update( void);
static gboolean gui_draw( void );
static GtkWidget *create_dialog1 (void);

static GtkWidget *dialog=NULL;
static flySrtPos *myCrop=NULL;
static int lock=0;

/**
      \fn DIA_srtPos
      \brief Dialog that handles subtitle size and position
*/
int DIA_srtPos(AVDMGenericVideoStream *in,uint32_t *size,uint32_t *position)
{
  uint8_t ret=0;
  
        uint32_t width,height;

        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        dialog=create_dialog1();

		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);

        gtk_register_dialog(dialog);
        gtk_window_set_title (GTK_WINDOW (dialog), QT_TR_NOOP("Subtitle Size and Position"));
        gtk_widget_show(dialog);
	
        myCrop=new flySrtPos( width, height,in,WID(drawingarea1),WID(hscale1));
        myCrop->param.fontSize=*size;
        myCrop->param.position=*position;

        gtk_range_set_range(GTK_RANGE(WID(vscale1)),0,height-1);

        myCrop->upload();
        myCrop->sliderChanged();
        
        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
            GTK_SIGNAL_FUNC(gui_draw),
            NULL);
        
        gtk_signal_connect (GTK_OBJECT(WID( spinbutton1)), "value_changed",
                    GTK_SIGNAL_FUNC (gui_update),
                    NULL);
        
         gtk_signal_connect (GTK_OBJECT(WID( hscale1)), "value_changed",
                    GTK_SIGNAL_FUNC (slider_update),
                    NULL);

          gtk_signal_connect (GTK_OBJECT(WID( vscale1)), "value_changed",
                    GTK_SIGNAL_FUNC (gui_update),
                    NULL);
       
        ret=0;
        int response;
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            *size=myCrop->param.fontSize;
            *position=myCrop->param.position;
            ret=1;
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        delete myCrop;
        return ret;
}
/**********************************/
void read( void )
{
	myCrop->download();
}
gboolean slider_update( void )
{
        myCrop->sliderChanged();
        return true;
}
gboolean gui_update( void)
{
  if(lock) return true;
      myCrop->update();
  return true;
}
gboolean gui_draw( void )
{
	myCrop->display();
	return true;
}

/******************************/
#define SPIN_GET(x,y) {y= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lookup_widget(dialog,#x))) ;\
				printf(#x":%d\n", y);}

#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(lookup_widget(dialog,#x)),(gfloat)y) ; printf(#x":%d\n", y);}


uint8_t    flySrtPos::upload(void)
{
        SPIN_SET(spinbutton1,param.fontSize);
        
        int32_t max=_h;
        max-=(SRT_MAX_LINE)*param.fontSize;
        if(max<0) max=0;
        if(param.position>=max)
        {
          param.position=max;
        }
        
        GtkAdjustment *adj=gtk_range_get_adjustment (GTK_RANGE(WID(vscale1)));
        GTK_ADJUSTMENT(adj)->value=param.position;
        return 1;
}
uint8_t    flySrtPos::download(void)
{
        SPIN_GET(spinbutton1,param.fontSize);
        GtkAdjustment *adj=gtk_range_get_adjustment (GTK_RANGE(WID(vscale1)));
        param.position=(uint32_t)GTK_ADJUSTMENT(adj)->value;
        
        int32_t max=_h;
        max-=(SRT_MAX_LINE)*param.fontSize;
        if(max<0) max=0;
        if(param.position>=max)
        {
          param.position=max;
          upload(); 
        }
        
        return 1;
}

/*
	Almost straigh out of glade2

*/

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label1;
  GtkObject *spinbutton1_adj;
  GtkWidget *spinbutton1;
  GtkWidget *hscale1;
  GtkWidget *hbox2;
  GtkWidget *drawingarea1;
  GtkWidget *vscale1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Subtitle Size and Position"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("Font Size:"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);

  spinbutton1_adj = gtk_adjustment_new (1, 6, 99, 1, 10, 10);
  spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 2);
  gtk_widget_show (spinbutton1);
  gtk_box_pack_start (GTK_BOX (hbox1), spinbutton1, FALSE, FALSE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton1), TRUE);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 99, 1, 1, 1)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, FALSE, FALSE, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, TRUE, TRUE, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_box_pack_start (GTK_BOX (hbox2), drawingarea1, TRUE, TRUE, 0);

  vscale1 = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 100, 1, 1, 1, 1)));
  gtk_widget_show (vscale1);
  gtk_box_pack_start (GTK_BOX (hbox2), vscale1, FALSE, FALSE, 0);
  gtk_scale_set_digits (GTK_SCALE (vscale1), 0);

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
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton1, "spinbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT (dialog1, vscale1, "vscale1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}

//-------------------------------

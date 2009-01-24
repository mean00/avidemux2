/***************************************************************************
                          ADM_guiChromaShift.cpp  -  description
                             -------------------
    begin                : Sun Aug 24 2003
    copyright            : (C) 2002-2003 by mean
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


#include "ADM_image.h"
#include "ADM_videoFilter.h"

#include "ADM_assert.h"
#include "DIA_flyDialog.h"
#include "../ADM_vidChromaShift_param.h"
#include "DIA_flyChromaShift.h"

static int lock=0;

static GtkWidget *dialog;
static GtkWidget *create_ChromaShift( void );


static void read( void );
static void upload ( void );
static gboolean gui_draw( void );
static gboolean gui_update( void );
static gboolean slider_update( void );
static void update(void);

static flyChromaShift *myCrop=NULL;

//**************************************

uint8_t DIA_getChromaShift( AVDMGenericVideoStream *instream,CHROMASHIFT_PARAM    *param );
uint8_t DIA_getChromaShift( AVDMGenericVideoStream *in,CHROMASHIFT_PARAM    *param )
{
uint8_t ret=0;
  
        uint32_t width,height;

        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        dialog=create_ChromaShift();
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
											GTK_RESPONSE_OK,
											GTK_RESPONSE_CANCEL,
											GTK_RESPONSE_APPLY,
											-1);
        gtk_register_dialog(dialog);
        gtk_window_set_title (GTK_WINDOW (dialog), QT_TR_NOOP("Chroma Shift"));
        gtk_widget_show(dialog);
	
        myCrop=new flyChromaShift( width, height,in,WID(drawingarea1),WID(hscale));
        memcpy(&(myCrop->param),param,sizeof(CHROMASHIFT_PARAM));
        myCrop->upload();
        myCrop->sliderChanged();
        
        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
            GTK_SIGNAL_FUNC(gui_draw),
            NULL);
        
        gtk_signal_connect (GTK_OBJECT(WID( spinbutton_U)), "value_changed",
                    GTK_SIGNAL_FUNC (gui_update),
                    NULL);
        gtk_signal_connect (GTK_OBJECT(WID( spinbutton_V)), "value_changed",
                    GTK_SIGNAL_FUNC (gui_update),
                    NULL);
        gtk_signal_connect (GTK_OBJECT(WID( spinbutton_V)), "value_changed",
                    GTK_SIGNAL_FUNC (gui_update),
                    NULL);
         gtk_signal_connect (GTK_OBJECT(WID( hscale)), "value_changed",
                    GTK_SIGNAL_FUNC (slider_update),
                    NULL);

          
       
        ret=0;
        int response;
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            memcpy(param,&(myCrop->param),sizeof(CHROMASHIFT_PARAM));
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
void upload ( void )
{
	myCrop->upload();
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


uint8_t    flyChromaShift::upload(void)
{
        SPIN_SET(spinbutton_U,param.u);
        SPIN_SET(spinbutton_V,param.v);
  return 1;
}
uint8_t    flyChromaShift::download(void)
{
        SPIN_GET(spinbutton_U,param.u);
        SPIN_GET(spinbutton_V,param.v);
  
  return 1;
}


/*----------------------------------------------------------------*/

GtkWidget*
create_ChromaShift (void)
{
  GtkWidget *ChromaShift;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkObject *spinbutton_U_adj;
  GtkWidget *spinbutton_U;
  GtkObject *spinbutton_V_adj;
  GtkWidget *spinbutton_V;
  GtkWidget *hscale;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *drawingarea1;
  GtkWidget *label3;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *applybutton1;
  GtkWidget *okbutton1;

  ChromaShift = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (ChromaShift), QT_TR_NOOP("ChromaShift"));
  gtk_window_set_type_hint (GTK_WINDOW (ChromaShift), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (ChromaShift)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("U Shift :"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (QT_TR_NOOP("V Shift :"));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  spinbutton_U_adj = gtk_adjustment_new (0, -32, 32, 1, 10, 10);
  spinbutton_U = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_U_adj), 1, 0);
  gtk_widget_show (spinbutton_U);
  gtk_table_attach (GTK_TABLE (table1), spinbutton_U, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_U), TRUE);

  spinbutton_V_adj = gtk_adjustment_new (0, -32, 32, 1, 10, 10);
  spinbutton_V = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_V_adj), 1, 0);
  gtk_widget_show (spinbutton_V);
  gtk_table_attach (GTK_TABLE (table1), spinbutton_V, 1, 2, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton_V), TRUE);

  hscale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 1)));
  gtk_widget_show (hscale);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale, FALSE, FALSE, 0);
  gtk_scale_set_digits (GTK_SCALE (hscale), 0);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_container_add (GTK_CONTAINER (alignment1), drawingarea1);

  label3 = gtk_label_new (QT_TR_NOOP("<b>Preview</b>"));
  gtk_widget_show (label3);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label3);
  gtk_label_set_use_markup (GTK_LABEL (label3), TRUE);

  dialog_action_area1 = GTK_DIALOG (ChromaShift)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (ChromaShift), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  applybutton1 = gtk_button_new_from_stock ("gtk-apply");
  gtk_widget_show (applybutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (ChromaShift), applybutton1, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (applybutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (ChromaShift), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (ChromaShift, ChromaShift, "ChromaShift");
  GLADE_HOOKUP_OBJECT_NO_REF (ChromaShift, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (ChromaShift, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (ChromaShift, table1, "table1");
  GLADE_HOOKUP_OBJECT (ChromaShift, label1, "label1");
  GLADE_HOOKUP_OBJECT (ChromaShift, label2, "label2");
  GLADE_HOOKUP_OBJECT (ChromaShift, spinbutton_U, "spinbutton_U");
  GLADE_HOOKUP_OBJECT (ChromaShift, spinbutton_V, "spinbutton_V");
  GLADE_HOOKUP_OBJECT (ChromaShift, hscale, "hscale");
  GLADE_HOOKUP_OBJECT (ChromaShift, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (ChromaShift, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (ChromaShift, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT (ChromaShift, label3, "label3");
  GLADE_HOOKUP_OBJECT_NO_REF (ChromaShift, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (ChromaShift, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (ChromaShift, applybutton1, "applybutton1");
  GLADE_HOOKUP_OBJECT (ChromaShift, okbutton1, "okbutton1");

  return ChromaShift;
}



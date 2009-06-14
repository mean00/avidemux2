/***************************************************************************
                           Fly-Ui for hue & sat

    copyright            : (C) 2004/5/7 by mean
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


#include "ADM_image.h"
#include "ADM_videoFilter.h"

#include "ADM_vidEq2.h"

#include "DIA_flyEq2.h"


static GtkWidget*create_dialog1 (void);
static float getAdj(GtkWidget *widget);
static void setAdj(GtkWidget *widget,float val);

static GtkWidget *dialog=NULL;

static void             update ( void);
static gboolean         draw (void );
static void             eq2_changed( void);
static void 			frame_changed( void );

static int lock=0;
static flyEq2 *myCrop=NULL;

/**
 * 		\fn DIA_getEQ2Param
 * 		\brief flyDialogGtk handling the mplayer EQ2 user Interface dialog.
 */
uint8_t DIA_getEQ2Param(Eq2_Param *param, AVDMGenericVideoStream *in)
{
uint8_t r=0;
uint32_t w,h;
        // Allocate space for green-ised video
        w=in->getInfo()->width;
        h=in->getInfo()->height;

        dialog=create_dialog1();
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
											GTK_RESPONSE_OK,
											GTK_RESPONSE_CANCEL,
											-1);
        gtk_register_dialog(dialog);
        gtk_widget_show(dialog);	
        
        myCrop=new flyEq2( w, h,in,WID(drawingarea1),WID(hscale1));
        memcpy(&(myCrop->param),param,sizeof(Eq2_Param));
        myCrop->upload();
        myCrop->sliderChanged();
        myCrop->update();
        
        int ret=0;
        int response;
        //-----------------------
        
#define HCONECT(x)  gtk_signal_connect(GTK_OBJECT(WID(hscale##x)), "value_changed",GTK_SIGNAL_FUNC(eq2_changed),   NULL);

        HCONECT(Brightness);
        HCONECT(Saturation);
        HCONECT(Contrast);

        HCONECT(Gamma);        
        HCONECT(GammaWeight);
        HCONECT(GammaR);
        HCONECT(GammaG);
        HCONECT(GammaB);
        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event", GTK_SIGNAL_FUNC(draw),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(hscale1)), "value_changed",GTK_SIGNAL_FUNC(frame_changed),   NULL);
        //-----------------------
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            memcpy(param,&(myCrop->param),sizeof(Eq2_Param));
            ret=1;
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        delete myCrop;
        myCrop=NULL;
        return ret;

}
/**************************************/
#undef SET
#undef GET
#define SET(x,y)        gtk_range_set_value (GTK_RANGE(WID(hscale##x)),(gdouble)param.y)
#define GET(x,y)        param.y= (float)gtk_range_get_value (GTK_RANGE(WID(hscale##x)))


uint8_t flyEq2::upload(void)
{
  lock++;
  SET(Contrast,contrast);
  SET(Brightness,brightness);
  SET(Saturation,saturation);

  SET(Gamma,gamma);        
  SET(GammaWeight,gamma_weight);
  SET(GammaR,rgamma);
  SET(GammaG,ggamma);
  SET(GammaB,bgamma);
  
  lock--;
  return 1;
}
uint8_t flyEq2::download (void)
{

  GET(Contrast,contrast);
  GET(Brightness,brightness);
  GET(Saturation,saturation);

  GET(Gamma,gamma);        
  GET(GammaWeight,gamma_weight);
  GET(GammaR,rgamma);
  GET(GammaG,ggamma);
  GET(GammaB,bgamma);

 return 1;
}

/**********************/
void frame_changed( void )
{
          myCrop->sliderChanged();
}
void eq2_changed( void)
{
    if(lock) return;
    myCrop->update();
}
gboolean draw (void)
{
    myCrop->display();
	return true;
}
//**

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *frame1;
  GtkWidget *table2;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *hscaleBrightness;
  GtkWidget *hscaleSaturation;
  GtkWidget *hscaleContrast;
  GtkWidget *label3;
  GtkWidget *label1;
  GtkWidget *frame2;
  GtkWidget *table3;
  GtkWidget *label6;
  GtkWidget *hscaleGamma;
  GtkWidget *hscaleGammaR;
  GtkWidget *label8;
  GtkWidget *hscaleGammaG;
  GtkWidget *label9;
  GtkWidget *label10;
  GtkWidget *label7;
  GtkWidget *hscaleGammaB;
  GtkWidget *hscaleGammaWeight;
  GtkWidget *label2;
  GtkWidget *hscale1;
  GtkWidget *drawingarea1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("MPlayer eq2"));
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox1), frame1, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

  table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (frame1), table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 12);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 12);

  label4 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Brigh_tness:"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table2), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label5 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Saturation:"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table2), label5, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  hscaleBrightness = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -1, 1, 0.05, 1, 0)));
  gtk_widget_show (hscaleBrightness);
  gtk_table_attach (GTK_TABLE (table2), hscaleBrightness, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleBrightness, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleBrightness), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleBrightness), 2);

  hscaleSaturation = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 3, 0.05, 1, 0)));
  gtk_widget_show (hscaleSaturation);
  gtk_table_attach (GTK_TABLE (table2), hscaleSaturation, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleSaturation, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleSaturation), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleSaturation), 2);

  hscaleContrast = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, -2, 2, 0.05, 1, 0)));
  gtk_widget_show (hscaleContrast);
  gtk_table_attach (GTK_TABLE (table2), hscaleContrast, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleContrast, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleContrast), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleContrast), 2);

  label3 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Contrast:"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table2), label3, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label1 = gtk_label_new ("");
  gtk_widget_show (label1);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label1);
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox1), frame2, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

  table3 = gtk_table_new (5, 2, FALSE);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (frame2), table3);
  gtk_container_set_border_width (GTK_CONTAINER (table3), 12);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 12);

  label6 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Initial:"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table3), label6, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  hscaleGamma = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.1, 3, 0.05, 1, 0)));
  gtk_widget_show (hscaleGamma);
  gtk_table_attach (GTK_TABLE (table3), hscaleGamma, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleGamma, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleGamma), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleGamma), 2);

  hscaleGammaR = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.1, 3, 0.05, 1, 0)));
  gtk_widget_show (hscaleGammaR);
  gtk_table_attach (GTK_TABLE (table3), hscaleGammaR, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleGammaR, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleGammaR), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleGammaR), 2);

  label8 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Red:"));
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table3), label8, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  hscaleGammaG = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.1, 3, 0.05, 1, 0)));
  gtk_widget_show (hscaleGammaG);
  gtk_table_attach (GTK_TABLE (table3), hscaleGammaG, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleGammaG, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleGammaG), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleGammaG), 2);

  label9 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Green:"));
  gtk_widget_show (label9);
  gtk_table_attach (GTK_TABLE (table3), label9, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  label10 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Blue:"));
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table3), label10, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  label7 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Weight:"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table3), label7, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  hscaleGammaB = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.1, 3, 0.05, 1, 0)));
  gtk_widget_show (hscaleGammaB);
  gtk_table_attach (GTK_TABLE (table3), hscaleGammaB, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleGammaB, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleGammaB), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleGammaB), 2);

  hscaleGammaWeight = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1, 0.05, 1, 0)));
  gtk_widget_show (hscaleGammaWeight);
  gtk_table_attach (GTK_TABLE (table3), hscaleGammaWeight, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_widget_set_size_request (hscaleGammaWeight, 144, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleGammaWeight), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (hscaleGammaWeight), 2);

  label2 = gtk_label_new (QT_TR_NOOP("<b>Gamma</b>"));
  gtk_widget_show (label2);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label2);
  gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 0)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, FALSE, FALSE, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_box_pack_start (GTK_BOX (vbox1), drawingarea1, TRUE, TRUE, 0);

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

  gtk_label_set_mnemonic_widget (GTK_LABEL (label4), hscaleBrightness);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label5), hscaleSaturation);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label3), hscaleContrast);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label6), hscaleGamma);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label8), hscaleGammaR);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label9), hscaleGammaG);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label10), hscaleGammaB);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label7), hscaleGammaWeight);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, table2, "table2");
  GLADE_HOOKUP_OBJECT (dialog1, label4, "label4");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleBrightness, "hscaleBrightness");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleSaturation, "hscaleSaturation");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleContrast, "hscaleContrast");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, table3, "table3");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleGamma, "hscaleGamma");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleGammaR, "hscaleGammaR");
  GLADE_HOOKUP_OBJECT (dialog1, label8, "label8");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleGammaG, "hscaleGammaG");
  GLADE_HOOKUP_OBJECT (dialog1, label9, "label9");
  GLADE_HOOKUP_OBJECT (dialog1, label10, "label10");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleGammaB, "hscaleGammaB");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleGammaWeight, "hscaleGammaWeight");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


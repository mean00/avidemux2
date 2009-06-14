/**/
/***************************************************************************
                          DIA_hue
                             -------------------

                           Ui for hue & sat

    begin                : 08 Apr 2005
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



#include "ADM_toolkitGtk.h"


#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_vidASharp_param.h"
#include "DIA_flyDialog.h"
#include "DIA_flyAsharp.h"


uint8_t DIA_getASharp(ASHARP_PARAM *param, AVDMGenericVideoStream *in);

static GtkWidget        *create_dialog1 (void);
static gboolean         draw (void );
static void             upload(void);
static void             download(void);
static void             frame_changed( void );
static void             hue_changed( void);

static int lock=0;
static GtkWidget *dialog=NULL;
static flyASharp *myCrop=NULL;

//
//      Video is in YV12 Colorspace
//
//
uint8_t DIA_getASharp(ASHARP_PARAM *param, AVDMGenericVideoStream *in)
{
      uint32_t width,height;
      uint8_t ret=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        dialog=create_dialog1();
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);
        gtk_register_dialog(dialog);
        gtk_window_set_title (GTK_WINDOW (dialog), QT_TR_NOOP("ASHARP"));
        gtk_widget_show(dialog);

        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
            GTK_SIGNAL_FUNC(draw),
            NULL);
//
        gtk_signal_connect(GTK_OBJECT(WID(hscale1)), "value_changed",GTK_SIGNAL_FUNC(frame_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(spinbuttonT)), "value_changed",GTK_SIGNAL_FUNC(hue_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(spinbuttonD)), "value_changed",GTK_SIGNAL_FUNC(hue_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(spinbuttonB)), "value_changed",GTK_SIGNAL_FUNC(hue_changed),   NULL);
        gtk_widget_show(dialog);

          
        myCrop=new flyASharp( width, height,in,WID(drawingarea1),WID(hscale1));
        memcpy(&(myCrop->param),param,sizeof(ASHARP_PARAM));
        myCrop->upload();
        myCrop->sliderChanged();
        ret=0;
        int response;
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            memcpy(param,&(myCrop->param),sizeof(ASHARP_PARAM));
            ret=1;
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        delete myCrop;
        return ret;
}
void hue_changed( void)
{
  if(lock) return;
  
   myCrop->update();
  
}
void frame_changed( void )
{
  myCrop->sliderChanged();
}
gboolean draw (void)
{
    myCrop->display();
}
//
#define SPIN_GET(x,y) {param.x= gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(y))) ;}
#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(y)),(gfloat)param.x) ;}

#define CHECK_GET(x,y) {param.x=gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(WID(y)));}
#define CHECK_SET(x,y) {gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(y)),param.x);}

void upload( void)
{
    myCrop->upload();
}
void download (void)
{
     myCrop->download();
}
/**************************************/
uint8_t flyASharp::upload(void)
{
    lock++;
        SPIN_SET(t,spinbuttonT);
        SPIN_SET(d,spinbuttonD);
        SPIN_SET(b,spinbuttonB);

        CHECK_SET(bf,checkbuttonBF);
    lock--;
        return 1;
}
uint8_t flyASharp::download (void)
{
    
        SPIN_GET(t,spinbuttonT);
        SPIN_GET(d,spinbuttonD);
        SPIN_GET(b,spinbuttonB);

        CHECK_GET(bf,checkbuttonBF);
        return 1;
}

/**************************************************/
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkObject *spinbuttonT_adj;
  GtkWidget *spinbuttonT;
  GtkObject *spinbuttonD_adj;
  GtkWidget *spinbuttonD;
  GtkObject *spinbuttonB_adj;
  GtkWidget *spinbuttonB;
  GtkWidget *checkbuttonBF;
  GtkWidget *hscale1;
  GtkWidget *drawingarea1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("ASharp by MarcFD"));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table1 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("Threshold :"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (QT_TR_NOOP("Strength :"));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  label3 = gtk_label_new (QT_TR_NOOP("Block Adaptative :"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label4 = gtk_label_new (QT_TR_NOOP("Unknown flag :"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  spinbuttonT_adj = gtk_adjustment_new (1, -1, 100, 0.5, 0.1, 0.1);
  spinbuttonT = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonT_adj), 1, 1);
  gtk_widget_show (spinbuttonT);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonT, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonT), TRUE);

  spinbuttonD_adj = gtk_adjustment_new (1, -1, 100, 0.5, 0.1, 0.1);
  spinbuttonD = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonD_adj), 1, 1);
  gtk_widget_show (spinbuttonD);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonD, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonD), TRUE);

  spinbuttonB_adj = gtk_adjustment_new (1, -1, 100, 0.5, 0.1, 0.1);
  spinbuttonB = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonB_adj), 1, 1);
  gtk_widget_show (spinbuttonB);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonB, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonB), TRUE);

  checkbuttonBF = gtk_check_button_new_with_mnemonic (QT_TR_NOOP(" "));
  gtk_widget_show (checkbuttonBF);
  gtk_table_attach (GTK_TABLE (table1), checkbuttonBF, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 1)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, TRUE, TRUE, 0);

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

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, table1, "table1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label4, "label4");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonT, "spinbuttonT");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonD, "spinbuttonD");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonB, "spinbuttonB");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonBF, "checkbuttonBF");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


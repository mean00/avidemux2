/**/
/***************************************************************************
                          DIA_hue
                             -------------------

			   Ui for hue & sat

    begin                : 08 Apr 2005
    copyright            : (C) 2004/7 by mean
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
#include "ADM_vidHue.h"

#include "DIA_flyHue.h"


static GtkWidget	*create_dialog1 (void);
static void  		update ( void);
static gboolean 	draw (void );
static void 		upload(void);
static void 		download(void);
static void 		frame_changed( void );
static void             hue_changed( void);

static int lock=0;
static GtkWidget *dialog=NULL;
static flyHue *myCrop=NULL;

/**
      \fn DIA_getHue
      \brief Handle Hue (fly)Dialog
*/
uint8_t DIA_getHue(Hue_Param *param, AVDMGenericVideoStream *in)
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
        
        gtk_window_set_title (GTK_WINDOW (dialog), QT_TR_NOOP("Hue"));

        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
            GTK_SIGNAL_FUNC(draw),
            NULL);
//
        gtk_signal_connect(GTK_OBJECT(WID(hscale1)), "value_changed",GTK_SIGNAL_FUNC(frame_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(hscaleHue)), "value_changed",GTK_SIGNAL_FUNC(hue_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(hscaleSaturation)), "value_changed",GTK_SIGNAL_FUNC(hue_changed),   NULL);
        gtk_widget_show(dialog);
          
        myCrop=new flyHue( width, height,in,WID(drawingarea1),WID(hscale1));
        memcpy(&(myCrop->param),param,sizeof(Hue_Param));
        myCrop->upload();
        myCrop->sliderChanged();
        ret=0;
        int response;
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            memcpy(param,&(myCrop->param),sizeof(Hue_Param));
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
void upload( void)
{
    myCrop->upload();
}
void download (void)
{
     myCrop->download();
}
/**************************************/
uint8_t flyHue::upload(void)
{
    lock++;
      gtk_range_set_value (GTK_RANGE(WID(hscaleHue)),(gdouble)param.hue);
      gtk_range_set_value (GTK_RANGE(WID(hscaleSaturation)),(gdouble)param.saturation);
    lock--;
        return 1;
}
uint8_t flyHue::download (void)
{
    
       GtkAdjustment *adj;	

        adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleHue)));
        param.hue=(float)GTK_ADJUSTMENT(adj)->value;
        
        adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleSaturation)));
        param.saturation=(float)GTK_ADJUSTMENT(adj)->value;
        return 1;
}




//******************
GtkWidget   *create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkWidget *hscaleHue;
  GtkWidget *hscaleSaturation;
  GtkWidget *hscale1;
  GtkWidget *drawingarea1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Hue/Saturation"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("<b>Hue:</b>"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (QT_TR_NOOP("<b>Saturation:</b>"));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  hscaleHue = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -90, 90, 1, 1, 1)));
  gtk_widget_show (hscaleHue);
  gtk_table_attach (GTK_TABLE (table1), hscaleHue, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  hscaleSaturation = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -10, 10, 1, 1, 1)));
  gtk_widget_show (hscaleSaturation);
  gtk_table_attach (GTK_TABLE (table1), hscaleSaturation, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

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
  GLADE_HOOKUP_OBJECT (dialog1, hscaleHue, "hscaleHue");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleSaturation, "hscaleSaturation");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}

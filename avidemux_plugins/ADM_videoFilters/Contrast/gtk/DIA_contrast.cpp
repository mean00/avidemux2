/***************************************************************************
                          DIA_contrast.cpp  -  description
                             -------------------
    begin                : Mon Sep 23 2002
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
#include "ADM_videoFilter.h"
#include "ADM_vidContrast.h"


#include "DIA_flyDialog.h"
#include "DIA_flyContrast.h"

/********************************************************************/
static GtkWidget *create_dialog1 (void);
static GtkWidget *dialog = NULL;

static gboolean gui_draw (GtkWidget * widget,
			  GdkEventExpose * event, gpointer user_data);
static void gui_update (GtkButton * button, gpointer user_data);
static void frame_changed( void );

static int lock=0;
static flyContrast *myCrop=NULL;

/********************************************************************/
uint8_t DIA_contrast(AVDMGenericVideoStream *in,CONTRAST_PARAM *param)
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
        gtk_window_set_title (GTK_WINDOW (dialog), QT_TR_NOOP("Contrast"));
        gtk_widget_show(dialog);	
        
          // and value changed
#define CNX(x,y) gtk_signal_connect(GTK_OBJECT(WID(x)), y, GTK_SIGNAL_FUNC(gui_update), (void *) (1));

        CNX (checkLuma, "toggled");
        CNX (checkbuttonU, "toggled");
        CNX (checkbuttonV, "toggled");
        
        
        CNX (hscaleContrast, "value_changed");
        CNX (hscaleContrast, "drag_data_received");
      
        CNX (hscaleBright, "value_changed");
        CNX (hscaleBright, "drag_data_received");
        gtk_signal_connect(GTK_OBJECT(WID(hscale1)), "value_changed",GTK_SIGNAL_FUNC(frame_changed),   NULL);
        gtk_signal_connect(GTK_OBJECT(WID(drawingarea1)), "expose_event",
            GTK_SIGNAL_FUNC(gui_draw),
            NULL);


          
        myCrop=new flyContrast( width, height,in,WID(drawingarea1),WID(hscale1));
        memcpy(&(myCrop->param),param,sizeof(CONTRAST_PARAM));
        myCrop->upload();
        myCrop->sliderChanged();
        ret=0;
        int response;
        response=gtk_dialog_run(GTK_DIALOG(dialog));

        if(response==GTK_RESPONSE_OK)
        {
            myCrop->download();
            memcpy(param,&(myCrop->param),sizeof(CONTRAST_PARAM));
            ret=1;
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        delete myCrop;
        return ret;
}


//
//        Check entered values and green-out the selected portion on the screen
//        Each value must be even
//
void frame_changed( void )
{
  myCrop->sliderChanged();
}
void gui_update (GtkButton * button, gpointer user_data)
{
  if(lock) return;
    myCrop->update();
}

gboolean gui_draw (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
  myCrop->display();
  return TRUE;
}

/**************************************/
#define CHECKBOX(x,y) if(TRUE==gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog,#x))))  \
			y=1; else y=0;
#define SCHECKBOX(x,y) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(lookup_widget(dialog,#x)),y)


uint8_t flyContrast::upload(void)
{
    lock++;
      SCHECKBOX (checkLuma, param.doLuma);
      SCHECKBOX (checkbuttonU, param.doChromaU);
      SCHECKBOX (checkbuttonV, param.doChromaV);

      GtkAdjustment *adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleBright)));
      GTK_ADJUSTMENT(adj)->value=param.offset;
  
      adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleContrast)));
      GTK_ADJUSTMENT(adj)->value=param.coef;
    lock--;
    return 1;
}
uint8_t flyContrast::download (void)
{
  CHECKBOX (checkLuma, param.doLuma);
  CHECKBOX (checkbuttonU, param.doChromaU);
  CHECKBOX (checkbuttonV, param.doChromaV);
  
  GtkAdjustment *adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleBright)));
  param.offset= (int32_t)GTK_ADJUSTMENT(adj)->value;
  
  adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscaleContrast)));
  param.coef=GTK_ADJUSTMENT(adj)->value;
   return 1;
}

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *vbox3;
  GtkWidget *label1;
  GtkWidget *hscaleContrast;
  GtkWidget *label2;
  GtkWidget *hscaleBright;
  GtkWidget *vbox2;
  GtkWidget *checkLuma;
  GtkWidget *checkbuttonU;
  GtkWidget *checkbuttonV;
  GtkWidget *hscale1;
  GtkWidget *drawingarea1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Contrast"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("Contrast"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox3), label1, FALSE, FALSE, 0);

  hscaleContrast = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.5, 1.5, 0.1, 0.1, 0)));
  gtk_widget_show (hscaleContrast);
  gtk_box_pack_start (GTK_BOX (vbox3), hscaleContrast, FALSE, TRUE, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleContrast), GTK_POS_LEFT);

  label2 = gtk_label_new (QT_TR_NOOP("Brightness"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox3), label2, FALSE, FALSE, 0);

  hscaleBright = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -127, 127, 1, 1, 0)));
  gtk_widget_show (hscaleBright);
  gtk_box_pack_start (GTK_BOX (vbox3), hscaleBright, FALSE, TRUE, 0);
  gtk_scale_set_value_pos (GTK_SCALE (hscaleBright), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (hscaleBright), 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  checkLuma = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("Luma"));
  gtk_widget_show (checkLuma);
  gtk_box_pack_start (GTK_BOX (vbox2), checkLuma, FALSE, FALSE, 0);

  checkbuttonU = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("Chroma U"));
  gtk_widget_show (checkbuttonU);
  gtk_box_pack_start (GTK_BOX (vbox2), checkbuttonU, FALSE, FALSE, 0);

  checkbuttonV = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("Chroma v"));
  gtk_widget_show (checkbuttonV);
  gtk_box_pack_start (GTK_BOX (vbox2), checkbuttonV, FALSE, FALSE, 0);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 99, 1, 1, 1)));
  gtk_widget_show (hscale1);
  gtk_box_pack_start (GTK_BOX (vbox1), hscale1, TRUE, TRUE, 0);

  drawingarea1 = gtk_drawing_area_new ();
  gtk_widget_show (drawingarea1);
  gtk_box_pack_start (GTK_BOX (vbox1), drawingarea1, TRUE, TRUE, 0);
  gtk_widget_set_size_request (drawingarea1, -1, 300);

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
  GLADE_HOOKUP_OBJECT (dialog1, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleContrast, "hscaleContrast");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, hscaleBright, "hscaleBright");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, checkLuma, "checkLuma");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonU, "checkbuttonU");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonV, "checkbuttonV");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, drawingarea1, "drawingarea1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


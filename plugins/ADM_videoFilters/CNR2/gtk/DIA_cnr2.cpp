//
// C++ Implementation: DIA_Msmooth
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
//

#include "ADM_toolkitGtk.h"
#include "ADM_vidCNR2_param.h"

static GtkWidget        *create_dialog1 (void);

#define CHECK_GET(x,y) {param->y=gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(WID(x)));}
#define CHECK_SET(x,y) {gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(x)),param->y);}     

#define ENTRY_SET(x) {gtk_write_entry(WID(entry##x),(int)param->x);}
#define ENTRY_GET(x) {param->x=gtk_read_entry(WID(entry##x));}


uint8_t DIA_cnr2(CNR2Param *param);
uint8_t DIA_cnr2(CNR2Param *param)
{
GtkWidget *dialog;
int ret=0;
        dialog=create_dialog1();
		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);

        // Update
        ENTRY_SET(lm);ENTRY_SET(ln);
        ENTRY_SET(um);ENTRY_SET(un);
        ENTRY_SET(vm);ENTRY_SET(vn);

        CHECK_SET(checkbuttonChroma,sceneChroma);

        if(param->mode & 0xFF0000) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(checkbuttonl)),1);
        if(param->mode & 0xFF00) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(checkbuttonu)),1);
        if(param->mode & 0xFF) gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(checkbuttonv)),1);
        
        gtk_range_set_value (GTK_RANGE(WID(hscale1)),(gdouble)param->scdthr);
        
        //**************************
        gtk_register_dialog(dialog);
        if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_OK)
        {
          GtkAdjustment *adj;   

                adj=gtk_range_get_adjustment (GTK_RANGE(WID(hscale1)));
                param->scdthr=(double)GTK_ADJUSTMENT(adj)->value;
        
                ENTRY_GET(lm);ENTRY_GET(ln);
                ENTRY_GET(um);ENTRY_GET(un);
                ENTRY_GET(vm);ENTRY_GET(vn);

                CHECK_GET(checkbuttonChroma,sceneChroma);
                param->mode=0;
#define UPD(x,y) if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(x)))) param->mode|=y;
                        UPD(checkbuttonl,0xFF0000);
                        UPD(checkbuttonu,0xFF00);
                        UPD(checkbuttonv,0xFF);
               

                ret=1;
        
        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        return ret;

}

//__________________________________

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *frame3;
  GtkWidget *alignment3;
  GtkWidget *hscale1;
  GtkWidget *label9;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *table1;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label2;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  GtkWidget *entryln;
  GtkWidget *entrylm;
  GtkWidget *entryun;
  GtkWidget *entryvn;
  GtkWidget *entryum;
  GtkWidget *entryvm;
  GtkWidget *label10;
  GtkWidget *checkbuttonl;
  GtkWidget *checkbuttonu;
  GtkWidget *checkbuttonv;
  GtkWidget *label1;
  GtkWidget *frame2;
  GtkWidget *alignment2;
  GtkWidget *vbox2;
  GtkWidget *checkbuttonChroma;
  GtkWidget *label8;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Chroma Noise Reduction 2"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame3), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);

  hscale1 = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 0)));
  gtk_widget_show (hscale1);
  gtk_container_add (GTK_CONTAINER (alignment3), hscale1);
  gtk_scale_set_digits (GTK_SCALE (hscale1), 2);

  label9 = gtk_label_new (QT_TR_NOOP("<b>Scene change Treshold</b>"));
  gtk_widget_show (label9);
  gtk_frame_set_label_widget (GTK_FRAME (frame3), label9);
  gtk_label_set_use_markup (GTK_LABEL (label9), TRUE);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 0, 12, 0);

  table1 = gtk_table_new (4, 4, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (alignment1), table1);

  label3 = gtk_label_new (QT_TR_NOOP("Sensibility"));
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_CENTER);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label4 = gtk_label_new (QT_TR_NOOP("Max"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_CENTER);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label2 = gtk_label_new ("");
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  label5 = gtk_label_new (QT_TR_NOOP("Luma :"));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  label6 = gtk_label_new (QT_TR_NOOP("Chroma U :"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  label7 = gtk_label_new (QT_TR_NOOP("Chroma V :"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table1), label7, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  entryln = gtk_entry_new ();
  gtk_widget_show (entryln);
  gtk_table_attach (GTK_TABLE (table1), entryln, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entrylm = gtk_entry_new ();
  gtk_widget_show (entrylm);
  gtk_table_attach (GTK_TABLE (table1), entrylm, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entryun = gtk_entry_new ();
  gtk_widget_show (entryun);
  gtk_table_attach (GTK_TABLE (table1), entryun, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entryvn = gtk_entry_new ();
  gtk_widget_show (entryvn);
  gtk_table_attach (GTK_TABLE (table1), entryvn, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entryum = gtk_entry_new ();
  gtk_widget_show (entryum);
  gtk_table_attach (GTK_TABLE (table1), entryum, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entryvm = gtk_entry_new ();
  gtk_widget_show (entryvm);
  gtk_table_attach (GTK_TABLE (table1), entryvm, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label10 = gtk_label_new (QT_TR_NOOP("Narrow"));
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (table1), label10, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  checkbuttonl = gtk_check_button_new_with_mnemonic ("");
  gtk_widget_show (checkbuttonl);
  gtk_table_attach (GTK_TABLE (table1), checkbuttonl, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbuttonu = gtk_check_button_new_with_mnemonic ("");
  gtk_widget_show (checkbuttonu);
  gtk_table_attach (GTK_TABLE (table1), checkbuttonu, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  checkbuttonv = gtk_check_button_new_with_mnemonic ("");
  gtk_widget_show (checkbuttonv);
  gtk_table_attach (GTK_TABLE (table1), checkbuttonv, 3, 4, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label1 = gtk_label_new (QT_TR_NOOP("<b>Settings</b>"));
  gtk_widget_show (label1);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label1);
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame2), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (alignment2), vbox2);

  checkbuttonChroma = gtk_check_button_new_with_mnemonic (QT_TR_NOOP("Use also chroma to detect scene change"));
  gtk_widget_show (checkbuttonChroma);
  gtk_box_pack_start (GTK_BOX (vbox2), checkbuttonChroma, FALSE, FALSE, 0);

  label8 = gtk_label_new (QT_TR_NOOP("<b>Mode</b>"));
  gtk_widget_show (label8);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label8);
  gtk_label_set_use_markup (GTK_LABEL (label8), TRUE);

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
  GLADE_HOOKUP_OBJECT (dialog1, frame3, "frame3");
  GLADE_HOOKUP_OBJECT (dialog1, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (dialog1, hscale1, "hscale1");
  GLADE_HOOKUP_OBJECT (dialog1, label9, "label9");
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dialog1, table1, "table1");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");
  GLADE_HOOKUP_OBJECT (dialog1, label4, "label4");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, entryln, "entryln");
  GLADE_HOOKUP_OBJECT (dialog1, entrylm, "entrylm");
  GLADE_HOOKUP_OBJECT (dialog1, entryun, "entryun");
  GLADE_HOOKUP_OBJECT (dialog1, entryvn, "entryvn");
  GLADE_HOOKUP_OBJECT (dialog1, entryum, "entryum");
  GLADE_HOOKUP_OBJECT (dialog1, entryvm, "entryvm");
  GLADE_HOOKUP_OBJECT (dialog1, label10, "label10");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonl, "checkbuttonl");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonu, "checkbuttonu");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonv, "checkbuttonv");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonChroma, "checkbuttonChroma");
  GLADE_HOOKUP_OBJECT (dialog1, label8, "label8");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


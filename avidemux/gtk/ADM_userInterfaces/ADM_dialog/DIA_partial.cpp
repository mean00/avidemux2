//
// C++ Implementation: DIA_decimate
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
// C++ Implementation: DIA_dectel
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "ADM_toolkitGtk.h"


#include "ADM_video/ADM_vidPartial_param.h"
#include "ADM_videoFilter.h"
#include "DIA_coreToolkit.h"
#define SPIN_GET(x,y) {param->y= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(WID(x))) ;}
#define SPIN_SET(x,y)  {gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(x)),(gfloat)param->y) ;}
#define SGET(x) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(WID(x))) 

static GtkWidget        *create_dialog1 (void);
#define F_CONF 0x100
uint8_t DIA_getPartial(PARTIAL_CONFIG *param,AVDMGenericVideoStream *son,AVDMGenericVideoStream *previous)
{
  
    GtkWidget *dialog;
    int ret=0,done=0;
    dialog=create_dialog1();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
								GTK_RESPONSE_OK,
								GTK_RESPONSE_CANCEL,
								-1);

    // Update
    SPIN_SET(spinbuttonStart,_start);
    SPIN_SET(spinbuttonSize,_end);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), WID(buttonConf), F_CONF);
    // Run
    gtk_register_dialog(dialog);
    while(!done)
    {
      switch(gtk_dialog_run(GTK_DIALOG(dialog)))
      {
          case F_CONF:
              printf("Poof\n");
              son->configure(previous);
              break;
          case GTK_RESPONSE_OK:
            {
              uint32_t s,e;
                s=SGET(spinbuttonStart);
                e=SGET(spinbuttonSize);
                if(e<s)
                {
                  GUI_Error_HIG("Bad parameters","The end frame must be after the start frame !"); 
                }
                else
                {
                  param->_start=s;
                  param->_end=e;
                  done=1;
                  ret=1;
                }
              }
                break;
            default:
              done=1;
              break;
                
        }
    }
    gtk_unregister_dialog(dialog);
    gtk_widget_destroy(dialog);
    return ret;
}
GtkWidget*
    create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *table1;
  GtkWidget *label1;
  GtkWidget *label2;
  GtkObject *spinbuttonStart_adj;
  GtkWidget *spinbuttonStart;
  GtkObject *spinbuttonSize_adj;
  GtkWidget *spinbuttonSize;
  GtkWidget *buttonConf;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Partial"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("Start frame for partial :"));
  gtk_widget_show (label1);
  gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  label2 = gtk_label_new (QT_TR_NOOP("End frame for partial :"));
  gtk_widget_show (label2);
  gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

  spinbuttonStart_adj = gtk_adjustment_new (1, 0, 1000000, 1, 10, 10);
  spinbuttonStart = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonStart_adj), 1, 0);
  gtk_widget_show (spinbuttonStart);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonStart, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonStart), TRUE);

  spinbuttonSize_adj = gtk_adjustment_new (1, 0, 1000000, 1, 10, 10);
  spinbuttonSize = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonSize_adj), 1, 0);
  gtk_widget_show (spinbuttonSize);
  gtk_table_attach (GTK_TABLE (table1), spinbuttonSize, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonSize), TRUE);

  buttonConf = gtk_button_new_with_mnemonic (QT_TR_NOOP("Configure partialized filter"));
  gtk_widget_show (buttonConf);
  gtk_box_pack_start (GTK_BOX (vbox1), buttonConf, FALSE, FALSE, 0);

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
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonStart, "spinbuttonStart");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonSize, "spinbuttonSize");
  GLADE_HOOKUP_OBJECT (dialog1, buttonConf, "buttonConf");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


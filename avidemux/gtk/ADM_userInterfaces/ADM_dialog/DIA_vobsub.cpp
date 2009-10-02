/***************************************************************************
                          DIA_vobsub.cpp  -  description
                             -------------------
    begin                : Thu Apr 21 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

  This class deals with the working window

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

#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"
#if 0
#define CHECK_GET(x,y) {*y=gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(WID(x)));}
#define CHECK_SET(x,y) {gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(WID(x)),*y);}   
#include "ADM_videoFilter.h"



#include "ADM_videoFilter/ADM_vobsubinfo.h"
#include "ADM_videoFilter/ADM_vidVobSub.h"

#define MAX_INDECES ADM_MAX_LANGUAGE

static GtkWidget        *create_dialog1 (void);
static GtkWidget        **fq;
static GtkWidget        *dialog;
static int              indeces[MAX_INDECES];
static void update(char *name,int i);

/* Dialog for update vobsub parameters :
 * - File name
 * - Shift
 * - Selected language
 */
uint8_t DIA_vobsub(vobSubParam *param);
uint8_t DIA_vobsub(vobSubParam *param)
{
  char *name;
  int32_t shift;     
  int ret,ext;
        
  ret = 0;
  ext = 0;
  name = param->subname;
  shift = param->subShift;
  
  while(!ext)
  {
    dialog = create_dialog1();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_OK,
										GTK_RESPONSE_CANCEL,
										-1);

    gtk_register_dialog(dialog);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog),WID(buttonSelect),GTK_RESPONSE_APPLY);
    
    fq = new GtkWidget*[ADM_MAX_LANGUAGE];
    
    // update if any
        if(name)
        {
                update(name,param->index);
      gtk_label_set_text(GTK_LABEL(WID(labelVobsub)),name);
        }
        else
        {
                gtk_label_set_text(GTK_LABEL(WID(labelVobsub)),QT_TR_NOOP("none"));     
        }

        gtk_write_entry(WID(entryShift),shift);

    switch(gtk_dialog_run(GTK_DIALOG(dialog)))
        {
              case GTK_RESPONSE_APPLY:
                char *file;
                        GUI_FileSelRead(QT_TR_NOOP("Select .idx file"),&file); 
                        if(file)
                        {
          ADM_dealloc(name);
          name = file;
              }
        shift = gtk_read_entry(WID(entryShift));
                break;
              case GTK_RESPONSE_OK:
        if(!name)
                {
          GUI_Error_HIG(QT_TR_NOOP("Wrong VobSub parametering"),QT_TR_NOOP("The idx/sub file does not set."));
        }
        else
        {
          param->subname = name;
          param->index = indeces[getRangeInMenu(WID(optionmenu1))];
          param->subShift = gtk_read_entry(WID(entryShift));
          ret = 1;
          ext = 1;
                }
                break;
              case GTK_RESPONSE_CANCEL:
      default:
        if(name != param->subname)
        {
          ADM_dealloc(name);
        }
                ret=0;
                ext=1;
                break;
        }
        delete [] fq;          
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
  }
     
  return ret;
}    

//*****************************************************   
void update(char *name,int idx)
{
  GtkWidget *menu1;
   // Print
  if(!name) return;
  menu1 = gtk_menu_new ();  
  gtk_label_set_text(GTK_LABEL(WID(labelVobsub)),name);     
    // Try to load it...
  vobSubLanguage *lang=NULL;
    
  lang=vobSubAllocateLanguage();
    
  if(vobSubGetLanguage(name,lang))
  {
      // add them
    for(int i=0;i<lang->nbLanguage;i++)
    {
      fq[i]=gtk_menu_item_new_with_mnemonic (lang->language[i].name);
      gtk_widget_show (fq[i]);
      gtk_container_add (GTK_CONTAINER (menu1), fq[i]);
      indeces[i]=lang->language[i].index;
      ADM_assert(i<MAX_INDECES);
    }
    gtk_option_menu_set_menu (GTK_OPTION_MENU (WID(optionmenu1)), menu1);
    gtk_option_menu_set_history(GTK_OPTION_MENU(WID(optionmenu1)), idx);
  }
    // Destroy
  vobSubDestroyLanguage(lang);
}
//*****************************************************

GtkWidget*
    create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *frame1;
  GtkWidget *table1;
  GtkWidget *buttonSelect;
  GtkWidget *labelVobsub;
  GtkWidget *label4;
  GtkWidget *optionmenu1;
  GtkWidget *label2;
  GtkWidget *frame2;
  GtkWidget *table2;
  GtkWidget *label6;
  GtkObject *spinbutton1_adj;
  GtkWidget *spinbutton1;
  GtkWidget *label7;
  GtkWidget *entryShift;
  GtkWidget *label5;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("VobSub Settings"));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, TRUE, TRUE, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (frame1), table1);

  buttonSelect = gtk_button_new_with_mnemonic (QT_TR_NOOP("Select .idx"));
  gtk_widget_show (buttonSelect);
  gtk_table_attach (GTK_TABLE (table1), buttonSelect, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  labelVobsub = gtk_label_new (QT_TR_NOOP("None"));
  gtk_widget_show (labelVobsub);
  gtk_table_attach (GTK_TABLE (table1), labelVobsub, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (labelVobsub), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (labelVobsub), 0, 0.5);

  label4 = gtk_label_new (QT_TR_NOOP("Select Language :"));
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  optionmenu1 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu1);
  gtk_table_attach (GTK_TABLE (table1), optionmenu1, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label2 = gtk_label_new (QT_TR_NOOP("Select Sub"));
  gtk_widget_show (label2);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label2);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);

  table2 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (frame2), table2);

  label6 = gtk_label_new (QT_TR_NOOP("Extra Shrink Factor :"));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table2), label6, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);

  spinbutton1_adj = gtk_adjustment_new (1, 1, 2, 0.1, 0.2, 0.2);
  spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
  gtk_widget_show (spinbutton1);
  gtk_table_attach (GTK_TABLE (table2), spinbutton1, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label7 = gtk_label_new (QT_TR_NOOP("Shift (ms) :"));
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table2), label7, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label7), 0, 0.5);

  entryShift = gtk_entry_new ();
  gtk_widget_show (entryShift);
  gtk_table_attach (GTK_TABLE (table2), entryShift, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label5 = gtk_label_new (QT_TR_NOOP("Extra Settings"));
  gtk_widget_show (label5);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label5);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);

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
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, table1, "table1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonSelect, "buttonSelect");
  GLADE_HOOKUP_OBJECT (dialog1, labelVobsub, "labelVobsub");
  GLADE_HOOKUP_OBJECT (dialog1, label4, "label4");
  GLADE_HOOKUP_OBJECT (dialog1, optionmenu1, "optionmenu1");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, table2, "table2");
  GLADE_HOOKUP_OBJECT (dialog1, label6, "label6");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton1, "spinbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, entryShift, "entryShift");
  GLADE_HOOKUP_OBJECT (dialog1, label5, "label5");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}
#endif

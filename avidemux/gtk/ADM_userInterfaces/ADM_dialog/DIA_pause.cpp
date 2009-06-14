//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ADM_toolkitGtk.h"


static GtkWidget	*create_dialog1 (void);
//	Return 1 if resume, 0 if abort
//
uint8_t DIA_Paused( void )
{
	uint8_t ret=2;
	GtkWidget *dialog;
	
	while(ret==2)
	{
		dialog=create_dialog1();

		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_CANCEL,
										GTK_RESPONSE_OK,
										-1);
		//  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), WID(button1), GTK_RESPONSE_CANCEL);
		//  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), WID(button2), GTK_RESPONSE_OK);
		//gtk_transient(dialog);
		gtk_register_dialog(dialog);
		int i=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(i)
		{
			case GTK_RESPONSE_OK:
				ret=0;	// abort
				break;
			case GTK_RESPONSE_CANCEL:
				ret=1;	// resume
				break;
			default:
				ret=2;	// continnue;
				break;
		}
		gtk_unregister_dialog(dialog);
		gtk_widget_destroy(dialog);
	};
	return ret;
}


GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *button2;
  GtkWidget *alignment1;
  GtkWidget *hbox2;
  GtkWidget *image2;
  GtkWidget *label2;
  GtkWidget *button1;
  GtkWidget *alignment2;
  GtkWidget *hbox3;
  GtkWidget *image3;
  GtkWidget *label3;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Paused"));
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 12);

  image1 = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("<b>Encoding is paused.</b>\n\nYou can either resume or Abort it."));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, FALSE, 0);
  gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  button2 = gtk_button_new ();
  gtk_widget_show (button2);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button2, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (button2), alignment1);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox2);

  image2 = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox2), image2, FALSE, FALSE, 0);

  label2 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Abort"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox2), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  button1 = gtk_button_new ();
  gtk_widget_show (button1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), button1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (button1), alignment2);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox3);

  image3 = gtk_image_new_from_stock ("gtk-redo", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image3);
  gtk_box_pack_start (GTK_BOX (hbox3), image3, FALSE, FALSE, 0);

  label3 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Resume"));
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox3), label3, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, button2, "button2");
  GLADE_HOOKUP_OBJECT (dialog1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (dialog1, image2, "image2");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, button1, "button1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (dialog1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (dialog1, image3, "image3");
  GLADE_HOOKUP_OBJECT (dialog1, label3, "label3");

  gtk_widget_grab_focus (button1);
  gtk_widget_grab_default (button1);
  return dialog1;
}


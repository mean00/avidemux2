#include "ADM_toolkitGtk.h"
#include "prefs.h"

static GtkWidget	*create_dialog1 (void);

uint8_t DIA_pipe(char **cmd,char **param)
{
	GtkWidget *dialog;

	char *str;
	uint8_t ret=0;

	gint r;

#define FILL_ENTRY(widget_name,value) {		 \
gtk_editable_delete_text(GTK_EDITABLE(lookup_widget(dialog,#widget_name)), 0,-1);\
gtk_editable_insert_text(GTK_EDITABLE(lookup_widget(dialog,#widget_name)), str, strlen(str), &r);}

	dialog=create_dialog1();
	gtk_transient(dialog);
	*cmd=NULL;
	*param=NULL;

	if(!prefs->get(PIPE_CMD,&str)) str=NULL;
	if(str)
	{
		FILL_ENTRY(entryName,left);
	}
	if(!prefs->get(PIPE_PARAM,&str)) str=NULL;
	if(str)
	{
		FILL_ENTRY(entryParam,left);
	}

	ret=0;
	if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_OK)
	{

		// gchar *s;
		// uint32_t val1;
#define READ_ENTRY(widget_name)   gtk_editable_get_chars(GTK_EDITABLE (lookup_widget(dialog,#widget_name)), 0, -1);

			str=READ_ENTRY(entryName);
			if(strlen(str))
			{
				*cmd=ADM_strdup(str);;
				prefs->set(PIPE_CMD,str);
			}
			str=READ_ENTRY(entryName);
			if(strlen(str))
			{
				*param=ADM_strdup(str);;
				prefs->set(PIPE_PARAM,str);
			}
			ret=1;
	}

	gtk_widget_destroy(dialog);

	return ret;

}
GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *entryName;
  GtkWidget *label2;
  GtkWidget *entryParam;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Pipe to"));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("Application path/name (/usr/bin/sox...)"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  entryName = gtk_entry_new ();
  gtk_widget_show (entryName);
  gtk_box_pack_start (GTK_BOX (vbox1), entryName, FALSE, FALSE, 0);

  label2 = gtk_label_new (QT_TR_NOOP("Full parameter line"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox1), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  entryParam = gtk_entry_new ();
  gtk_widget_show (entryParam);
  gtk_box_pack_start (GTK_BOX (vbox1), entryParam, FALSE, FALSE, 0);

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
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT (dialog1, entryName, "entryName");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT (dialog1, entryParam, "entryParam");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


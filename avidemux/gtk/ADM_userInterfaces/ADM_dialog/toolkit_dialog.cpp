/***************************************************************************
                          toolkit_dialog.cpp  -  description
                             -------------------

  Handle simple dialog (alert, yes./no)


    begin                : Fri Dec 14 2001
    copyright            : (C) 2001 by mean
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
#include "prefs.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

static GtkWidget	*create_dialogYN (void);
static GtkWidget	*create_dialogConfirmation (const char *confirm_text);
static GtkWidget	*create_dialogWarning (void);
static GtkWidget       *create_dialogInfo (void);
static int beQuiet=0;


/**
	GUI_Quiet : Prevents gui from poping alert
	Answers always no to question

*/
namespace ADM_GtkCoreUIToolkit
{
uint8_t GUI_isQuiet(void )
{
	return beQuiet;
}
void GUI_Quiet( void )
{
	beQuiet=1;

}
/**
	GUI_Verbose : Allow gui to ask question & popup alert

*/
void GUI_Verbose( void )
{
	beQuiet=0;

}

/**
	GUI_Question
		Ask the question passed in alertstring
			Return 1 if yes
			Return 0 if no

		In silent mode, always return 0

*/
int 		GUI_Question(const char *alertstring)
{
int ret=0;

        GtkWidget *dialog;

        if(beQuiet) 
        {

                printf("?? %s ?? -> NO\n",alertstring);
                return 0 ;
        }

        dialog=create_dialogYN();
        gtk_label_set_text(GTK_LABEL(WID(label1)),alertstring);
        gtk_register_dialog(dialog);
        if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_YES)
        {
                ret=1;
        }

        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        UI_purge();

	return ret;
}

/*
GUI_Info: display an info dialog.
Deprecated - use GUI_Info_HIG instead.
*/

void             GUI_Info(const char *alertstring)
{
        GtkWidget *dialog;

        if(beQuiet) 
        {
                printf("Info: %s\n",alertstring);
                return  ;
        }
        dialog=create_dialogInfo();
        gtk_label_set_text(GTK_LABEL(WID(label1)),alertstring);
        gtk_register_dialog(dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);
        UI_purge();

}

/**
GUI_Info_HIG: display an info dialog.
Takes primary and optional secondary string, as described in GNOME HIG 2.0.

@primary: primary string
@secondary_format: printf()-style format string for secondary text, or NULL for no secondary text
@...: arguments for secondary_format
*/
void GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format)
{
	GtkWidget *dialog;
	uint32_t msglvl=2;

        prefs->get(MESSAGE_LEVEL,&msglvl);

        if(msglvl<level)
        {
                printf("Info : %s \n",primary);
                return;
        }


	char *alertstring;
	
	if (secondary_format)
	{

		if (beQuiet)
		{
			printf("Info: %s\n%s\n", primary, secondary_format);
			return;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>\n\n%s", primary, secondary_format);
	}
	else
	{	
		if (beQuiet)
		{
			printf("Info: %s\n", primary);
			return;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>", primary);
	}
	
	
	dialog=create_dialogInfo();
	gtk_label_set_text(GTK_LABEL(WID(label1)), alertstring);
	g_free(alertstring);
	gtk_label_set_use_markup(GTK_LABEL(WID(label1)), TRUE);
	gtk_register_dialog(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	UI_purge();
}


/**
GUI_Error_HIG: display an error dialog.
Takes primary and optional secondary string, as described in GNOME HIG 2.0.

@primary: primary string
@secondary_format: printf()-style format string for secondary text, or NULL for no secondary text
@...: arguments for secondary_format
*/
void GUI_Error_HIG(const char *primary, const char *secondary_format)
{
	GtkWidget *dialog;
	uint32_t msglvl=2;

        prefs->get(MESSAGE_LEVEL,&msglvl);
        if(msglvl==ADM_LOG_NONE) 
        {
                printf("Error :%s\n",primary);
                return;
        }


	char *alertstring;
	
	if (secondary_format)
	{
		
		if (beQuiet)
		{
			printf("Info: %s\n%s\n", primary, secondary_format);
			return;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>\n\n%s", primary, secondary_format);
	}
	else
	{	
		if (beQuiet)
		{
			printf("Info: %s\n", primary);
			return;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>", primary);
	}
	
	
	dialog=create_dialogWarning();
	gtk_label_set_text(GTK_LABEL(WID(label1)), alertstring);
	g_free(alertstring);
	gtk_label_set_use_markup(GTK_LABEL(WID(label1)), TRUE);
	gtk_register_dialog(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	UI_purge();
}

/**
GUI_Confirmation_HIG: display a confirmation dialog with Cancel and custom confirmation button.
See GNOME HIG 2.0, chapter 3, section "Alerts" for more details.

Returns 1 if the answer is yes, 0 if the answer is no.
In silent mode, always return 0.

Takes primary and optional secondary string.

@button_confirm: confirmation button text
@primary: primary string
@secondary_format: printf()-style format string for secondary text, or NULL for no secondary text
@...: arguments for secondary_format
*/
int GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format)
{
	int ret=0;
	GtkWidget *dialog;
	
	char *alertstring;
	
	if (secondary_format)
	{
		
		if (beQuiet)
		{
			printf("Info: %s\n%s\n", primary, secondary_format);
			return 0;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>\n\n%s", primary, secondary_format);
	}
	else
	{	
		if (beQuiet)
		{
			printf("Info: %s\n", primary);
			return 0;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>", primary);
	}
	

	dialog=create_dialogConfirmation(button_confirm);
	gtk_label_set_text(GTK_LABEL(WID(label1)), alertstring);
	g_free(alertstring);
	gtk_label_set_use_markup(GTK_LABEL(WID(label1)), TRUE);
	gtk_register_dialog(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_YES)
	{
		ret=1;
	}
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	UI_purge();
	return ret;
}

/**
GUI_YesNo: display a question dialog with Yes/No buttons.
Returns 1 if the answer is yes, 0 if the answer is no.
In silent mode, always return 0.

Takes primary and optional secondary string.

Note: Yes/No alerts are not recommended - if possible, use GUI_Confirmation_HIG.
See GNOME HIG 2.0, chapter 3, section "Alerts" for more details.

@primary: primary string
@secondary_format: printf()-style format string for secondary text, or NULL for no secondary text
@...: arguments for secondary_format
*/
int GUI_YesNo(const char *primary, const char *secondary_format)
{
	int ret=0;
	GtkWidget *dialog;
	
	char *alertstring;
	
	if (secondary_format)
	{
		
		if (beQuiet)
		{
			printf("Info: %s\n%s\n", primary, secondary_format);
			return 0;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>\n\n%s", primary, secondary_format);
	}
	else
	{	
		if (beQuiet)
		{
			printf("Info: %s\n", primary);
			return 0;
		}
		alertstring = g_markup_printf_escaped("<span size=\"larger\" weight=\"bold\">%s</span>", primary);
	}
	

	dialog=create_dialogYN();
	gtk_label_set_text(GTK_LABEL(WID(label1)), alertstring);
	g_free(alertstring);
	gtk_label_set_use_markup(GTK_LABEL(WID(label1)), TRUE);
	gtk_register_dialog(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_YES)
	{
		ret=1;
	}
	gtk_unregister_dialog(dialog);
	gtk_widget_destroy(dialog);
	UI_purge();
	return ret;
}
extern int GUI_Alternate(const char *title,const char *choice1,const char *choice2);
extern DIA_workingBase *createWorking(const char *title);
extern DIA_encodingBase *createEncoding(uint64_t duration);

void getVersion(uint32_t *maj,uint32_t *minor)
{
    *maj=ADM_CORE_TOOLKIT_MAJOR;
    *minor=ADM_CORE_TOOLKIT_MINOR;
}
/**
    \fn UI_purge
    \brief make sure Gtk process events & refresh the windows
*/
void UI_purge( void )
{        
        while (gtk_events_pending())
                                {
                                                  gtk_main_iteration();
                              }
	
}
} // End of namespace

static CoreToolkitDescriptor GtkCoreToolkitDescriptor=
{
        &ADM_GtkCoreUIToolkit::getVersion,
		&ADM_GtkCoreUIToolkit::GUI_Info_HIG,
		&ADM_GtkCoreUIToolkit::GUI_Error_HIG,
		&ADM_GtkCoreUIToolkit::GUI_Confirmation_HIG,
		&ADM_GtkCoreUIToolkit::GUI_YesNo,
		&ADM_GtkCoreUIToolkit::GUI_Question,
		&ADM_GtkCoreUIToolkit::GUI_Alternate,
		&ADM_GtkCoreUIToolkit::GUI_Verbose,
		&ADM_GtkCoreUIToolkit::GUI_Quiet,
		&ADM_GtkCoreUIToolkit::GUI_isQuiet,
        &ADM_GtkCoreUIToolkit::createWorking,
        &ADM_GtkCoreUIToolkit::createEncoding,
        &ADM_GtkCoreUIToolkit::UI_purge
};

void InitCoreToolkit(void )
{
	DIA_toolkitInit(&GtkCoreToolkitDescriptor);
	
}
/**
	Return the line number of a selection
	0 if no selection of fails

*/
uint8_t getSelectionNumber(uint32_t nb,GtkWidget *tree  , GtkListStore 	*store,uint32_t *number)
{
		GtkTreeSelection *selection;
		GtkTreeIter ref; //iter,ref;
		int count=-1;

		*number=0;
		selection= gtk_tree_view_get_selection    (GTK_TREE_VIEW(tree));

		if(TRUE!=    gtk_tree_model_get_iter_first   (GTK_TREE_MODEL(store),&ref))
		{
			return 0;
		}

		for(uint32_t l=0;l<nb;l++)
		{
			if(gtk_tree_selection_iter_is_selected  (selection,&ref)) count=l;
			gtk_tree_model_iter_next  (GTK_TREE_MODEL(store),&ref);
		}
//		printf(" Found sel :%d\n",count);
		if(count==-1)
		{
			return 0;

		}
		else
		{
			*number=count;;
			return 1;
		}
}
/**

	Select the row number number in the list given as arg

*/
uint8_t setSelectionNumber(uint32_t nb,GtkWidget *tree  , GtkListStore 	*store,uint32_t number)
{
		GtkTreeSelection *selection;
		GtkTreeIter ref; //iter,ref;

	 	selection= gtk_tree_view_get_selection    (GTK_TREE_VIEW(tree));
		/*
		gtk_tree_selection_select_all (selection);
		return 1;
		*/
		if(TRUE!=    gtk_tree_model_get_iter_first   (GTK_TREE_MODEL(store),&ref))
		{
			printf("Cannot get first iter...\n");
			return 0;
		}

		for(uint32_t l=0;l<nb;l++)
		{
			if(l==number)
			{
				gtk_tree_selection_select_iter (selection,&ref);
				return 1;
			}
			gtk_tree_model_iter_next  (GTK_TREE_MODEL(store),&ref);
		}
		printf(" Could not set selection %d!!\n",number);
		return 0;
}





GtkWidget*
create_dialogYN (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *buttonYes;
  GtkWidget *buttonNo;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), "");
  
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);  

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_box_set_spacing (GTK_BOX (dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_set_spacing (GTK_BOX (hbox1), 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_stock ("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0.0);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("label1"));
  gtk_label_set_line_wrap (GTK_LABEL(label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.0);
  gtk_label_set_selectable (GTK_LABEL(label1), TRUE);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_CENTER);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  buttonYes = gtk_button_new_from_stock ("gtk-no");
  gtk_widget_show (buttonYes);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonYes, GTK_RESPONSE_NO);
  GTK_WIDGET_SET_FLAGS (buttonYes, GTK_CAN_DEFAULT);

  buttonNo = gtk_button_new_from_stock ("gtk-yes");
  gtk_widget_show (buttonNo);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonNo, GTK_RESPONSE_YES);
  GTK_WIDGET_SET_FLAGS (buttonNo, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonYes, "buttonYes");
  GLADE_HOOKUP_OBJECT (dialog1, buttonNo, "buttonNo");

  gtk_widget_grab_default (buttonYes);

  gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog1),
										GTK_RESPONSE_YES,
										GTK_RESPONSE_NO,
										-1);

  return dialog1;
}


GtkWidget*
create_dialogConfirmation (const char *confirm_text)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *buttonCancel;
  GtkWidget *buttonYes;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), "");
  
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);  

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_box_set_spacing (GTK_BOX (dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_set_spacing (GTK_BOX (hbox1), 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0.0);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("label1"));
  gtk_label_set_line_wrap (GTK_LABEL(label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.0);
  gtk_label_set_selectable (GTK_LABEL(label1), TRUE);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);
  //gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_CENTER);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  buttonCancel = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (buttonCancel);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonCancel, GTK_RESPONSE_NO);
  GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

  buttonYes = gtk_button_new_from_stock (confirm_text);
  gtk_widget_show (buttonYes);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonYes, GTK_RESPONSE_YES);
  GTK_WIDGET_SET_FLAGS (buttonYes, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonCancel, "buttonCancel");
  GLADE_HOOKUP_OBJECT (dialog1, buttonYes, "buttonYes");

  gtk_widget_grab_default (buttonYes);

  gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog1),
										GTK_RESPONSE_YES,
										GTK_RESPONSE_NO,
										-1);

  return dialog1;
}


GtkWidget*
create_dialogOK (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Alert"));

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_stock ("gtk-dialog-warning", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("label1"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


GtkWidget       *create_dialogInfo (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *closebutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), "");
  
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_box_set_spacing (GTK_BOX (dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_set_spacing (GTK_BOX (hbox1), 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0.0);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("label1"));
  gtk_label_set_line_wrap (GTK_LABEL(label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.0);
  gtk_label_set_selectable (GTK_LABEL(label1), TRUE);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  closebutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (closebutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), closebutton1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (closebutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, closebutton1, "closebutton1");

  gtk_widget_grab_default (closebutton1);
  return dialog1;
}

GtkWidget*
create_dialogWarning (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *closebutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), "");
  
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_box_set_spacing (GTK_BOX (dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_set_spacing (GTK_BOX (hbox1), 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
  gtk_misc_set_alignment (GTK_MISC (image1), 0.5, 0.0);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label1 = gtk_label_new (QT_TR_NOOP("label1"));
  gtk_label_set_line_wrap (GTK_LABEL(label1), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label1), 0.5, 0.0);
  gtk_label_set_selectable (GTK_LABEL(label1), TRUE);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, TRUE, TRUE, 0);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  closebutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (closebutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), closebutton1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (closebutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, closebutton1, "closebutton1");

  gtk_widget_grab_default (closebutton1);
  return dialog1;
}

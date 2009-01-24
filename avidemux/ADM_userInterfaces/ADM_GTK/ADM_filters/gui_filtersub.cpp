
/*
	gui_filterlist
	Build the dialog box that list the active filters


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"
#include "ADM_toolkitGtk.h"
#include "ADM_default.h"

GtkWidget		*create_filterMain (void);
GtkListStore 	*storeMainFilter;
GtkWidget       *create_filterMain (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *hbox1;
  GtkWidget *scrolledwindow2;
  GtkWidget *treeview2;
  GtkWidget *toolbar1;
  GtkIconSize tmp_toolbar_icon_size;
  GtkWidget *toolbuttonAdd;
  GtkWidget *toolbuttonRemove;
  GtkWidget *toolbuttonProperties;
  GtkWidget *toolbuttonUp;
  GtkWidget *toolbuttonDown;
  GtkWidget *tmp_image;
  GtkWidget *toolbuttonVCD;
  GtkWidget *toolbuttonSVCD;
  GtkWidget *toolbuttonDVD;
  GtkWidget *toolbuttonHD1;
  GtkWidget *toolbuttonOpen;
  GtkWidget *toolbuttonSave;
  GtkWidget *toolbuttonPartial;
  GtkWidget *toolbuttonPreview;
  GtkWidget *toolbutton13;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Video Filters"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (viewport1), hbox1);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow2, TRUE, TRUE, 0);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);

  treeview2 = gtk_tree_view_new ();
  gtk_widget_show (treeview2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), treeview2);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview2), FALSE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview2), FALSE);

  toolbar1 = gtk_toolbar_new ();
  gtk_widget_show (toolbar1);
  gtk_box_pack_start (GTK_BOX (hbox1), toolbar1, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH_HORIZ);
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar1), GTK_ORIENTATION_VERTICAL);
  tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));

  toolbuttonAdd = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-add");
  gtk_widget_show (toolbuttonAdd);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonAdd);

  toolbuttonRemove = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-remove");
  gtk_widget_show (toolbuttonRemove);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonRemove);

  toolbuttonProperties = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-properties");
  gtk_widget_show (toolbuttonProperties);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonProperties);

  toolbuttonUp = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-go-up");
  gtk_widget_show (toolbuttonUp);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonUp);

  toolbuttonDown = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-go-down");
  gtk_widget_show (toolbuttonDown);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonDown);

  tmp_image = gtk_image_new_from_stock ("gtk-zoom-fit", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonVCD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("VCD res"));
  gtk_widget_show (toolbuttonVCD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonVCD);

  tmp_image = gtk_image_new_from_stock ("gtk-zoom-fit", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonSVCD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("SVCD res"));
  gtk_widget_show (toolbuttonSVCD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonSVCD);

  tmp_image = gtk_image_new_from_stock ("gtk-zoom-fit", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonDVD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("DVD res"));
  gtk_widget_show (toolbuttonDVD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonDVD);

  tmp_image = gtk_image_new_from_stock ("gtk-zoom-fit", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonHD1 = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Half D1 res"));
  gtk_widget_show (toolbuttonHD1);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonHD1);

  toolbuttonOpen = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-open");
  gtk_widget_show (toolbuttonOpen);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonOpen);

  toolbuttonSave = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-save");
  gtk_widget_show (toolbuttonSave);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonSave);

  tmp_image = gtk_image_new_from_stock ("gtk-dnd-multiple", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonPartial = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Partial"));
  gtk_widget_show (toolbuttonPartial);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonPartial);

  tmp_image = gtk_image_new_from_stock ("gtk-print-preview", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonPreview = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Preview"));
  gtk_widget_show (toolbuttonPreview);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonPreview);

  tmp_image = gtk_image_new_from_stock ("gtk-save-as", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbutton13 = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Save as script"));
  gtk_widget_show (toolbutton13);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbutton13);

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
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (dialog1, viewport1, "viewport1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (dialog1, treeview2, "treeview2");
  GLADE_HOOKUP_OBJECT (dialog1, toolbar1, "toolbar1");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonAdd, "toolbuttonAdd");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonRemove, "toolbuttonRemove");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonProperties, "toolbuttonProperties");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonUp, "toolbuttonUp");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonDown, "toolbuttonDown");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonVCD, "toolbuttonVCD");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonSVCD, "toolbuttonSVCD");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonDVD, "toolbuttonDVD");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonHD1, "toolbuttonHD1");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonOpen, "toolbuttonOpen");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonSave, "toolbuttonSave");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonPartial, "toolbuttonPartial");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonPreview, "toolbuttonPreview");
  GLADE_HOOKUP_OBJECT (dialog1, toolbutton13, "toolbutton13");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


/*
	gui_filterlist
	Build the dialog box that list all available filters


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

GtkListStore 	*storeFilterList;
GtkWidget	*create_dialogList (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *notebook1;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview11;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label17;
  GtkWidget *scrolledwindow2;
  GtkWidget *treeview12;
  GtkWidget *hbox2;
  GtkWidget *image2;
  GtkWidget *label18;
  GtkWidget *scrolledwindow3;
  GtkWidget *treeview13;
  GtkWidget *hbox3;
  GtkWidget *image3;
  GtkWidget *label19;
  GtkWidget *scrolledwindow4;
  GtkWidget *treeview14;
  GtkWidget *hbox4;
  GtkWidget *image4;
  GtkWidget *label20;
  GtkWidget *scrolledwindow5;
  GtkWidget *treeview15;
  GtkWidget *hbox5;
  GtkWidget *image5;
  GtkWidget *label21;
  GtkWidget *scrolledwindow6;
  GtkWidget *treeview16;
  GtkWidget *hbox6;
  GtkWidget *image6;
  GtkWidget *label22;
  GtkWidget *scrolledwindow7;
  GtkWidget *treeview17;
  GtkWidget *hbox7;
  GtkWidget *image7;
  GtkWidget *label23;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Add Video Filter"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook1), 6);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook1), GTK_POS_LEFT);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow1), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview11 = gtk_tree_view_new ();
  gtk_widget_show (treeview11);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview11);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview11), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview11), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview11), FALSE);

  hbox1 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), hbox1);

  image1 = create_pixmap (dialog1, "1.png");
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, FALSE, 0);

  label17 = gtk_label_new (QT_TR_NOOP("Transformation"));
  gtk_widget_show (label17);
  gtk_box_pack_start (GTK_BOX (hbox1), label17, FALSE, FALSE, 0);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow2), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview12 = gtk_tree_view_new ();
  gtk_widget_show (treeview12);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), treeview12);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview12), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview12), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview12), FALSE);

  hbox2 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), hbox2);

  image2 = create_pixmap (dialog1, "2.png");
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox2), image2, FALSE, FALSE, 0);

  label18 = gtk_label_new (QT_TR_NOOP("Interlacing"));
  gtk_widget_show (label18);
  gtk_box_pack_start (GTK_BOX (hbox2), label18, FALSE, FALSE, 0);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow3), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview13 = gtk_tree_view_new ();
  gtk_widget_show (treeview13);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), treeview13);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview13), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview13), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview13), FALSE);

  hbox3 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox3);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), hbox3);

  image3 = create_pixmap (dialog1, "4.png");
  gtk_widget_show (image3);
  gtk_box_pack_start (GTK_BOX (hbox3), image3, FALSE, FALSE, 0);

  label19 = gtk_label_new (QT_TR_NOOP("Colors"));
  gtk_widget_show (label19);
  gtk_box_pack_start (GTK_BOX (hbox3), label19, FALSE, FALSE, 0);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow4), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow4);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview14 = gtk_tree_view_new ();
  gtk_widget_show (treeview14);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), treeview14);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview14), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview14), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview14), FALSE);

  hbox4 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), hbox4);

  image4 = create_pixmap (dialog1, "5.png");
  gtk_widget_show (image4);
  gtk_box_pack_start (GTK_BOX (hbox4), image4, FALSE, FALSE, 0);

  label20 = gtk_label_new (QT_TR_NOOP("Denoise"));
  gtk_widget_show (label20);
  gtk_box_pack_start (GTK_BOX (hbox4), label20, FALSE, FALSE, 0);

  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow5), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow5);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview15 = gtk_tree_view_new ();
  gtk_widget_show (treeview15);
  gtk_container_add (GTK_CONTAINER (scrolledwindow5), treeview15);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview15), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview15), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview15), FALSE);

  hbox5 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), hbox5);

  image5 = create_pixmap (dialog1, "3.png");
  gtk_widget_show (image5);
  gtk_box_pack_start (GTK_BOX (hbox5), image5, FALSE, FALSE, 0);

  label21 = gtk_label_new (QT_TR_NOOP("Sharpen/Blur"));
  gtk_widget_show (label21);
  gtk_box_pack_start (GTK_BOX (hbox5), label21, FALSE, FALSE, 0);

  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow6), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow6);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview16 = gtk_tree_view_new ();
  gtk_widget_show (treeview16);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), treeview16);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview16), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview16), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview16), FALSE);

  hbox6 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), hbox6);

  image6 = create_pixmap (dialog1, "7.png");
  gtk_widget_show (image6);
  gtk_box_pack_start (GTK_BOX (hbox6), image6, FALSE, FALSE, 0);

  label22 = gtk_label_new (QT_TR_NOOP("Subtitles"));
  gtk_widget_show (label22);
  gtk_box_pack_start (GTK_BOX (hbox6), label22, FALSE, FALSE, 0);

  scrolledwindow7 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow7), GTK_SHADOW_IN);
  gtk_widget_show (scrolledwindow7);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow7);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow7), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview17 = gtk_tree_view_new ();
  gtk_widget_show (treeview17);
  gtk_container_add (GTK_CONTAINER (scrolledwindow7), treeview17);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview17), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview17), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview17), FALSE);

  hbox7 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox7);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 6), hbox7);

  image7 = create_pixmap (dialog1, "6.png");
  gtk_widget_show (image7);
  gtk_box_pack_start (GTK_BOX (hbox7), image7, FALSE, FALSE, 0);

  label23 = gtk_label_new (QT_TR_NOOP("Miscellaneous"));
  gtk_widget_show (label23);
  gtk_box_pack_start (GTK_BOX (hbox7), label23, FALSE, FALSE, 0);

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
  GLADE_HOOKUP_OBJECT (dialog1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (dialog1, treeview11, "treeview11");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label17, "label17");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (dialog1, treeview12, "treeview12");
  GLADE_HOOKUP_OBJECT (dialog1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (dialog1, image2, "image2");
  GLADE_HOOKUP_OBJECT (dialog1, label18, "label18");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow3, "scrolledwindow3");
  GLADE_HOOKUP_OBJECT (dialog1, treeview13, "treeview13");
  GLADE_HOOKUP_OBJECT (dialog1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (dialog1, image3, "image3");
  GLADE_HOOKUP_OBJECT (dialog1, label19, "label19");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow4, "scrolledwindow4");
  GLADE_HOOKUP_OBJECT (dialog1, treeview14, "treeview14");
  GLADE_HOOKUP_OBJECT (dialog1, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (dialog1, image4, "image4");
  GLADE_HOOKUP_OBJECT (dialog1, label20, "label20");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow5, "scrolledwindow5");
  GLADE_HOOKUP_OBJECT (dialog1, treeview15, "treeview15");
  GLADE_HOOKUP_OBJECT (dialog1, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (dialog1, image5, "image5");
  GLADE_HOOKUP_OBJECT (dialog1, label21, "label21");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow6, "scrolledwindow6");
  GLADE_HOOKUP_OBJECT (dialog1, treeview16, "treeview16");
  GLADE_HOOKUP_OBJECT (dialog1, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (dialog1, image6, "image6");
  GLADE_HOOKUP_OBJECT (dialog1, label22, "label22");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow7, "scrolledwindow7");
  GLADE_HOOKUP_OBJECT (dialog1, treeview17, "treeview17");
  GLADE_HOOKUP_OBJECT (dialog1, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (dialog1, image7, "image7");
  GLADE_HOOKUP_OBJECT (dialog1, label23, "label23");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}


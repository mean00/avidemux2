/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_toolkitGtk.h"
#include "ADM_default.h"



#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *vbox3b;
  GtkWidget *toolbar1;
  GtkIconSize tmp_toolbar_icon_size;
  GtkWidget *toolbuttonOpen;
  GtkWidget *toolbuttonSave;
  GtkWidget *tmp_image;
  GtkWidget *toolbuttonScript;
  GtkWidget *toolbuttonDVD;
  GtkWidget *toolbuttonHalfD1;
  GtkWidget *toolbuttonSVCD;
  GtkWidget *toolbuttonVCD;
  GtkWidget *hbox11;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *vbox2;
  GtkWidget *notebook1;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview1;
  GtkWidget *hbox5;
  GtkWidget *image1;
  GtkWidget *label11;
  GtkWidget *scrolledwindow2;
  GtkWidget *treeview2;
  GtkWidget *hbox6;
  GtkWidget *image2;
  GtkWidget *label17;
  GtkWidget *scrolledwindow3;
  GtkWidget *treeview3;
  GtkWidget *hbox7;
  GtkWidget *image3;
  GtkWidget *label18;
  GtkWidget *scrolledwindow4;
  GtkWidget *treeview4;
  GtkWidget *hbox8;
  GtkWidget *image4;
  GtkWidget *label19;
  GtkWidget *scrolledwindow5;
  GtkWidget *treeview5;
  GtkWidget *hbox9;
  GtkWidget *image5;
  GtkWidget *label20;
  GtkWidget *scrolledwindow6;
  GtkWidget *treeview6;
  GtkWidget *hbox10;
  GtkWidget *image6;
  GtkWidget *label21;
  GtkWidget *scrolledwindow7;
  GtkWidget *treeview7;
  GtkWidget *hbox4;
  GtkWidget *image7;
  GtkWidget *label22;
  GtkWidget *treeview8;
  GtkWidget *label28;
  GtkWidget *hbox13;
  GtkWidget *buttonAdd;
  GtkWidget *image11;
  GtkWidget *label23;
  GtkWidget *frame2;
  GtkWidget *alignment2;
  GtkWidget *hbox1;
  GtkWidget *vbox3;
  GtkWidget *scrolledwindow9;
  GtkWidget *treeview0;
  GtkWidget *hbox14;
  GtkWidget *buttonRemove;
  GtkWidget *image15;
  GtkWidget *buttonDown;
  GtkWidget *image14;
  GtkWidget *buttonUp;
  GtkWidget *image13;
  GtkWidget *buttonPartial;
  GtkWidget *buttonProperties;
  GtkWidget *alignment4;
  GtkWidget *hbox16;
  GtkWidget *label25;
  GtkWidget *label2;
  GtkWidget *dialog_action_area1;
  GtkWidget *buttonPreview;
  GtkWidget *alignment5;
  GtkWidget *hbox17;
  GtkWidget *image17;
  GtkWidget *label26;
  GtkWidget *buttonClose;
  GtkAccelGroup *accel_group;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  accel_group = gtk_accel_group_new ();

  dialog1 = gtk_dialog_new ();
  gtk_container_set_border_width (GTK_CONTAINER (dialog1), 6);
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Video Filter Manager"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog1), FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_box_set_spacing (GTK_BOX(dialog_vbox1), 12);
  gtk_widget_show (dialog_vbox1);

  vbox3b = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (vbox3b);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), vbox3b, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3b), 6);

  toolbar1 = gtk_toolbar_new ();
  gtk_widget_show (toolbar1);
  gtk_box_pack_start (GTK_BOX (vbox3b), toolbar1, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH);
  tmp_toolbar_icon_size = gtk_toolbar_get_icon_size (GTK_TOOLBAR (toolbar1));

  toolbuttonOpen = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-open");
  gtk_widget_show (toolbuttonOpen);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonOpen);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonOpen), tooltips, QT_TR_NOOP("Open filter list [Ctrl-O]"), NULL);
  gtk_widget_add_accelerator (toolbuttonOpen, "clicked", accel_group,
                              GDK_O, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonOpen), TRUE);

  toolbuttonSave = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-save");
  gtk_widget_show (toolbuttonSave);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonSave);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonSave), tooltips, QT_TR_NOOP("Save filter list [Ctrl-S]"), NULL);
  gtk_widget_add_accelerator (toolbuttonSave, "clicked", accel_group,
                              GDK_S, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonSave), TRUE);

  tmp_image = gtk_image_new_from_stock ("gtk-save-as", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonScript = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Save Script"));
  gtk_widget_show (toolbuttonScript);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonScript);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonScript), tooltips, QT_TR_NOOP("Save as script [Ctrl-J]"), NULL);
  gtk_widget_add_accelerator (toolbuttonScript, "clicked", accel_group,
                              GDK_J, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonScript), TRUE);

  tmp_image = gtk_image_new_from_stock ("gtk-cdrom", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonDVD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("DVD Res"));
  gtk_widget_show (toolbuttonDVD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonDVD);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonDVD), tooltips, QT_TR_NOOP("DVD resolution [Ctrl-1]"), NULL);
  gtk_widget_add_accelerator (toolbuttonDVD, "clicked", accel_group,
                              GDK_1, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonDVD), TRUE);

  tmp_image = gtk_image_new_from_stock ("gtk-cdrom", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonHalfD1 = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("Half D1 Res"));
  gtk_widget_show (toolbuttonHalfD1);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonHalfD1);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonHalfD1), tooltips, QT_TR_NOOP("Half D1 resolution [Ctrl-2]"), NULL);
  gtk_widget_add_accelerator (toolbuttonHalfD1, "clicked", accel_group,
                              GDK_2, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonHalfD1), TRUE);

  tmp_image = gtk_image_new_from_stock ("gtk-cdrom", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonSVCD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("SVCD Res"));
  gtk_widget_show (toolbuttonSVCD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonSVCD);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonSVCD), tooltips, QT_TR_NOOP("SVCD resolution [Ctrl-3]"), NULL);
  gtk_widget_add_accelerator (toolbuttonSVCD, "clicked", accel_group,
                              GDK_3, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonSVCD), TRUE);

  tmp_image = gtk_image_new_from_stock ("gtk-cdrom", tmp_toolbar_icon_size);
  gtk_widget_show (tmp_image);
  toolbuttonVCD = (GtkWidget*) gtk_tool_button_new (tmp_image, QT_TR_NOOP("VCD Res"));
  gtk_widget_show (toolbuttonVCD);
  gtk_container_add (GTK_CONTAINER (toolbar1), toolbuttonVCD);
  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (toolbuttonVCD), tooltips, QT_TR_NOOP("VCD resolution [Ctrl-4]"), NULL);
  gtk_widget_add_accelerator (toolbuttonVCD, "clicked", accel_group,
                              GDK_4, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_tool_item_set_is_important (GTK_TOOL_ITEM (toolbuttonVCD), TRUE);

  hbox11 = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (hbox11);
  gtk_box_pack_start (GTK_BOX (vbox3b), hbox11, TRUE, TRUE, 0);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox11), frame1, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 6, 0, 18, 0);

  vbox2 = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (alignment1), vbox2);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox2), notebook1, TRUE, TRUE, 0);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook1), FALSE);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook1), GTK_POS_LEFT);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  treeview1 = gtk_tree_view_new ();
  gtk_widget_show (treeview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);
  gtk_widget_set_size_request (treeview1, 288, 336);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview1), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview1), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview1), FALSE);

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), hbox5);

  image1 = create_pixmap (dialog1, "1.png");
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox5), image1, FALSE, FALSE, 0);

  label11 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Transform"));
  gtk_widget_show (label11);
  gtk_box_pack_start (GTK_BOX (hbox5), label11, FALSE, FALSE, 4);
  gtk_label_set_use_markup (GTK_LABEL (label11), TRUE);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview2 = gtk_tree_view_new ();
  gtk_widget_show (treeview2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), treeview2);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview2), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview2), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview2), FALSE);

  hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), hbox6);

  image2 = create_pixmap (dialog1, "2.png");
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox6), image2, FALSE, FALSE, 0);

  label17 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Interlacing"));
  gtk_widget_show (label17);
  gtk_box_pack_start (GTK_BOX (hbox6), label17, FALSE, FALSE, 4);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview3 = gtk_tree_view_new ();
  gtk_widget_show (treeview3);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), treeview3);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview3), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview3), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview3), FALSE);

  hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox7);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), hbox7);

  image3 = create_pixmap (dialog1, "4.png");
  gtk_widget_show (image3);
  gtk_box_pack_start (GTK_BOX (hbox7), image3, FALSE, FALSE, 0);

  label18 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Colors"));
  gtk_widget_show (label18);
  gtk_box_pack_start (GTK_BOX (hbox7), label18, FALSE, FALSE, 4);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow4);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview4 = gtk_tree_view_new ();
  gtk_widget_show (treeview4);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), treeview4);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview4), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview4), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview4), FALSE);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox8);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), hbox8);

  image4 = create_pixmap (dialog1, "5.png");
  gtk_widget_show (image4);
  gtk_box_pack_start (GTK_BOX (hbox8), image4, FALSE, FALSE, 0);

  label19 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Noise"));
  gtk_widget_show (label19);
  gtk_box_pack_start (GTK_BOX (hbox8), label19, FALSE, FALSE, 4);

  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow5);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview5 = gtk_tree_view_new ();
  gtk_widget_show (treeview5);
  gtk_container_add (GTK_CONTAINER (scrolledwindow5), treeview5);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview5), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview5), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview5), FALSE);

  hbox9 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox9);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), hbox9);

  image5 = create_pixmap (dialog1, "3.png");
  gtk_widget_show (image5);
  gtk_box_pack_start (GTK_BOX (hbox9), image5, FALSE, FALSE, 0);

  label20 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Sharpness"));
  gtk_widget_show (label20);
  gtk_box_pack_start (GTK_BOX (hbox9), label20, FALSE, FALSE, 4);

  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow6);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview6 = gtk_tree_view_new ();
  gtk_widget_show (treeview6);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), treeview6);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview6), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview6), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview6), FALSE);

  hbox10 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox10);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), hbox10);

  image6 = create_pixmap (dialog1, "7.png");
  gtk_widget_show (image6);
  gtk_box_pack_start (GTK_BOX (hbox10), image6, FALSE, FALSE, 0);

  label21 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Subtitles"));
  gtk_widget_show (label21);
  gtk_box_pack_start (GTK_BOX (hbox10), label21, FALSE, FALSE, 4);

  scrolledwindow7 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow7);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow7);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow7), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  treeview7 = gtk_tree_view_new ();
  gtk_widget_show (treeview7);
  gtk_container_add (GTK_CONTAINER (scrolledwindow7), treeview7);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview7), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview7), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview7), FALSE);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 6), hbox4);

  image7 = create_pixmap (dialog1, "6.png");
  gtk_widget_show (image7);
  gtk_box_pack_start (GTK_BOX (hbox4), image7, FALSE, FALSE, 0);

  label22 = gtk_label_new_with_mnemonic (QT_TR_NOOP("Misc"));
  gtk_widget_show (label22);
  gtk_box_pack_start (GTK_BOX (hbox4), label22, FALSE, FALSE, 4);

  treeview8 = gtk_tree_view_new ();
  gtk_widget_show (treeview8);
  gtk_container_add (GTK_CONTAINER (notebook1), treeview8);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview8), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview8), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview8), FALSE);

  label28 = gtk_label_new (QT_TR_NOOP("External"));
  gtk_widget_show (label28);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 7), label28);

  hbox13 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox13);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox13, FALSE, FALSE, 0);

  buttonAdd = gtk_button_new ();
  gtk_widget_show (buttonAdd);
  gtk_box_pack_end (GTK_BOX (hbox13), buttonAdd, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonAdd, QT_TR_NOOP("Add selected filter to the Active Filters list"), NULL);

  image11 = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image11);
  gtk_container_add (GTK_CONTAINER (buttonAdd), image11);

  label23 = gtk_label_new (QT_TR_NOOP("<b>Available Filters</b>"));
  gtk_widget_show (label23);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label23);
  gtk_label_set_use_markup (GTK_LABEL (label23), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label23), 1, 1);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox11), frame2, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame2), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 6, 0, 18, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox1);

  vbox3 = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  scrolledwindow9 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow9);
  gtk_box_pack_start (GTK_BOX (vbox3), scrolledwindow9, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow9), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow9), GTK_SHADOW_OUT);

  treeview0 = gtk_tree_view_new ();
  gtk_widget_show (treeview0);
  gtk_container_add (GTK_CONTAINER (scrolledwindow9), treeview0);
  gtk_widget_set_size_request (treeview0, 288, 336);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview0), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview0), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (treeview0), FALSE);

  hbox14 = gtk_hbox_new (FALSE, 6);
  gtk_widget_show (hbox14);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox14, FALSE, FALSE, 0);

  buttonRemove = gtk_button_new ();
  gtk_widget_show (buttonRemove);
  gtk_box_pack_end (GTK_BOX (hbox14), buttonRemove, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonRemove, QT_TR_NOOP("Remove filter"), NULL);

  image15 = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image15);
  gtk_container_add (GTK_CONTAINER (buttonRemove), image15);

  buttonDown = gtk_button_new ();
  gtk_widget_show (buttonDown);
  gtk_box_pack_end (GTK_BOX (hbox14), buttonDown, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonDown, QT_TR_NOOP("Move filter down"), NULL);

  image14 = gtk_image_new_from_icon_name ("gtk-go-down", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image14);
  gtk_container_add (GTK_CONTAINER (buttonDown), image14);

  buttonUp = gtk_button_new ();
  gtk_widget_show (buttonUp);
  gtk_box_pack_end (GTK_BOX (hbox14), buttonUp, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonUp, QT_TR_NOOP("Move filter up"), NULL);

  image13 = gtk_image_new_from_icon_name ("gtk-go-up", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image13);
  gtk_container_add (GTK_CONTAINER (buttonUp), image13);

  buttonPartial = gtk_button_new_with_mnemonic (QT_TR_NOOP("P_artial"));
  gtk_widget_show (buttonPartial);
  gtk_box_pack_end (GTK_BOX (hbox14), buttonPartial, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonPartial, QT_TR_NOOP("Apply the current filter only to a part of the file"), NULL);

  buttonProperties = gtk_button_new ();
  gtk_widget_show (buttonProperties);
  gtk_box_pack_end (GTK_BOX (hbox14), buttonProperties, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonProperties, QT_TR_NOOP("Configure filter"), NULL);

  alignment4 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment4);
  gtk_container_add (GTK_CONTAINER (buttonProperties), alignment4);

  hbox16 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox16);
  gtk_container_add (GTK_CONTAINER (alignment4), hbox16);

  label25 = gtk_label_new_with_mnemonic (QT_TR_NOOP("C_onfigure"));
  gtk_widget_show (label25);
  gtk_box_pack_start (GTK_BOX (hbox16), label25, FALSE, FALSE, 0);

  label2 = gtk_label_new (QT_TR_NOOP("<b >Active Filters</b>"));
  gtk_widget_show (label2);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label2);
  gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label2), 1, 1);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  buttonPreview = gtk_button_new ();
  gtk_widget_show (buttonPreview);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonPreview, GTK_RESPONSE_APPLY);
  GTK_WIDGET_SET_FLAGS (buttonPreview, GTK_CAN_DEFAULT);

  alignment5 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment5);
  gtk_container_add (GTK_CONTAINER (buttonPreview), alignment5);

  hbox17 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox17);
  gtk_container_add (GTK_CONTAINER (alignment5), hbox17);

  image17 = create_pixmap (dialog1, "preview-button.png");
  gtk_widget_show (image17);
  gtk_box_pack_start (GTK_BOX (hbox17), image17, FALSE, FALSE, 0);

  label26 = gtk_label_new_with_mnemonic (QT_TR_NOOP("_Preview"));
  gtk_widget_show (label26);
  gtk_box_pack_start (GTK_BOX (hbox17), label26, FALSE, FALSE, 0);

  buttonClose = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (buttonClose);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonClose, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (buttonClose, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox3b, "vbox3b");
  GLADE_HOOKUP_OBJECT (dialog1, toolbar1, "toolbar1");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonOpen, "toolbuttonOpen");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonSave, "toolbuttonSave");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonScript, "toolbuttonScript");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonDVD, "toolbuttonDVD");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonHalfD1, "toolbuttonHalfD1");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonSVCD, "toolbuttonSVCD");
  GLADE_HOOKUP_OBJECT (dialog1, toolbuttonVCD, "toolbuttonVCD");
  GLADE_HOOKUP_OBJECT (dialog1, hbox11, "hbox11");
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (dialog1, treeview1, "treeview1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (dialog1, image1, "image1");
  GLADE_HOOKUP_OBJECT (dialog1, label11, "label11");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (dialog1, treeview2, "treeview2");
  GLADE_HOOKUP_OBJECT (dialog1, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (dialog1, image2, "image2");
  GLADE_HOOKUP_OBJECT (dialog1, label17, "label17");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow3, "scrolledwindow3");
  GLADE_HOOKUP_OBJECT (dialog1, treeview3, "treeview3");
  GLADE_HOOKUP_OBJECT (dialog1, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (dialog1, image3, "image3");
  GLADE_HOOKUP_OBJECT (dialog1, label18, "label18");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow4, "scrolledwindow4");
  GLADE_HOOKUP_OBJECT (dialog1, treeview4, "treeview4");
  GLADE_HOOKUP_OBJECT (dialog1, hbox8, "hbox8");
  GLADE_HOOKUP_OBJECT (dialog1, image4, "image4");
  GLADE_HOOKUP_OBJECT (dialog1, label19, "label19");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow5, "scrolledwindow5");
  GLADE_HOOKUP_OBJECT (dialog1, treeview5, "treeview5");
  GLADE_HOOKUP_OBJECT (dialog1, hbox9, "hbox9");
  GLADE_HOOKUP_OBJECT (dialog1, image5, "image5");
  GLADE_HOOKUP_OBJECT (dialog1, label20, "label20");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow6, "scrolledwindow6");
  GLADE_HOOKUP_OBJECT (dialog1, treeview6, "treeview6");
  GLADE_HOOKUP_OBJECT (dialog1, hbox10, "hbox10");
  GLADE_HOOKUP_OBJECT (dialog1, image6, "image6");
  GLADE_HOOKUP_OBJECT (dialog1, label21, "label21");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow7, "scrolledwindow7");
  GLADE_HOOKUP_OBJECT (dialog1, treeview7, "treeview7");
  GLADE_HOOKUP_OBJECT (dialog1, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (dialog1, image7, "image7");
  GLADE_HOOKUP_OBJECT (dialog1, label22, "label22");
  GLADE_HOOKUP_OBJECT (dialog1, treeview8, "treeview8");
  GLADE_HOOKUP_OBJECT (dialog1, label28, "label28");
  GLADE_HOOKUP_OBJECT (dialog1, hbox13, "hbox13");
  GLADE_HOOKUP_OBJECT (dialog1, buttonAdd, "buttonAdd");
  GLADE_HOOKUP_OBJECT (dialog1, image11, "image11");
  GLADE_HOOKUP_OBJECT (dialog1, label23, "label23");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow9, "scrolledwindow9");
  GLADE_HOOKUP_OBJECT (dialog1, treeview0, "treeview0");
  GLADE_HOOKUP_OBJECT (dialog1, hbox14, "hbox14");
  GLADE_HOOKUP_OBJECT (dialog1, buttonRemove, "buttonRemove");
  GLADE_HOOKUP_OBJECT (dialog1, image15, "image15");
  GLADE_HOOKUP_OBJECT (dialog1, buttonDown, "buttonDown");
  GLADE_HOOKUP_OBJECT (dialog1, image14, "image14");
  GLADE_HOOKUP_OBJECT (dialog1, buttonUp, "buttonUp");
  GLADE_HOOKUP_OBJECT (dialog1, image13, "image13");
  GLADE_HOOKUP_OBJECT (dialog1, buttonPartial, "buttonPartial");
  GLADE_HOOKUP_OBJECT (dialog1, buttonProperties, "buttonProperties");
  GLADE_HOOKUP_OBJECT (dialog1, alignment4, "alignment4");
  GLADE_HOOKUP_OBJECT (dialog1, hbox16, "hbox16");
  GLADE_HOOKUP_OBJECT (dialog1, label25, "label25");
  GLADE_HOOKUP_OBJECT (dialog1, label2, "label2");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonPreview, "buttonPreview");
  GLADE_HOOKUP_OBJECT (dialog1, alignment5, "alignment5");
  GLADE_HOOKUP_OBJECT (dialog1, hbox17, "hbox17");
  GLADE_HOOKUP_OBJECT (dialog1, image17, "image17");
  GLADE_HOOKUP_OBJECT (dialog1, label26, "label26");
  GLADE_HOOKUP_OBJECT (dialog1, buttonClose, "buttonClose");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, tooltips, "tooltips");

  gtk_window_add_accel_group (GTK_WINDOW (dialog1), accel_group);

  return dialog1;
}


/***************************************************************************
                          DIA_threshold.cpp  -  configuration dialog for
						threshold filter
                              -------------------
                         Chris MacGregor, September 2007
                         chris-avidemux@bouncingdog.com
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

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_videoFilter/ADM_vidThreshold.h"

#include "DIA_flyDialog.h"
#include "DIA_flyThreshold.h"


#undef _
#define _(_s) QT_TR_NOOP(_s)

/********************************************************************/
static GtkWidget *create_threshold_dialog (void);
static GtkWidget *dialog = 0;

static gboolean gui_draw (GtkWidget * widget,
			  GdkEventExpose * event, gpointer user_data);
static void gui_update (GtkButton * button, gpointer user_data);
static void frame_changed (void);

static volatile int lock = 0;
static flyThreshold * myDialog = 0;

/********************************************************************/

static gboolean previewButtonEvent (GtkWidget * widget,
                                    GdkEventButton * event,
                                    gpointer data)
{
    if (event->type != GDK_BUTTON_PRESS)
        return FALSE;

    flyThreshold * myFly = static_cast <flyThreshold *> (data);

    uint32_t x, y;

    float zoom = myFly->getZoom();
    if (zoom == 1.0)
    {
        x = static_cast <uint32_t> (event->x);
        y = static_cast <uint32_t> (event->y);
    }
    else
    {
        zoom = 1 / zoom;
        x = static_cast <uint32_t> (event->x * zoom + .5);
        y = static_cast <uint32_t> (event->y * zoom + .5);
    }

    ADMImage * image = event->button == 1
        ? myFly->getInputImage() : myFly->getOutputImage();

    uint32_t width = image->_width;
    uint32_t height = image->_height;

    if (x >= width || y >= height)
        return FALSE;

    uint8_t range = 3;
    if (event->state & GDK_SHIFT_MASK)
        range += 2;
    if (event->state & GDK_CONTROL_MASK)
        range += 4;
    if (event->state & GDK_MOD1_MASK)
        range += 6;

    uint8_t halfrange = range / 2;

    uint32_t xmin = (x > halfrange) ? x - halfrange : 0;
    uint32_t ymin = (y > halfrange) ? y - halfrange : 0;

    uint32_t xmax = x + halfrange;
    uint32_t ymax = y + halfrange;

    if (xmax >= width)
        xmax = width - 1;
    if (ymax >= height)
        ymax = height - 1;

    uint8_t * imagepix = YPLANE (image);

    printf ("\n  x|");
    for (x = xmin; x <= xmax; x++)
        printf (" %3d", x);
    printf ("\ny: +");
    for (x = xmin; x <= xmax; x++)
        printf (" ---", x);
    printf ("\n");

    for (y = ymin; y <= ymax; y++)
    {
        uint8_t * pixp = imagepix + (y * width) + xmin;
        printf ("%3d|", y);
        for (x = xmin; x <= xmax; x++)
            printf (" %3d", *pixp++);
        printf ("\n");
    }

    return TRUE;
}

/********************************************************************/

static void previewOutputMenuChange (GtkComboBox * combo, gpointer user_data)
{
    flyThreshold * myFly = static_cast <flyThreshold *> (user_data);
    uint32_t index = gtk_combo_box_get_active (combo);
    uint32_t filter_count;
    FILTER * filters = getCurrentVideoFilterList (&filter_count);
    FILTER * filter = filters + index;
    VF_FILTERS tag = filter->tag;

    gchar * activestr = gtk_combo_box_get_active_text (combo);

    printf ("user selected preview of #%d = %s (%d) @%p (was %p)\n",
            index, activestr, tag, filter->filter, myFly->getSource());

    if (strncmp (activestr, "XX ", 3) == 0)
    {
        printf ("selected preview source has different dimensions - "
                "forcing selection to current filter\n");
        gtk_combo_box_set_active (combo, myFly->this_filter_index);
    }
    else
    {
        flyThreshold::PreviewMode mode;
        if (index == myFly->this_filter_index)
            mode = flyThreshold::PREVIEWMODE_THIS_FILTER;
        else if (index > myFly->this_filter_index)
            mode = flyThreshold::PREVIEWMODE_LATER_FILTER;
        else // if (index < myFly->this_filter_index)
            mode = flyThreshold::PREVIEWMODE_EARLIER_FILTER;

        myFly->changeSource (filter->filter, mode);
    }

    g_free (activestr);
}

/********************************************************************/

static gboolean preview_video_configured (GtkWidget * widget,
                                          GdkEventConfigure * event,
                                          gpointer user_data)
{
    fprintf (stderr, "preview_configured: now %dx%d @ +%d+%d\n",
             event->width, event->height, event->x, event->y);

    flyThreshold * myFly = static_cast <flyThreshold *> (user_data);
    myFly->recomputeSize();

    return FALSE;
}

/********************************************************************/

uint8_t DIA_threshold (AVDMGenericVideoStream *in,
                       ADMVideoThreshold * thresholdp,
                       THRESHOLD_PARAM * param)
{
    // Allocate space for preview video
    uint32_t width = in->getInfo()->width;
    uint32_t height = in->getInfo()->height;

    dialog = create_threshold_dialog();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
										GTK_RESPONSE_OK,
										GTK_RESPONSE_CANCEL,
										-1);

    gtk_register_dialog(dialog);
    gtk_window_set_title (GTK_WINDOW (dialog),
                          QT_TR_NOOP("Threshold Configuration"));
    gtk_widget_show(dialog);

    myDialog = new flyThreshold (width, height, in,
                                 WID(previewVideo), WID(previewSlider),
                                 thresholdp, param);

    g_signal_connect (GTK_OBJECT (WID(previewVideo)), "configure-event",
                      GTK_SIGNAL_FUNC (preview_video_configured),
                      gpointer (myDialog));

    myDialog->upload();
    myDialog->sliderChanged();

    // update things when settings are changed
#define CNX(_widg,_signame) \
    g_signal_connect(GTK_OBJECT(WID(_widg)), _signame,                \
                     GTK_SIGNAL_FUNC(gui_update), (void *) (1));

//    CNX (minValueSlider, "drag_data_received");
    CNX (minValueSpinner, "value_changed");

//    CNX (maxValueSlider, "drag_data_received");
    CNX (maxValueSpinner, "value_changed");
      
    CNX (outputValuesMenu, "changed");

    g_signal_connect(GTK_OBJECT(WID(previewSlider)), "value_changed",
                     GTK_SIGNAL_FUNC(frame_changed), 0);
    g_signal_connect(GTK_OBJECT(WID(previewVideo)), "expose_event",
                     GTK_SIGNAL_FUNC(gui_draw), 0);

    g_signal_connect(GTK_OBJECT(WID(previewVideo)), "button_press_event",
                     GTK_SIGNAL_FUNC(previewButtonEvent),
                     gpointer(myDialog));
#if 0
    g_signal_connect(GTK_OBJECT(WID(previewVideo)), "motion_notify_event",
                     GTK_SIGNAL_FUNC(previewMotionEvent),
                     gpointer(myDialog));
#endif

    GtkWidget * previewOutputMenu = WID(previewOutputMenu);
    uint32_t filter_count;
    FILTER * filters = getCurrentVideoFilterList (&filter_count);
    int32_t active = -1;

    // The " + (active < 0)" below is a bit of a hack.  We know that in
    // on_action() in gui_filtermanager.cpp, case A_ADD, the new filter-to-be
    // is added to the filter list without incrementing nb_active_filter yet.
    // So if we get to the end of the list and haven't yet found the filter
    // that we're configuring, we know it's a new one and therefore that it is
    // one past the apparent end of the list.  It's not a clean solution, but
    // it seems like the cleanEST solution.

    for (uint32_t i = 0; i < filter_count + (active < 0); i++)
    {
        const char * name
            = (i == 0) ? "(input)" : filterGetNameFromTag (filters [i].tag);
        bool free_name = false;
                                   
        FILTER * filter = filters + i;
        AVDMGenericVideoStream * source = filter->filter;
        uint32_t w = source->getInfo()->width;
        uint32_t h = source->getInfo()->height;
        if (w != width || h != height)
        {
            name = g_strconcat ("XX ", name, " XX", NULL);
            free_name = true;
        }

        printf ("filter [%d] = %s (%d) @ %p; %dx%d\n",
                i, name, filter->tag, source, w, h);
        gtk_combo_box_append_text (GTK_COMBO_BOX (previewOutputMenu), name);
        if (filter->filter == myDialog->getSource())
        {
            gtk_combo_box_set_active (GTK_COMBO_BOX (previewOutputMenu), i);
            printf ("\tfilter [%d] is being configured now\n", i);
            active = i;
        }

        if (free_name)
            g_free (const_cast <char *> (name));
    }

    ADM_assert (active >= 0);
    myDialog->this_filter_index = active;

    g_signal_connect (GTK_OBJECT(previewOutputMenu), "changed",
                      GTK_SIGNAL_FUNC(previewOutputMenuChange),
                      gpointer(myDialog));

    uint8_t ret = 0;
    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
        myDialog->download();
        myDialog->pushParam();
        ret = 1;
    }
    else
        myDialog->restoreParam();

    gtk_unregister_dialog(dialog);
    gtk_widget_destroy(dialog);

    delete myDialog;

    return ret;
}

void frame_changed (void)
{
    myDialog->sliderChanged();
}

void gui_update (GtkButton * button, gpointer user_data)
{
    if (lock)
        return;

    myDialog->update();
}

gboolean gui_draw (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
    myDialog->display();
    return TRUE;
}

/**************************************/

uint8_t flyThreshold::upload (void)
{
    lock++;

    gtk_spin_button_set_value (GTK_SPIN_BUTTON(WID(minValueSpinner)), param.min);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(WID(maxValueSpinner)), param.max);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON(WID(debugSpinButton)), param.debug);
    gtk_combo_box_set_active (GTK_COMBO_BOX(WID(outputValuesMenu)),
                              param.in_range_is_white);

    lock--;
    return 1;
}

uint8_t flyThreshold::download (void)
{
    param.min = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (WID(minValueSpinner)));
    param.max = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (WID(maxValueSpinner)));
    param.debug = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (WID(debugSpinButton)));
    param.in_range_is_white
        = gtk_combo_box_get_active (GTK_COMBO_BOX(WID(outputValuesMenu)));

    return 1;
}

// The following was generated by glade once upon a time.  It has since been
// tweaked by hand (to share the GtkAdjustment objects between the spinbutton
// and hscale, for instance), so beware...

GtkWidget*
create_threshold_dialog (void)
{
  GtkWidget *threshold_dialog;
  GtkWidget *dialogVbox;
  GtkWidget *allButButtonsVbox;
  GtkWidget *settingsOuterHbox;
  GtkWidget *settingsOuterVbox;
  GtkWidget *minValueVbox;
  GtkWidget *minValueLabel;
  GtkWidget *minValueHbox;
  GtkWidget *minValueSlider;
  GtkObject *minValueSpinner_adj;
  GtkWidget *minValueSpinner;
  GtkWidget *maxValueVbox;
  GtkWidget *maxValueLabel;
  GtkWidget *maxValueHbox;
  GtkWidget *maxValueSlider;
  GtkObject *maxValueSpinner_adj;
  GtkWidget *maxValueSpinner;
  GtkWidget *outputValuesHbox;
  GtkWidget *outputValuesLabel;
  GtkWidget *outputValuesMenu;
  GtkWidget *debugHbox;
  GtkWidget *debugLabel;
  GtkObject *debugSpinButton_adj;
  GtkWidget *debugSpinButton;
  GtkWidget *previewFrame;
  GtkWidget *previewAlignment;
  GtkWidget *previewVbox;
  GtkWidget *previewControlHbox;
  GtkWidget *previewOutputMenu;
  GtkWidget *previewSlider;
  GtkWidget *previewVideo;
  GtkWidget *previewLabel;
  GtkWidget *dialogButtonBox;
  GtkWidget *cancelButton;
  GtkWidget *okButton;

  threshold_dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (threshold_dialog), QT_TR_NOOP("Threshold"));
  gtk_window_set_type_hint (GTK_WINDOW (threshold_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialogVbox = GTK_DIALOG (threshold_dialog)->vbox;
  gtk_widget_show (dialogVbox);

  allButButtonsVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (allButButtonsVbox);
  gtk_box_pack_start (GTK_BOX (dialogVbox), allButButtonsVbox, TRUE, TRUE, 0);

  settingsOuterHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (settingsOuterHbox);
  gtk_box_pack_start (GTK_BOX (allButButtonsVbox), settingsOuterHbox, FALSE, TRUE, 0);

  settingsOuterVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (settingsOuterVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterHbox), settingsOuterVbox, TRUE, TRUE, 0);

  minValueVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (minValueVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), minValueVbox, FALSE, FALSE, 0);

  minValueLabel = gtk_label_new (QT_TR_NOOP("Minimum value to be in-range:"));
  gtk_widget_show (minValueLabel);
  gtk_box_pack_start (GTK_BOX (minValueVbox), minValueLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (minValueLabel), 0.5, 1);
  gtk_misc_set_padding (GTK_MISC (minValueLabel), 0, 8);

  minValueHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (minValueHbox);
  gtk_box_pack_start (GTK_BOX (minValueVbox), minValueHbox, TRUE, TRUE, 0);

#ifdef ORIGINAL_CODE_GENERATED_BY_GLADE
  minValueSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 255, 1, 1, 0)));
#else
  minValueSpinner_adj = gtk_adjustment_new (0, 0, 255, 1, 1, 0);
  minValueSlider = gtk_hscale_new (GTK_ADJUSTMENT (minValueSpinner_adj));
#endif
  gtk_widget_show (minValueSlider);
  gtk_box_pack_start (GTK_BOX (minValueHbox), minValueSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (minValueSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (minValueSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (minValueSlider), 0);

#ifdef ORIGINAL_CODE_GENERATED_BY_GLADE
  minValueSpinner_adj = gtk_adjustment_new (0, 0, 255, 1, 10, 10);
#endif
  minValueSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (minValueSpinner_adj), 1, 0);
  gtk_widget_show (minValueSpinner);
  gtk_box_pack_start (GTK_BOX (minValueHbox), minValueSpinner, FALSE, TRUE, 0);

  maxValueVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (maxValueVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), maxValueVbox, FALSE, FALSE, 0);

  maxValueLabel = gtk_label_new (QT_TR_NOOP("Maximum value to be in-range:"));
  gtk_widget_show (maxValueLabel);
  gtk_box_pack_start (GTK_BOX (maxValueVbox), maxValueLabel, FALSE, FALSE, 0);

  maxValueHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (maxValueHbox);
  gtk_box_pack_start (GTK_BOX (maxValueVbox), maxValueHbox, TRUE, TRUE, 0);

#ifdef ORIGINAL_CODE_GENERATED_BY_GLADE
  maxValueSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (255, 0, 255, 1, 1, 0)));
#else
  maxValueSpinner_adj = gtk_adjustment_new (255, 0, 255, 1, 1, 0);
  maxValueSlider = gtk_hscale_new (GTK_ADJUSTMENT (maxValueSpinner_adj));
#endif
  gtk_widget_show (maxValueSlider);
  gtk_box_pack_start (GTK_BOX (maxValueHbox), maxValueSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (maxValueSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (maxValueSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (maxValueSlider), 0);

#ifdef ORIGINAL_CODE_GENERATED_BY_GLADE
  maxValueSpinner_adj = gtk_adjustment_new (255, 0, 255, 1, 10, 10);
#endif
  maxValueSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (maxValueSpinner_adj), 1, 0);
  gtk_widget_show (maxValueSpinner);
  gtk_box_pack_start (GTK_BOX (maxValueHbox), maxValueSpinner, FALSE, TRUE, 0);

  outputValuesHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (outputValuesHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), outputValuesHbox, TRUE, TRUE, 0);

  outputValuesLabel = gtk_label_new (QT_TR_NOOP("Output values:   "));
  gtk_widget_show (outputValuesLabel);
  gtk_box_pack_start (GTK_BOX (outputValuesHbox), outputValuesLabel, FALSE, FALSE, 0);

  outputValuesMenu = gtk_combo_box_new_text ();
  gtk_widget_show (outputValuesMenu);
  gtk_box_pack_start (GTK_BOX (outputValuesHbox), outputValuesMenu, TRUE, TRUE, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (outputValuesMenu), QT_TR_NOOP("In-range values go black, out-of-range values go white"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (outputValuesMenu), QT_TR_NOOP("In-range values go white, out-of-range values go black"));

  debugHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (debugHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), debugHbox, TRUE, TRUE, 0);

  debugLabel = gtk_label_new (QT_TR_NOOP("Debugging settings (bits):   "));
  gtk_widget_show (debugLabel);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugLabel, FALSE, FALSE, 0);

  debugSpinButton_adj = gtk_adjustment_new (0, 0, 16777215, 1, 10, 10);
  debugSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (debugSpinButton_adj), 1, 0);
  gtk_widget_show (debugSpinButton);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugSpinButton, TRUE, TRUE, 0);

  previewFrame = gtk_frame_new (NULL);
  gtk_widget_show (previewFrame);
  gtk_box_pack_start (GTK_BOX (allButButtonsVbox), previewFrame, TRUE, TRUE, 0);

  previewAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (previewAlignment);
  gtk_container_add (GTK_CONTAINER (previewFrame), previewAlignment);
  gtk_alignment_set_padding (GTK_ALIGNMENT (previewAlignment), 0, 8, 6, 8);

  previewVbox = gtk_vbox_new (FALSE, 5);
  gtk_widget_show (previewVbox);
  gtk_container_add (GTK_CONTAINER (previewAlignment), previewVbox);

  previewControlHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (previewControlHbox);
  gtk_box_pack_start (GTK_BOX (previewVbox), previewControlHbox, TRUE, TRUE, 0);

  previewOutputMenu = gtk_combo_box_new_text ();
  gtk_widget_show (previewOutputMenu);
  gtk_box_pack_start (GTK_BOX (previewControlHbox), previewOutputMenu, FALSE, TRUE, 0);

  previewSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 99, 1, 1, 1)));
  gtk_widget_show (previewSlider);
  gtk_box_pack_start (GTK_BOX (previewControlHbox), previewSlider, TRUE, TRUE, 0);
  gtk_scale_set_digits (GTK_SCALE (previewSlider), 0);

  previewVideo = gtk_drawing_area_new ();
  gtk_widget_show (previewVideo);
  gtk_box_pack_start (GTK_BOX (previewVbox), previewVideo, TRUE, TRUE, 0);
  gtk_widget_set_size_request (previewVideo, 30, 30);
  gtk_widget_set_events (previewVideo, GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK);

  previewLabel = gtk_label_new (QT_TR_NOOP("Preview"));
  gtk_widget_show (previewLabel);
  gtk_frame_set_label_widget (GTK_FRAME (previewFrame), previewLabel);

  dialogButtonBox = GTK_DIALOG (threshold_dialog)->action_area;
  gtk_widget_show (dialogButtonBox);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialogButtonBox), GTK_BUTTONBOX_END);

  cancelButton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (threshold_dialog), cancelButton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelButton, GTK_CAN_DEFAULT);

  okButton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (threshold_dialog), okButton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okButton, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (threshold_dialog, threshold_dialog, "threshold_dialog");
  GLADE_HOOKUP_OBJECT_NO_REF (threshold_dialog, dialogVbox, "dialogVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, allButButtonsVbox, "allButButtonsVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, settingsOuterHbox, "settingsOuterHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, settingsOuterVbox, "settingsOuterVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, minValueVbox, "minValueVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, minValueLabel, "minValueLabel");
  GLADE_HOOKUP_OBJECT (threshold_dialog, minValueHbox, "minValueHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, minValueSlider, "minValueSlider");
  GLADE_HOOKUP_OBJECT (threshold_dialog, minValueSpinner, "minValueSpinner");
  GLADE_HOOKUP_OBJECT (threshold_dialog, maxValueVbox, "maxValueVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, maxValueLabel, "maxValueLabel");
  GLADE_HOOKUP_OBJECT (threshold_dialog, maxValueHbox, "maxValueHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, maxValueSlider, "maxValueSlider");
  GLADE_HOOKUP_OBJECT (threshold_dialog, maxValueSpinner, "maxValueSpinner");
  GLADE_HOOKUP_OBJECT (threshold_dialog, outputValuesHbox, "outputValuesHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, outputValuesLabel, "outputValuesLabel");
  GLADE_HOOKUP_OBJECT (threshold_dialog, outputValuesMenu, "outputValuesMenu");
  GLADE_HOOKUP_OBJECT (threshold_dialog, debugHbox, "debugHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, debugLabel, "debugLabel");
  GLADE_HOOKUP_OBJECT (threshold_dialog, debugSpinButton, "debugSpinButton");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewFrame, "previewFrame");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewAlignment, "previewAlignment");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewVbox, "previewVbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewControlHbox, "previewControlHbox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewOutputMenu, "previewOutputMenu");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewSlider, "previewSlider");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewVideo, "previewVideo");
  GLADE_HOOKUP_OBJECT (threshold_dialog, previewLabel, "previewLabel");
  GLADE_HOOKUP_OBJECT_NO_REF (threshold_dialog, dialogButtonBox, "dialogButtonBox");
  GLADE_HOOKUP_OBJECT (threshold_dialog, cancelButton, "cancelButton");
  GLADE_HOOKUP_OBJECT (threshold_dialog, okButton, "okButton");

  return threshold_dialog;
}


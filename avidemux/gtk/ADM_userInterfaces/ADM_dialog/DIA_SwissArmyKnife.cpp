/***************************************************************************
                          DIA_SwissArmyKnife.cpp  -  configuration dialog for
						Swiss Army Knife filter
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
#include <cmath>

#include "fourcc.h"


#include "avi_vars.h"

#ifdef HAVE_ENCODER

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter/ADM_vidSwissArmyKnife.h"

#include "DIA_flyDialog.h"
#include "DIA_flySwissArmyKnife.h"
#include "DIA_factory.h"
#include "ADM_videoFilter_internal.h"

#undef _
#define _(_s) QT_TR_NOOP(_s)

/********************************************************************/

static GtkWidget *create_swiss_army_knife_dialog (void);
static GtkWidget *dialog = 0;

static gboolean gui_draw (GtkWidget * widget,
			  GdkEventExpose * event, gpointer user_data);
static void gui_update (GtkObject * button, gpointer user_data);
static void frame_changed (GtkRange *, gpointer user_data);

static volatile int lock = 0;

/********************************************************************/

static gboolean previewButtonEvent (GtkWidget *,
                                    GdkEventButton * event,
                                    gpointer data)
{
    if (event->type != GDK_BUTTON_PRESS)
        return FALSE;

    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (data);

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

static void alphaRecalcOther (GtkSpinButton * spinbutton, gpointer user_data)
{
    static int alpha_lock = 0;

    // avoid double screen updates (can be slow if the settings involve a lot
    // of calculation, such as recalculating the head start) due to recursive
    // calls to this function (e.g., it gets invoked on the denominator to a
    // change made by this function in response to the user changing the float
    // field).

    if (alpha_lock++)
    {
        --alpha_lock;
        return;
    }

    GtkSpinButton * targetspinbutton = GTK_SPIN_BUTTON (user_data);
    gtk_spin_button_set_value (targetspinbutton,
                               1 / gtk_spin_button_get_value (spinbutton));

    --alpha_lock;
}

/********************************************************************/

struct PixelValueScalingInfo
{
    GtkAdjustment * addAdjustment;
    GtkAdjustment * multAdjustment;
    GtkAdjustment * minAdjustment;
    GtkAdjustment * maxAdjustment;
    flySwissArmyKnife * myFly;
    double addValue;
    double multValue;
    double minValue;
    double maxValue;
    bool valuesSet;
    PixelValueScalingInfo (GtkAdjustment * add,
                           GtkAdjustment * mult,
                           GtkAdjustment * min,
                           GtkAdjustment * max,
                           flySwissArmyKnife * myFly)
        : addAdjustment (add),
          multAdjustment (mult),
          minAdjustment (min),
          maxAdjustment (max),
          myFly (myFly),
          valuesSet (false)

    {
    }
};

/********************************************************************/

static void updateScalingStuff (GtkAdjustment * adj, gpointer user_data)
{
    static int scaling_lock = 0;

    // avoid screwy behavior due to recursive calls to this function (e.g., if
    // it gets invoked due to a change that it initiated, just exit - no good
    // will come of trying to redo the calculations midstream).

    if (scaling_lock++)
    {
        // printf ("skipping recursive updateScalingStuff(%p)\n", adj);
        --scaling_lock;
        return;
    }

    PixelValueScalingInfo * info
        = static_cast <PixelValueScalingInfo *> (user_data);

    bool valueChanged = false;
    if (!info->valuesSet)
    {
        info->addValue = gtk_adjustment_get_value (info->addAdjustment);
        info->multValue = gtk_adjustment_get_value (info->multAdjustment);
        info->minValue = gtk_adjustment_get_value (info->minAdjustment);
        info->maxValue = gtk_adjustment_get_value (info->maxAdjustment);
        info->valuesSet = true;
        valueChanged = true; // force update - we have to assume it did change
    }

    // No matter who sent the signal, we will want the value from it.

    gdouble value = gtk_adjustment_get_value (adj);

    // We need to know which thing sent the signal so we propagate values in
    // the right directions.

    // Here's how we got the calculations below:
    //     P' = (P + add) * mult
    // (solve for each of mult, add, and P:)
    //     mult = P' / (P + add)
    //     add = (P' / mult) - P
    //     P = (P' / mult) - add
    // Thus:
    //     for min, where P' = 0, P = -add
    //     for max, where P' = 255, P = (255 / mult) - add
    // And so:
    //     add = -min
    //     mult = 255 / (max - min)
    //     min = -add
    //     max = (255 / mult) - add

    if (adj == info->addAdjustment)
    {
        // printf ("updateScalingStuff(%p) - add is now %.12f (was %.12f)\n",
        //         adj, value, info->addValue);
        if (valueChanged || fabs (value - info->addValue) >= 0.00001)
        {
            gtk_adjustment_set_value (info->minAdjustment, -value);
            gdouble mult_value
                = gtk_adjustment_get_value (info->multAdjustment);
            gtk_adjustment_set_value (info->maxAdjustment,
                                      (255 / mult_value) - value);
            info->addValue = value;
            valueChanged = true;
        }
    }
    else if (adj == info->multAdjustment)
    {
        // printf ("updateScalingStuff(%p) - mult is now %.12f (was %.12f)\n",
        //         adj, value, info->multValue);
        if (valueChanged || fabs (value - info->multValue) >= 0.00001)
        {
            gdouble add_value
                = gtk_adjustment_get_value (info->addAdjustment);
            gtk_adjustment_set_value (info->maxAdjustment,
                                      (255 / value) - add_value);
            info->multValue = value;
            valueChanged = true;
        }
    }
    else if (adj == info->minAdjustment)
    {
        // printf ("updateScalingStuff(%p) - min is now %.12f (was %.12f)\n",
        //         adj, value, info->minValue);
        if (valueChanged || fabs (value - info->minValue) >= 0.1)
        {
            gdouble max_value
                = gtk_adjustment_get_value (info->maxAdjustment);
            gtk_adjustment_set_value (info->addAdjustment, -value);
            gtk_adjustment_set_value (info->multAdjustment,
                                      255 / (max_value - value));
            info->minValue = value;
            valueChanged = true;
        }
    }
    else if (adj == info->maxAdjustment)
    {
        // printf ("updateScalingStuff(%p) - max is now %.12f (was %.12f)\n",
        //         adj, value, info->maxValue);
        if (valueChanged || fabs (value - info->maxValue) >= 0.1)
        {
            gdouble min_value
                = gtk_adjustment_get_value (info->minAdjustment);
            gtk_adjustment_set_value (info->multAdjustment,
                                      255 / (value - min_value));
            info->maxValue = value;
            valueChanged = true;
        }
    }
    else
    {
        // uh oh!
        fprintf (stderr, "updateScalingStuff() called with unknown adj\n");
        ADM_assert(0);
    }

    if (valueChanged)
    {
        // printf ("updateScalingStuff(%p) - about to gui_update()\n", adj);
        gui_update (GTK_OBJECT (adj), gpointer (info->myFly));
    }
    // printf ("updateScalingStuff(%p) - done\n", adj);

    --scaling_lock;
}

/********************************************************************/

static void inputTypeChange (GtkComboBox * combo, gpointer user_data)
{
    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (user_data);
    const MenuMapping * mm = myFly->lookupMenu ("inputTypeMenu");
    uint32_t inputType = myFly->getMenuValue (mm);

    GtkWidget * convolutionSettingsVbox = WID (convolutionSettingsVbox);
    GtkWidget * imageFileSettingsVbox = WID (imageFileSettingsVbox);
    GtkWidget * constantSettingsVbox = WID (constantSettingsVbox);
    GtkWidget * rollingAvgSettingsVbox = WID (rollingAvgSettingsVbox);

    gtk_widget_hide (convolutionSettingsVbox);
    gtk_widget_hide (imageFileSettingsVbox);
    gtk_widget_hide (constantSettingsVbox);
    gtk_widget_hide (rollingAvgSettingsVbox);

    switch (inputType)
    {
    case ADMVideoSwissArmyKnife::INPUT_CUSTOM_CONVOLUTION:
        gtk_widget_show (convolutionSettingsVbox);
        break;
    case ADMVideoSwissArmyKnife::INPUT_FILE_IMAGE_FLOAT:
    case ADMVideoSwissArmyKnife::INPUT_FILE_IMAGE_INTEGER:
        gtk_widget_show (imageFileSettingsVbox);
        break;
    case ADMVideoSwissArmyKnife::INPUT_CONSTANT_VALUE:
        gtk_widget_show (constantSettingsVbox);
        break;
    case ADMVideoSwissArmyKnife::INPUT_ROLLING_AVERAGE:
        gtk_widget_show (rollingAvgSettingsVbox);
        break;
    default:
        fprintf (stderr, "unknown input type %d!!!\n", inputType);
        ADM_assert(0);
    }

    myFly->updateConfigDescription (true);

    // If the preview window is enabled, then draw on it and clear the output
    // buffer so there isn't a leftover image (in the case where the config we
    // switched to can't output right now, e.g., invalid params - missing
    // filename, for instance).

    myFly->wipeOutputBuffer();

    if (myFly->param.enable_preview)
    {
        GtkWidget * previewVideo = WID(previewVideo);
        int max_x = previewVideo->allocation.width;
        int max_y = previewVideo->allocation.height;
        gdk_draw_rectangle
            (previewVideo->window,
             previewVideo->style->fg_gc[GTK_WIDGET_STATE (previewVideo)],
             TRUE, 0, 0, max_x, max_y);
    }

    myFly->download();
    myFly->pushParam();
    myFly->sliderChanged();
}

/********************************************************************/

static void previewEnableButton (GtkToggleButton * button, gpointer user_data)
{
    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (user_data);
    bool isActive = gtk_toggle_button_get_active (button);
    myFly->param.enable_preview = isActive;

    GtkWidget * previewVideo = WID(previewVideo);
    GtkWidget * previewDisabled = WID(previewDisabled);

    if (isActive)
    {
        gtk_widget_hide (previewDisabled);
        gtk_widget_show (previewVideo);
        myFly->download();
        myFly->pushParam();
        myFly->sliderChanged();
    }
    else
    {
        gtk_widget_hide (previewVideo);
        gtk_widget_show (previewDisabled);
    }
}

/********************************************************************/

void flySwissArmyKnife::updateConfigDescription (bool do_download)
{
    if (do_download)
        download();
    gtk_label_set_text (GTK_LABEL(WID(currentConfigDescription)),
                        ADMVideoSwissArmyKnife::getConf (&param, true));
}

/********************************************************************/

static void updateConfigDescription (GtkSpinButton * button,
                                     gpointer user_data)
{
    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (user_data);
    myFly->updateConfigDescription (true);
}

/********************************************************************/

static void previewOutputMenuChange (GtkComboBox * combo, gpointer user_data)
{
    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (user_data);
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
        flySwissArmyKnife::PreviewMode mode;
        if (index == myFly->this_filter_index)
            mode = flySwissArmyKnife::PREVIEWMODE_THIS_FILTER;
        else if (index > myFly->this_filter_index)
            mode = flySwissArmyKnife::PREVIEWMODE_LATER_FILTER;
        else // if (index < myFly->this_filter_index)
            mode = flySwissArmyKnife::PREVIEWMODE_EARLIER_FILTER;

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

    if (event->x == 1 && event->y == 1)
        return FALSE;

    flySwissArmyKnife * myFly = static_cast <flySwissArmyKnife *> (user_data);
    myFly->recomputeSize();

    gint scaled_width, scaled_height;
    gtk_widget_get_size_request (widget, &scaled_width, &scaled_height);
    GtkWidget * previewVideo = WID(previewVideo);
    gtk_widget_set_size_request (widget == previewVideo ? WID(previewDisabled)
                                 : previewVideo,
                                 scaled_width, scaled_height);

    return FALSE;
}

/********************************************************************/

uint8_t DIA_SwissArmyKnife (AVDMGenericVideoStream * in,
                            ADMVideoSwissArmyKnife * sakp,
                            SWISSARMYKNIFE_PARAM * param,
                            const MenuMapping * menu_mapping,
                            uint32_t menu_mapping_count)
{
    // Allocate space for preview video
    uint32_t width = in->getInfo()->width;
    uint32_t height = in->getInfo()->height;

    dialog = create_swiss_army_knife_dialog();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);

    gtk_register_dialog (dialog);
    gtk_window_set_title (GTK_WINDOW (dialog),
                          QT_TR_NOOP("Swiss Army Knife Configuration"));

    // Fix up a bunch of things that Glade can't do.  This is less efficient
    // than just editing the Glade output, but it's not that big a deal and
    // doing it this way makes it MUCH easier to use Glade to tweak the layout
    // later.

    // 1. Connect the slider/spinner pairs by making them share a
    //    GtkAdjustment.

#define JOIN_SPINPAIR(_basename) \
    gtk_range_set_adjustment \
        (GTK_RANGE(WID(_basename ## Slider)), \
         gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(WID(_basename ## Spinner))))

    JOIN_SPINPAIR (imageFileScalingMin);
    JOIN_SPINPAIR (imageFileScalingMax);
    JOIN_SPINPAIR (bias);
    JOIN_SPINPAIR (resultScalingMin);
    JOIN_SPINPAIR (resultScalingMax);

    // 2. Connect the alpha spin buttons.  Conveniently, they are reciprocals
    //    of each other, so we can use the same function for updates in both
    //    directions.

#define ONVALCHG(_from_widg, _func, _to_widg) \
    g_signal_connect (GTK_OBJECT(WID(_from_widg)), "value_changed", \
                      GTK_SIGNAL_FUNC(_func),                       \
                      gpointer (GTK_OBJECT(WID(_to_widg))))

    ONVALCHG (rollingAvgAlphaSpinButton, alphaRecalcOther,
              rollingAvgAlphaDenomSpinButton);
    ONVALCHG (rollingAvgAlphaDenomSpinButton, alphaRecalcOther,
              rollingAvgAlphaSpinButton);

    // 3. Create the flyDialog.

    gtk_widget_show(dialog);

    flySwissArmyKnife * myDialog
        = new flySwissArmyKnife (width, height, in,
                                 WID(previewVideo), WID(previewSlider),
                                 GTK_DIALOG(dialog), sakp, param,
                                 menu_mapping, menu_mapping_count);

    gint scaled_width, scaled_height;
    gtk_widget_get_size_request (WID(previewVideo),
                                 &scaled_width, &scaled_height);
    gtk_widget_set_size_request (WID(previewDisabled),
                                 scaled_width, scaled_height);

    g_signal_connect (GTK_OBJECT (WID(previewVideo)), "configure-event",
                      GTK_SIGNAL_FUNC (preview_video_configured),
                      gpointer (myDialog));
    g_signal_connect (GTK_OBJECT (WID(previewDisabled)), "configure-event",
                      GTK_SIGNAL_FUNC (preview_video_configured),
                      gpointer (myDialog));

    myDialog->upload();
    myDialog->sliderChanged();

    // 4. Connect up all the other signals and stuff.

#define CNX(_widg,_signame) \
    g_signal_connect (GTK_OBJECT(WID(_widg)), _signame,                \
                      GTK_SIGNAL_FUNC(gui_update), gpointer(myDialog));

    CNX (operationMenu, "changed");
    CNX (inputTypeMenu, "changed");

    CNX (convolutionInputFileChooser, "selection_changed");
    CNX (imageInputFileChooser, "selection_changed");

    CNX (constantValueSpinButton, "value_changed");
    CNX (rollingAvgAlphaDenomSpinButton, "value_changed");
    CNX (rollingAvgLookaheadSpinButton, "value_changed");
    CNX (rollingAvgInitTypeMenu, "changed");
    CNX (rollingAvgInitStartSpinButton, "value_changed");
    CNX (rollingAvgInitEndSpinButton, "value_changed");

    CNX (biasSpinner, "value_changed");

#define GETSPINADJ(_spinwidg) \
    gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (WID (_spinwidg)))

#define MK_PVSI(_basename) \
    new PixelValueScalingInfo                                \
        (GETSPINADJ(_basename ## ScalingAddSpinButton),      \
         GETSPINADJ(_basename ## ScalingMultSpinButton),     \
         GETSPINADJ(_basename ## ScalingMinSpinner),         \
         GETSPINADJ(_basename ## ScalingMaxSpinner),         \
         myDialog)

    PixelValueScalingInfo * ifsi = MK_PVSI (imageFile);
    PixelValueScalingInfo * ressi = MK_PVSI (result);

#define SCALSIG(_widg, _scalinfo) \
    g_signal_connect (GTK_OBJECT (GETSPINADJ (_widg)),             \
                      "value_changed",                             \
                      GTK_SIGNAL_FUNC(updateScalingStuff),         \
                      gpointer (_scalinfo))

#define SCALSIGS(_basename, _scalinfo) \
    do { \
        SCALSIG(_basename ## ScalingAddSpinButton, _scalinfo);        \
        SCALSIG(_basename ## ScalingMultSpinButton, _scalinfo);       \
        SCALSIG(_basename ## ScalingMinSpinner, _scalinfo);           \
        SCALSIG(_basename ## ScalingMaxSpinner, _scalinfo);           \
        updateScalingStuff                                            \
            (GETSPINADJ(_basename ## ScalingAddSpinButton),           \
             gpointer (_scalinfo));                                   \
    } while (0)

    SCALSIGS (imageFile, ifsi);
    SCALSIGS (result, ressi);

#if 0
// we now do this one manually (called from updateScalingStuff()) to avoid
// multiple screen updates (can be slow), such as for adjustments to min which
// causes propagation to some of the other fields).
#define SCALCNX(_basename) \
    do { \
        CNX (_basename ## ScalingAddSpinButton, "value_changed"); \
        CNX (_basename ## ScalingMultSpinButton, "value_changed"); \
        /* CNX (_basename ## ScalingMinSpinner, "value_changed"); */    \
        /* CNX (_basename ## ScalingMaxSpinner, "value_changed"); */    \
    } while (0)

    SCALCNX (imageFile);
    SCALCNX (result);
#endif

    // Also histogram and debugging settings, but these don't affect the
    // preview output.  However, we should update the config description when
    // they change.

    g_signal_connect (GTK_OBJECT(WID(histogramSpinButton)), "value_changed",
                      GTK_SIGNAL_FUNC(updateConfigDescription),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(debugSpinButton)), "value_changed",
                      GTK_SIGNAL_FUNC(updateConfigDescription),
                      gpointer(myDialog));

    // Reshuffle what is and isn't displayed when the user selects a different
    // input.

    g_signal_connect (GTK_OBJECT(WID(inputTypeMenu)), "changed",
                      GTK_SIGNAL_FUNC(inputTypeChange), gpointer(myDialog));
    inputTypeChange (GTK_COMBO_BOX(WID(inputTypeMenu)), gpointer(myDialog));

    // preview stuff:

    g_signal_connect (GTK_OBJECT(WID(previewSlider)), "value_changed",
                      GTK_SIGNAL_FUNC(frame_changed), gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "expose_event",
                      GTK_SIGNAL_FUNC(gui_draw), gpointer(myDialog));

    g_signal_connect (GTK_OBJECT(WID(previewEnableButton)), "toggled",
                      GTK_SIGNAL_FUNC(previewEnableButton),
                      gpointer(myDialog));
   
    previewEnableButton (GTK_TOGGLE_BUTTON(WID(previewEnableButton)),
                         gpointer(myDialog));

    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "button_press_event",
                      GTK_SIGNAL_FUNC(previewButtonEvent),
                      gpointer(myDialog));
#if 0
    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "motion_notify_event",
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

    delete ressi;
    delete ifsi;
    delete myDialog;

    return ret;
}

/********************************************************************/

void frame_changed (GtkRange *, gpointer user_data)
{
    flySwissArmyKnife * myDialog
        = static_cast <flySwissArmyKnife *> (user_data);

    myDialog->sliderChanged();
}

/********************************************************************/

void gui_update (GtkObject *, gpointer user_data)
{
    if (lock)
        return;

    flySwissArmyKnife * myDialog
        = static_cast <flySwissArmyKnife *> (user_data);
    myDialog->updateConfigDescription (true);
    myDialog->update();
}

/********************************************************************/

gboolean gui_draw (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
    flySwissArmyKnife * myDialog
        = static_cast <flySwissArmyKnife *> (user_data);
    myDialog->display();
    return TRUE;
}

/********************************************************************/

uint8_t flySwissArmyKnife::upload (void)
{
    lock++;

    gtk_file_chooser_set_filename
        (GTK_FILE_CHOOSER(WID(convolutionInputFileChooser)),
         param.input_file.c_str());

    gtk_file_chooser_set_filename
        (GTK_FILE_CHOOSER(WID(imageInputFileChooser)),
         param.input_file.c_str());
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(imageFileScalingAddSpinButton)),
         param.load_bias);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(imageFileScalingMultSpinButton)),
         param.load_multiplier);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(constantValueSpinButton)), param.input_constant);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(rollingAvgAlphaSpinButton)),
         param.memory_constant_alpha);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(rollingAvgLookaheadSpinButton)),
         param.lookahead_n_frames);
    gtk_combo_box_set_active
        (GTK_COMBO_BOX(WID(rollingAvgInitTypeMenu)), param.init_by_rolling);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(rollingAvgInitStartSpinButton)),
         param.init_start_frame);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(rollingAvgInitEndSpinButton)),
         param.init_end_frame);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(biasSpinner)), param.bias);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(resultScalingAddSpinButton)),
         param.result_bias);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(resultScalingMultSpinButton)),
         param.result_multiplier);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(histogramSpinButton)),
         param.histogram_frame_interval);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(debugSpinButton)), param.debug);

    gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON(WID(previewEnableButton)), param.enable_preview);

    updateConfigDescription (false);

    lock--;
    return 1;
}

/********************************************************************/

uint8_t flySwissArmyKnife::download (void)
{
    getMenuValues();

    const char * filename =
        gtk_file_chooser_get_filename
        (GTK_FILE_CHOOSER((param.input_type ==
                           ADMVideoSwissArmyKnife::INPUT_CUSTOM_CONVOLUTION)
                          ? WID(convolutionInputFileChooser)
                          : WID(imageInputFileChooser)));
    if (filename)
    {
        // gtk has its own allocator, and allocations from it should not be
        // intermixed with allocations from adm, lest heap corruption result!
        param.input_file = filename;
        g_free (const_cast <char *> (filename));
    }
    else
        param.input_file = "";

    param.load_bias
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(imageFileScalingAddSpinButton)));
    param.load_multiplier
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(imageFileScalingMultSpinButton)));

    param.input_constant
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(constantValueSpinButton)));

    param.memory_constant_alpha
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(rollingAvgAlphaSpinButton)));
    param.lookahead_n_frames
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(rollingAvgLookaheadSpinButton)));
    param.init_by_rolling
        = gtk_combo_box_get_active
        (GTK_COMBO_BOX(WID(rollingAvgInitTypeMenu)));
    param.init_start_frame
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(rollingAvgInitStartSpinButton)));
    param.init_end_frame
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(rollingAvgInitEndSpinButton)));

    param.bias
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(biasSpinner)));

    param.result_bias
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(resultScalingAddSpinButton)));
    param.result_multiplier
        = gtk_spin_button_get_value
        (GTK_SPIN_BUTTON(WID(resultScalingMultSpinButton)));

    param.histogram_frame_interval
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(histogramSpinButton)));
    param.debug
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(debugSpinButton)));

    param.enable_preview
        = gtk_toggle_button_get_active
        (GTK_TOGGLE_BUTTON(WID(previewEnableButton)));

    return 1;
}

/********************************************************************/

void flySwissArmyKnife::wipeOutputBuffer ()
{
    // printf ("wiping output buffer\n");
    uint32_t pixelcount = _w * _h;
    uint32_t * outp = reinterpret_cast <uint32_t *>
                      (YPLANE (_yuvBufferOut) + pixelcount);
    uint32_t intcount = pixelcount / 4 + 1;
    uint32_t value = 0x60;
    while (--intcount)
    {
        *--outp = value;
        value = value >> 8 | ((value & 0xff) << 24);
    }

    memset (UPLANE (_yuvBufferOut), 128, pixelcount >> 2);
    memset (VPLANE (_yuvBufferOut), 128, pixelcount >> 2);
}

/********************************************************************/

// The following was generated by glade from swissarmyknife.glade once upon a
// time (in fact, many times, since it took quite a few tries to get it right!
// :-).  Thus far, I've avoided editing it, so regenerating the code from
// Glade (I used 2.12.1) and swapping it in here should not lose anything.
// All the tweaking and adjusting and signal-connecting is done in code above,
// which is maintained by hand, and which has much knowledge of the names and
// layout of the widgets (so be cautious about renaming things!).

GtkWidget*
create_swiss_army_knife_dialog (void)
{
  GtkWidget *swiss_army_knife_dialog;
  GtkWidget *dialogVbox;
  GtkWidget *dialogHbox;
  GtkWidget *allSettingsVbox;
  GtkWidget *settingsOuterHbox;
  GtkWidget *settingsOuterVbox;
  GtkWidget *operationHbox;
  GtkWidget *operationLabel;
  GtkWidget *operationMenu;
  GtkWidget *inputTypeHbox;
  GtkWidget *inputTypeLabel;
  GtkWidget *inputTypeMenu;
  GtkWidget *topSeparator;
  GtkWidget *convolutionSettingsVbox;
  GtkWidget *convolutionInputFileHbox;
  GtkWidget *convolutionInputFileLabel;
  GtkWidget *convolutionInputFileChooser;
  GtkWidget *imageFileSettingsVbox;
  GtkWidget *imageInputFileHbox;
  GtkWidget *imageInputFileLabel;
  GtkWidget *imageInputFileChooser;
  GtkWidget *imageFileScalingFrame;
  GtkWidget *imageFileScalingAlignment;
  GtkWidget *imageFileScalingVbox;
  GtkWidget *imageFileScalingAddHbox;
  GtkWidget *imageFileScalingAddLabel;
  GtkObject *imageFileScalingAddSpinButton_adj;
  GtkWidget *imageFileScalingAddSpinButton;
  GtkWidget *imageFileScalingMultHbox;
  GtkWidget *imageFileScalingMultLabel;
  GtkObject *imageFileScalingMultSpinButton_adj;
  GtkWidget *imageFileScalingMultSpinButton;
  GtkWidget *imageFileScalingMinVbox;
  GtkWidget *imageFileScalingMinLabel;
  GtkWidget *imageFileScalingMinHbox;
  GtkWidget *imageFileScalingMinSlider;
  GtkObject *imageFileScalingMinSpinner_adj;
  GtkWidget *imageFileScalingMinSpinner;
  GtkWidget *imageFileScalingMaxVbox;
  GtkWidget *imageFileScalingMaxLabel;
  GtkWidget *imageFileScalingMaxHbox;
  GtkWidget *imageFileScalingMaxSlider;
  GtkObject *imageFileScalingMaxSpinner_adj;
  GtkWidget *imageFileScalingMaxSpinner;
  GtkWidget *imageFileScalingFrameLabel;
  GtkWidget *constantSettingsVbox;
  GtkWidget *constantValueHbox;
  GtkWidget *constantValueLabel;
  GtkObject *constantValueSpinButton_adj;
  GtkWidget *constantValueSpinButton;
  GtkWidget *rollingAvgSettingsVbox;
  GtkWidget *rollingAvgAlphaHbox;
  GtkWidget *rollingAvgAlphaLabel;
  GtkObject *rollingAvgAlphaSpinButton_adj;
  GtkWidget *rollingAvgAlphaSpinButton;
  GtkWidget *rollingAvgAlphaEqualsLabel;
  GtkObject *rollingAvgAlphaDenomSpinButton_adj;
  GtkWidget *rollingAvgAlphaDenomSpinButton;
  GtkWidget *rollingAvgLookaheadHbox;
  GtkWidget *rollingAvgLookaheadLabel1;
  GtkObject *rollingAvgLookaheadSpinButton_adj;
  GtkWidget *rollingAvgLookaheadSpinButton;
  GtkWidget *rollingAvgLookaheadLabel2;
  GtkWidget *rollingAvgInitVbox;
  GtkWidget *rollingAvgInitHbox1;
  GtkWidget *rollingAvgInitLabel1;
  GtkWidget *rollingAvgInitTypeMenu;
  GtkWidget *rollingAvgInitLabel2;
  GtkWidget *rollingAvgInitHbox2;
  GtkWidget *rollingAvgInitLabel3;
  GtkObject *rollingAvgInitStartSpinButton_adj;
  GtkWidget *rollingAvgInitStartSpinButton;
  GtkWidget *rollingAvgInitLabel4;
  GtkObject *rollingAvgInitEndSpinButton_adj;
  GtkWidget *rollingAvgInitEndSpinButton;
  GtkWidget *bottomSeparator;
  GtkWidget *biasVbox;
  GtkWidget *biasLabel;
  GtkWidget *biasHbox;
  GtkWidget *biasSlider;
  GtkObject *biasSpinner_adj;
  GtkWidget *biasSpinner;
  GtkWidget *resultScalingFrame;
  GtkWidget *resultScalingAlignment;
  GtkWidget *resultScalingVbox;
  GtkWidget *resultScalingAddHbox;
  GtkWidget *resultScalingAddLabel;
  GtkObject *resultScalingAddSpinButton_adj;
  GtkWidget *resultScalingAddSpinButton;
  GtkWidget *resultScalingMultHbox;
  GtkWidget *resultScalingMultLabel;
  GtkObject *resultScalingMultSpinButton_adj;
  GtkWidget *resultScalingMultSpinButton;
  GtkWidget *resultScalingMinVbox;
  GtkWidget *resultScalingMinLabel;
  GtkWidget *resultScalingMinHbox;
  GtkWidget *resultScalingMinSlider;
  GtkObject *resultScalingMinSpinner_adj;
  GtkWidget *resultScalingMinSpinner;
  GtkWidget *resultScalingMaxVbox;
  GtkWidget *resultScalingMaxLabel;
  GtkWidget *resultScalingMaxHbox;
  GtkWidget *resultScalingMaxSlider;
  GtkObject *resultScalingMaxSpinner_adj;
  GtkWidget *resultScalingMaxSpinner;
  GtkWidget *resultScalingFrameLabel;
  GtkWidget *hseparator2;
  GtkWidget *currentConfigDescription;
  GtkWidget *hseparator1;
  GtkWidget *histogramHbox;
  GtkWidget *histogramLabel;
  GtkObject *histogramSpinButton_adj;
  GtkWidget *histogramSpinButton;
  GtkWidget *debugHbox;
  GtkWidget *debugLabel;
  GtkObject *debugSpinButton_adj;
  GtkWidget *debugSpinButton;
  GtkWidget *previewVboxOuter;
  GtkWidget *previewFrame;
  GtkWidget *previewAlignment;
  GtkWidget *previewVbox;
  GtkWidget *previewControlHbox;
  GtkWidget *previewOutputMenu;
  GtkWidget *previewSlider;
  GtkWidget *previewVideo;
  GtkWidget *previewDisabled;
  GtkWidget *previewLabel;
  GtkWidget *previewEnableButtonHbox;
  GtkWidget *previewEnableButton;
  GtkWidget *dialogButtonBox;
  GtkWidget *cancelButton;
  GtkWidget *okButton;

  swiss_army_knife_dialog = gtk_dialog_new ();
  //                                                NO NEED TO "FIX" THAT _("...")!!
  //                                                see handy macro near top of file.
  gtk_window_set_title (GTK_WINDOW (swiss_army_knife_dialog), _("Swiss Army Knife"));
  gtk_window_set_type_hint (GTK_WINDOW (swiss_army_knife_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialogVbox = GTK_DIALOG (swiss_army_knife_dialog)->vbox;
  gtk_widget_show (dialogVbox);

  dialogHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (dialogHbox);
  gtk_box_pack_start (GTK_BOX (dialogVbox), dialogHbox, TRUE, TRUE, 0);

  allSettingsVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (allSettingsVbox);
  gtk_box_pack_start (GTK_BOX (dialogHbox), allSettingsVbox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (allSettingsVbox), 8);

  settingsOuterHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (settingsOuterHbox);
  gtk_box_pack_start (GTK_BOX (allSettingsVbox), settingsOuterHbox, FALSE, TRUE, 0);

  settingsOuterVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (settingsOuterVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterHbox), settingsOuterVbox, TRUE, TRUE, 0);

  operationHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (operationHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), operationHbox, TRUE, TRUE, 0);

  operationLabel = gtk_label_new_with_mnemonic (_("Select _Operation on each pixel P and input A:   "));
  gtk_widget_show (operationLabel);
  gtk_box_pack_start (GTK_BOX (operationHbox), operationLabel, FALSE, FALSE, 0);

  operationMenu = gtk_combo_box_new_text ();
  gtk_widget_show (operationMenu);
  gtk_box_pack_start (GTK_BOX (operationHbox), operationMenu, TRUE, TRUE, 0);

  inputTypeHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (inputTypeHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), inputTypeHbox, TRUE, TRUE, 0);

  inputTypeLabel = gtk_label_new_with_mnemonic (_("Input _Type:   "));
  gtk_widget_show (inputTypeLabel);
  gtk_box_pack_start (GTK_BOX (inputTypeHbox), inputTypeLabel, FALSE, FALSE, 0);

  inputTypeMenu = gtk_combo_box_new_text ();
  gtk_widget_show (inputTypeMenu);
  gtk_box_pack_start (GTK_BOX (inputTypeHbox), inputTypeMenu, TRUE, TRUE, 0);

  topSeparator = gtk_hseparator_new ();
  gtk_widget_show (topSeparator);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), topSeparator, TRUE, TRUE, 0);

  convolutionSettingsVbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), convolutionSettingsVbox, TRUE, TRUE, 0);

  convolutionInputFileHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (convolutionInputFileHbox);
  gtk_box_pack_start (GTK_BOX (convolutionSettingsVbox), convolutionInputFileHbox, TRUE, TRUE, 0);

  convolutionInputFileLabel = gtk_label_new_with_mnemonic (_("Input _File (convolution kernel):   "));
  gtk_widget_show (convolutionInputFileLabel);
  gtk_box_pack_start (GTK_BOX (convolutionInputFileHbox), convolutionInputFileLabel, FALSE, FALSE, 0);

  convolutionInputFileChooser = gtk_file_chooser_button_new (_("Select Convolution Kernel File"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_show (convolutionInputFileChooser);
  gtk_box_pack_start (GTK_BOX (convolutionInputFileHbox), convolutionInputFileChooser, TRUE, TRUE, 0);

  imageFileSettingsVbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), imageFileSettingsVbox, TRUE, TRUE, 0);

  imageInputFileHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (imageInputFileHbox);
  gtk_box_pack_start (GTK_BOX (imageFileSettingsVbox), imageInputFileHbox, TRUE, TRUE, 0);

  imageInputFileLabel = gtk_label_new_with_mnemonic (_("Input _File (image file):   "));
  gtk_widget_show (imageInputFileLabel);
  gtk_box_pack_start (GTK_BOX (imageInputFileHbox), imageInputFileLabel, FALSE, FALSE, 0);

  imageInputFileChooser = gtk_file_chooser_button_new (_("Select Convolution Kernel File"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_show (imageInputFileChooser);
  gtk_box_pack_start (GTK_BOX (imageInputFileHbox), imageInputFileChooser, TRUE, TRUE, 0);

  imageFileScalingFrame = gtk_frame_new (NULL);
  gtk_widget_show (imageFileScalingFrame);
  gtk_box_pack_start (GTK_BOX (imageFileSettingsVbox), imageFileScalingFrame, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (imageFileScalingFrame), GTK_SHADOW_IN);

  imageFileScalingAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (imageFileScalingAlignment);
  gtk_container_add (GTK_CONTAINER (imageFileScalingFrame), imageFileScalingAlignment);
  gtk_alignment_set_padding (GTK_ALIGNMENT (imageFileScalingAlignment), 2, 6, 6, 6);

  imageFileScalingVbox = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (imageFileScalingVbox);
  gtk_container_add (GTK_CONTAINER (imageFileScalingAlignment), imageFileScalingVbox);

  imageFileScalingAddHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (imageFileScalingAddHbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingVbox), imageFileScalingAddHbox, TRUE, TRUE, 0);

  imageFileScalingAddLabel = gtk_label_new_with_mnemonic (_("First, _add to each float pixel value:   "));
  gtk_widget_show (imageFileScalingAddLabel);
  gtk_box_pack_start (GTK_BOX (imageFileScalingAddHbox), imageFileScalingAddLabel, FALSE, FALSE, 0);

  imageFileScalingAddSpinButton_adj = gtk_adjustment_new (0, -99999, 99999, 0.10000000149, 1, 0);
  imageFileScalingAddSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (imageFileScalingAddSpinButton_adj), 1, 5);
  gtk_widget_show (imageFileScalingAddSpinButton);
  gtk_box_pack_start (GTK_BOX (imageFileScalingAddHbox), imageFileScalingAddSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (imageFileScalingAddSpinButton), TRUE);

  imageFileScalingMultHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (imageFileScalingMultHbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingVbox), imageFileScalingMultHbox, TRUE, TRUE, 0);

  imageFileScalingMultLabel = gtk_label_new_with_mnemonic (_("Then, _multiply each float pixel value by:   "));
  gtk_widget_show (imageFileScalingMultLabel);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMultHbox), imageFileScalingMultLabel, FALSE, FALSE, 0);

  imageFileScalingMultSpinButton_adj = gtk_adjustment_new (1, -99999, 99999, 0.0010000000475, 0.0010000000475, 0);
  imageFileScalingMultSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (imageFileScalingMultSpinButton_adj), 1, 5);
  gtk_widget_show (imageFileScalingMultSpinButton);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMultHbox), imageFileScalingMultSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (imageFileScalingMultSpinButton), TRUE);

  imageFileScalingMinVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (imageFileScalingMinVbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingVbox), imageFileScalingMinVbox, TRUE, TRUE, 0);

  imageFileScalingMinLabel = gtk_label_new_with_mnemonic (_("Mi_nimum loaded pixel value (will be scaled to 0):"));
  gtk_widget_show (imageFileScalingMinLabel);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMinVbox), imageFileScalingMinLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (imageFileScalingMinLabel), 0, 0.5);

  imageFileScalingMinHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (imageFileScalingMinHbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMinVbox), imageFileScalingMinHbox, TRUE, TRUE, 0);

  imageFileScalingMinSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -1024, 1024, 1, 1, 0)));
  gtk_widget_show (imageFileScalingMinSlider);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMinHbox), imageFileScalingMinSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (imageFileScalingMinSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (imageFileScalingMinSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (imageFileScalingMinSlider), 0);

  imageFileScalingMinSpinner_adj = gtk_adjustment_new (0, -1024, 1024, 1, 2, 0);
  imageFileScalingMinSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (imageFileScalingMinSpinner_adj), 1, 1);
  gtk_widget_show (imageFileScalingMinSpinner);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMinHbox), imageFileScalingMinSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (imageFileScalingMinSpinner), TRUE);

  imageFileScalingMaxVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (imageFileScalingMaxVbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingVbox), imageFileScalingMaxVbox, TRUE, TRUE, 0);

  imageFileScalingMaxLabel = gtk_label_new_with_mnemonic (_("Ma_ximum loaded pixel value (will be scaled to 255):"));
  gtk_widget_show (imageFileScalingMaxLabel);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMaxVbox), imageFileScalingMaxLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (imageFileScalingMaxLabel), 0, 0.5);

  imageFileScalingMaxHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (imageFileScalingMaxHbox);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMaxVbox), imageFileScalingMaxHbox, TRUE, TRUE, 0);

  imageFileScalingMaxSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (255, -1024, 1024, 1, 1, 0)));
  gtk_widget_show (imageFileScalingMaxSlider);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMaxHbox), imageFileScalingMaxSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (imageFileScalingMaxSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (imageFileScalingMaxSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (imageFileScalingMaxSlider), 0);

  imageFileScalingMaxSpinner_adj = gtk_adjustment_new (255, -1024, 1024, 1, 2, 0);
  imageFileScalingMaxSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (imageFileScalingMaxSpinner_adj), 1, 1);
  gtk_widget_show (imageFileScalingMaxSpinner);
  gtk_box_pack_start (GTK_BOX (imageFileScalingMaxHbox), imageFileScalingMaxSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (imageFileScalingMaxSpinner), TRUE);

  imageFileScalingFrameLabel = gtk_label_new (_("Pixel Value Scaling as Image File is Loaded"));
  gtk_widget_show (imageFileScalingFrameLabel);
  gtk_frame_set_label_widget (GTK_FRAME (imageFileScalingFrame), imageFileScalingFrameLabel);
  gtk_label_set_use_markup (GTK_LABEL (imageFileScalingFrameLabel), TRUE);

  constantSettingsVbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), constantSettingsVbox, TRUE, TRUE, 0);

  constantValueHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (constantValueHbox);
  gtk_box_pack_start (GTK_BOX (constantSettingsVbox), constantValueHbox, TRUE, TRUE, 0);

  constantValueLabel = gtk_label_new_with_mnemonic (_("Input _Constant:      "));
  gtk_widget_show (constantValueLabel);
  gtk_box_pack_start (GTK_BOX (constantValueHbox), constantValueLabel, FALSE, FALSE, 0);

  constantValueSpinButton_adj = gtk_adjustment_new (0, -99999, 99999, 0.10000000149, 1, 0);
  constantValueSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (constantValueSpinButton_adj), 1, 3);
  gtk_widget_show (constantValueSpinButton);
  gtk_box_pack_start (GTK_BOX (constantValueHbox), constantValueSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (constantValueSpinButton), TRUE);

  rollingAvgSettingsVbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), rollingAvgSettingsVbox, TRUE, TRUE, 0);

  rollingAvgAlphaHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (rollingAvgAlphaHbox);
  gtk_box_pack_start (GTK_BOX (rollingAvgSettingsVbox), rollingAvgAlphaHbox, TRUE, TRUE, 0);

  rollingAvgAlphaLabel = gtk_label_new_with_mnemonic (_("Memory constant _alpha:   "));
  gtk_widget_show (rollingAvgAlphaLabel);
  gtk_box_pack_start (GTK_BOX (rollingAvgAlphaHbox), rollingAvgAlphaLabel, FALSE, TRUE, 0);

  rollingAvgAlphaSpinButton_adj = gtk_adjustment_new (0.00999999977648, 9.99999996004e-12, 999, 0.0010000000475, 0.0010000000475, 0);
  rollingAvgAlphaSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rollingAvgAlphaSpinButton_adj), 1, 5);
  gtk_widget_show (rollingAvgAlphaSpinButton);
  gtk_box_pack_start (GTK_BOX (rollingAvgAlphaHbox), rollingAvgAlphaSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rollingAvgAlphaSpinButton), TRUE);

  rollingAvgAlphaEqualsLabel = gtk_label_new (_(" = 1 / "));
  gtk_widget_show (rollingAvgAlphaEqualsLabel);
  gtk_box_pack_start (GTK_BOX (rollingAvgAlphaHbox), rollingAvgAlphaEqualsLabel, FALSE, FALSE, 0);

  rollingAvgAlphaDenomSpinButton_adj = gtk_adjustment_new (100, 0.0010000000475, 99999, 1, 1, 0);
  rollingAvgAlphaDenomSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rollingAvgAlphaDenomSpinButton_adj), 1, 4);
  gtk_widget_show (rollingAvgAlphaDenomSpinButton);
  gtk_box_pack_start (GTK_BOX (rollingAvgAlphaHbox), rollingAvgAlphaDenomSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rollingAvgAlphaDenomSpinButton), TRUE);

  rollingAvgLookaheadHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (rollingAvgLookaheadHbox);
  gtk_box_pack_start (GTK_BOX (rollingAvgSettingsVbox), rollingAvgLookaheadHbox, TRUE, TRUE, 0);

  rollingAvgLookaheadLabel1 = gtk_label_new_with_mnemonic (_("_Look ahead "));
  gtk_widget_show (rollingAvgLookaheadLabel1);
  gtk_box_pack_start (GTK_BOX (rollingAvgLookaheadHbox), rollingAvgLookaheadLabel1, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (rollingAvgLookaheadLabel1), GTK_JUSTIFY_RIGHT);

  rollingAvgLookaheadSpinButton_adj = gtk_adjustment_new (0, 0, 99999, 1, 1, 0);
  rollingAvgLookaheadSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rollingAvgLookaheadSpinButton_adj), 1, 0);
  gtk_widget_show (rollingAvgLookaheadSpinButton);
  gtk_box_pack_start (GTK_BOX (rollingAvgLookaheadHbox), rollingAvgLookaheadSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rollingAvgLookaheadSpinButton), TRUE);

  rollingAvgLookaheadLabel2 = gtk_label_new_with_mnemonic (_(" frames.  (Anything > 0 will be slower.)"));
  gtk_widget_show (rollingAvgLookaheadLabel2);
  gtk_box_pack_start (GTK_BOX (rollingAvgLookaheadHbox), rollingAvgLookaheadLabel2, FALSE, FALSE, 0);

  rollingAvgInitVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (rollingAvgInitVbox);
  gtk_box_pack_start (GTK_BOX (rollingAvgSettingsVbox), rollingAvgInitVbox, TRUE, TRUE, 0);

  rollingAvgInitHbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (rollingAvgInitHbox1);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitVbox), rollingAvgInitHbox1, TRUE, TRUE, 0);

  rollingAvgInitLabel1 = gtk_label_new_with_mnemonic (_("_Initial average (\"head start\") is "));
  gtk_widget_show (rollingAvgInitLabel1);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox1), rollingAvgInitLabel1, FALSE, FALSE, 0);

  rollingAvgInitTypeMenu = gtk_combo_box_new_text ();
  gtk_widget_show (rollingAvgInitTypeMenu);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox1), rollingAvgInitTypeMenu, FALSE, TRUE, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (rollingAvgInitTypeMenu), _("straight"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (rollingAvgInitTypeMenu), _("rolling"));

  rollingAvgInitLabel2 = gtk_label_new (_(" average"));
  gtk_widget_show (rollingAvgInitLabel2);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox1), rollingAvgInitLabel2, FALSE, FALSE, 0);

  rollingAvgInitHbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (rollingAvgInitHbox2);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitVbox), rollingAvgInitHbox2, TRUE, TRUE, 0);

  rollingAvgInitLabel3 = gtk_label_new (_("      of frames "));
  gtk_widget_show (rollingAvgInitLabel3);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox2), rollingAvgInitLabel3, FALSE, FALSE, 0);

  rollingAvgInitStartSpinButton_adj = gtk_adjustment_new (0, 0, 9999999, 1, 1, 0);
  rollingAvgInitStartSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rollingAvgInitStartSpinButton_adj), 1, 0);
  gtk_widget_show (rollingAvgInitStartSpinButton);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox2), rollingAvgInitStartSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rollingAvgInitStartSpinButton), TRUE);

  rollingAvgInitLabel4 = gtk_label_new (_(" through "));
  gtk_widget_show (rollingAvgInitLabel4);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox2), rollingAvgInitLabel4, FALSE, FALSE, 0);

  rollingAvgInitEndSpinButton_adj = gtk_adjustment_new (99, 0, 9999999, 1, 1, 0);
  rollingAvgInitEndSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rollingAvgInitEndSpinButton_adj), 1, 0);
  gtk_widget_show (rollingAvgInitEndSpinButton);
  gtk_box_pack_start (GTK_BOX (rollingAvgInitHbox2), rollingAvgInitEndSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rollingAvgInitEndSpinButton), TRUE);

  bottomSeparator = gtk_hseparator_new ();
  gtk_widget_show (bottomSeparator);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), bottomSeparator, TRUE, TRUE, 0);

  biasVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (biasVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), biasVbox, TRUE, TRUE, 0);

  biasLabel = gtk_label_new_with_mnemonic (_("Integer _Bias (will be added to result of selected operation):"));
  gtk_widget_show (biasLabel);
  gtk_box_pack_start (GTK_BOX (biasVbox), biasLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (biasLabel), 0, 0.5);

  biasHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (biasHbox);
  gtk_box_pack_start (GTK_BOX (biasVbox), biasHbox, TRUE, TRUE, 0);

  biasSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (128, -256, 256, 1, 1, 0)));
  gtk_widget_show (biasSlider);
  gtk_box_pack_start (GTK_BOX (biasHbox), biasSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (biasSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (biasSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (biasSlider), 0);

  biasSpinner_adj = gtk_adjustment_new (128, -256, 256, 1, 1, 0);
  biasSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (biasSpinner_adj), 1, 0);
  gtk_widget_show (biasSpinner);
  gtk_box_pack_start (GTK_BOX (biasHbox), biasSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (biasSpinner), TRUE);

  resultScalingFrame = gtk_frame_new (NULL);
  gtk_widget_show (resultScalingFrame);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), resultScalingFrame, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (resultScalingFrame), GTK_SHADOW_IN);

  resultScalingAlignment = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (resultScalingAlignment);
  gtk_container_add (GTK_CONTAINER (resultScalingFrame), resultScalingAlignment);
  gtk_alignment_set_padding (GTK_ALIGNMENT (resultScalingAlignment), 2, 6, 6, 6);

  resultScalingVbox = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (resultScalingVbox);
  gtk_container_add (GTK_CONTAINER (resultScalingAlignment), resultScalingVbox);

  resultScalingAddHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (resultScalingAddHbox);
  gtk_box_pack_start (GTK_BOX (resultScalingVbox), resultScalingAddHbox, TRUE, TRUE, 0);

  resultScalingAddLabel = gtk_label_new_with_mnemonic (_("First, _add to each pixel result value:   "));
  gtk_widget_show (resultScalingAddLabel);
  gtk_box_pack_start (GTK_BOX (resultScalingAddHbox), resultScalingAddLabel, FALSE, FALSE, 0);

  resultScalingAddSpinButton_adj = gtk_adjustment_new (0, -99999, 99999, 0.10000000149, 1, 0);
  resultScalingAddSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (resultScalingAddSpinButton_adj), 1, 5);
  gtk_widget_show (resultScalingAddSpinButton);
  gtk_box_pack_start (GTK_BOX (resultScalingAddHbox), resultScalingAddSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (resultScalingAddSpinButton), TRUE);

  resultScalingMultHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (resultScalingMultHbox);
  gtk_box_pack_start (GTK_BOX (resultScalingVbox), resultScalingMultHbox, TRUE, TRUE, 0);

  resultScalingMultLabel = gtk_label_new_with_mnemonic (_("Then, _multiply each pixel result value by:   "));
  gtk_widget_show (resultScalingMultLabel);
  gtk_box_pack_start (GTK_BOX (resultScalingMultHbox), resultScalingMultLabel, FALSE, FALSE, 0);

  resultScalingMultSpinButton_adj = gtk_adjustment_new (1, -99999, 99999, 0.0010000000475, 0.0010000000475, 0);
  resultScalingMultSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (resultScalingMultSpinButton_adj), 1, 5);
  gtk_widget_show (resultScalingMultSpinButton);
  gtk_box_pack_start (GTK_BOX (resultScalingMultHbox), resultScalingMultSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (resultScalingMultSpinButton), TRUE);

  resultScalingMinVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (resultScalingMinVbox);
  gtk_box_pack_start (GTK_BOX (resultScalingVbox), resultScalingMinVbox, TRUE, TRUE, 0);

  resultScalingMinLabel = gtk_label_new_with_mnemonic (_("Mi_nimum result value (will be scaled to 0):"));
  gtk_widget_show (resultScalingMinLabel);
  gtk_box_pack_start (GTK_BOX (resultScalingMinVbox), resultScalingMinLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (resultScalingMinLabel), 0, 0.5);

  resultScalingMinHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (resultScalingMinHbox);
  gtk_box_pack_start (GTK_BOX (resultScalingMinVbox), resultScalingMinHbox, TRUE, TRUE, 0);

  resultScalingMinSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, -1024, 1024, 1, 1, 0)));
  gtk_widget_show (resultScalingMinSlider);
  gtk_box_pack_start (GTK_BOX (resultScalingMinHbox), resultScalingMinSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (resultScalingMinSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (resultScalingMinSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (resultScalingMinSlider), 0);

  resultScalingMinSpinner_adj = gtk_adjustment_new (0, -1024, 1024, 1, 2, 0);
  resultScalingMinSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (resultScalingMinSpinner_adj), 1, 1);
  gtk_widget_show (resultScalingMinSpinner);
  gtk_box_pack_start (GTK_BOX (resultScalingMinHbox), resultScalingMinSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (resultScalingMinSpinner), TRUE);

  resultScalingMaxVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (resultScalingMaxVbox);
  gtk_box_pack_start (GTK_BOX (resultScalingVbox), resultScalingMaxVbox, TRUE, TRUE, 0);

  resultScalingMaxLabel = gtk_label_new_with_mnemonic (_("Ma_ximum result value (will be scaled to 255):"));
  gtk_widget_show (resultScalingMaxLabel);
  gtk_box_pack_start (GTK_BOX (resultScalingMaxVbox), resultScalingMaxLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (resultScalingMaxLabel), 0, 0.5);

  resultScalingMaxHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (resultScalingMaxHbox);
  gtk_box_pack_start (GTK_BOX (resultScalingMaxVbox), resultScalingMaxHbox, TRUE, TRUE, 0);

  resultScalingMaxSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (255, -1024, 1024, 1, 1, 0)));
  gtk_widget_show (resultScalingMaxSlider);
  gtk_box_pack_start (GTK_BOX (resultScalingMaxHbox), resultScalingMaxSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (resultScalingMaxSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (resultScalingMaxSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (resultScalingMaxSlider), 0);

  resultScalingMaxSpinner_adj = gtk_adjustment_new (255, -1024, 1024, 1, 2, 0);
  resultScalingMaxSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (resultScalingMaxSpinner_adj), 1, 1);
  gtk_widget_show (resultScalingMaxSpinner);
  gtk_box_pack_start (GTK_BOX (resultScalingMaxHbox), resultScalingMaxSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (resultScalingMaxSpinner), TRUE);

  resultScalingFrameLabel = gtk_label_new (_("Pixel Value Scaling on Result of Selected Operation"));
  gtk_widget_show (resultScalingFrameLabel);
  gtk_frame_set_label_widget (GTK_FRAME (resultScalingFrame), resultScalingFrameLabel);
  gtk_label_set_use_markup (GTK_LABEL (resultScalingFrameLabel), TRUE);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_show (hseparator2);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), hseparator2, TRUE, TRUE, 0);

  currentConfigDescription = gtk_label_new (_("(description of current config will go here)"));
  gtk_widget_show (currentConfigDescription);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), currentConfigDescription, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (currentConfigDescription, GTK_CAN_FOCUS);
  gtk_label_set_use_markup (GTK_LABEL (currentConfigDescription), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (currentConfigDescription), TRUE);
  gtk_label_set_selectable (GTK_LABEL (currentConfigDescription), TRUE);
  gtk_misc_set_alignment (GTK_MISC (currentConfigDescription), 0, 0.5);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), hseparator1, TRUE, TRUE, 0);

  histogramHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (histogramHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), histogramHbox, TRUE, TRUE, 0);

  histogramLabel = gtk_label_new_with_mnemonic (_("_Histogram every N frames (0 to disable):   "));
  gtk_widget_show (histogramLabel);
  gtk_box_pack_start (GTK_BOX (histogramHbox), histogramLabel, FALSE, FALSE, 0);

  histogramSpinButton_adj = gtk_adjustment_new (0, 0, 16777215, 1, 10, 0);
  histogramSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (histogramSpinButton_adj), 1, 0);
  gtk_widget_show (histogramSpinButton);
  gtk_box_pack_start (GTK_BOX (histogramHbox), histogramSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (histogramSpinButton), TRUE);

  debugHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (debugHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), debugHbox, TRUE, TRUE, 0);

  debugLabel = gtk_label_new_with_mnemonic (_("_Debugging settings (bits):   "));
  gtk_widget_show (debugLabel);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugLabel, FALSE, FALSE, 0);

  debugSpinButton_adj = gtk_adjustment_new (0, 0, 16777215, 1, 10, 0);
  debugSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (debugSpinButton_adj), 1, 0);
  gtk_widget_show (debugSpinButton);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (debugSpinButton), TRUE);

  previewVboxOuter = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (previewVboxOuter);
  gtk_box_pack_start (GTK_BOX (dialogHbox), previewVboxOuter, FALSE, TRUE, 0);

  previewFrame = gtk_frame_new (NULL);
  gtk_widget_show (previewFrame);
  gtk_box_pack_start (GTK_BOX (previewVboxOuter), previewFrame, FALSE, TRUE, 0);

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

  previewSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 99, 1, 1, 0)));
  gtk_widget_show (previewSlider);
  gtk_box_pack_start (GTK_BOX (previewControlHbox), previewSlider, TRUE, TRUE, 0);
  gtk_scale_set_digits (GTK_SCALE (previewSlider), 0);

  previewVideo = gtk_drawing_area_new ();
  gtk_widget_show (previewVideo);
  gtk_box_pack_start (GTK_BOX (previewVbox), previewVideo, TRUE, TRUE, 0);
  gtk_widget_set_size_request (previewVideo, 30, 30);
  gtk_widget_set_events (previewVideo, GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK);

  previewDisabled = gtk_drawing_area_new ();
  gtk_box_pack_start (GTK_BOX (previewVbox), previewDisabled, TRUE, TRUE, 0);

  previewLabel = gtk_label_new (_("Preview"));
  gtk_widget_show (previewLabel);
  gtk_frame_set_label_widget (GTK_FRAME (previewFrame), previewLabel);

  previewEnableButtonHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (previewEnableButtonHbox);
  gtk_box_pack_start (GTK_BOX (previewVboxOuter), previewEnableButtonHbox, FALSE, TRUE, 5);

  previewEnableButton = gtk_toggle_button_new_with_mnemonic (_("Enable Preview"));
  gtk_widget_show (previewEnableButton);
  gtk_box_pack_end (GTK_BOX (previewEnableButtonHbox), previewEnableButton, FALSE, FALSE, 0);

  dialogButtonBox = GTK_DIALOG (swiss_army_knife_dialog)->action_area;
  gtk_widget_show (dialogButtonBox);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialogButtonBox), GTK_BUTTONBOX_END);

  cancelButton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (swiss_army_knife_dialog), cancelButton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelButton, GTK_CAN_DEFAULT);

  okButton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (swiss_army_knife_dialog), okButton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okButton, GTK_CAN_DEFAULT);

  gtk_label_set_mnemonic_widget (GTK_LABEL (imageFileScalingAddLabel), imageFileScalingAddSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (imageFileScalingMultLabel), imageFileScalingMultSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (constantValueLabel), constantValueSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (rollingAvgAlphaLabel), rollingAvgAlphaSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (rollingAvgLookaheadLabel1), rollingAvgLookaheadSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (rollingAvgLookaheadLabel2), rollingAvgLookaheadSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (resultScalingAddLabel), resultScalingAddSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (resultScalingMultLabel), resultScalingMultSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (histogramLabel), histogramSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (debugLabel), debugSpinButton);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (swiss_army_knife_dialog, swiss_army_knife_dialog, "swiss_army_knife_dialog");
  GLADE_HOOKUP_OBJECT_NO_REF (swiss_army_knife_dialog, dialogVbox, "dialogVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, dialogHbox, "dialogHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, allSettingsVbox, "allSettingsVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, settingsOuterHbox, "settingsOuterHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, settingsOuterVbox, "settingsOuterVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, operationHbox, "operationHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, operationLabel, "operationLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, operationMenu, "operationMenu");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, inputTypeHbox, "inputTypeHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, inputTypeLabel, "inputTypeLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, inputTypeMenu, "inputTypeMenu");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, topSeparator, "topSeparator");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, convolutionSettingsVbox, "convolutionSettingsVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, convolutionInputFileHbox, "convolutionInputFileHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, convolutionInputFileLabel, "convolutionInputFileLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, convolutionInputFileChooser, "convolutionInputFileChooser");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileSettingsVbox, "imageFileSettingsVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageInputFileHbox, "imageInputFileHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageInputFileLabel, "imageInputFileLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageInputFileChooser, "imageInputFileChooser");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingFrame, "imageFileScalingFrame");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingAlignment, "imageFileScalingAlignment");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingVbox, "imageFileScalingVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingAddHbox, "imageFileScalingAddHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingAddLabel, "imageFileScalingAddLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingAddSpinButton, "imageFileScalingAddSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMultHbox, "imageFileScalingMultHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMultLabel, "imageFileScalingMultLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMultSpinButton, "imageFileScalingMultSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMinVbox, "imageFileScalingMinVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMinLabel, "imageFileScalingMinLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMinHbox, "imageFileScalingMinHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMinSlider, "imageFileScalingMinSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMinSpinner, "imageFileScalingMinSpinner");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMaxVbox, "imageFileScalingMaxVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMaxLabel, "imageFileScalingMaxLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMaxHbox, "imageFileScalingMaxHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMaxSlider, "imageFileScalingMaxSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingMaxSpinner, "imageFileScalingMaxSpinner");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, imageFileScalingFrameLabel, "imageFileScalingFrameLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, constantSettingsVbox, "constantSettingsVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, constantValueHbox, "constantValueHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, constantValueLabel, "constantValueLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, constantValueSpinButton, "constantValueSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgSettingsVbox, "rollingAvgSettingsVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgAlphaHbox, "rollingAvgAlphaHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgAlphaLabel, "rollingAvgAlphaLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgAlphaSpinButton, "rollingAvgAlphaSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgAlphaEqualsLabel, "rollingAvgAlphaEqualsLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgAlphaDenomSpinButton, "rollingAvgAlphaDenomSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgLookaheadHbox, "rollingAvgLookaheadHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgLookaheadLabel1, "rollingAvgLookaheadLabel1");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgLookaheadSpinButton, "rollingAvgLookaheadSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgLookaheadLabel2, "rollingAvgLookaheadLabel2");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitVbox, "rollingAvgInitVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitHbox1, "rollingAvgInitHbox1");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitLabel1, "rollingAvgInitLabel1");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitTypeMenu, "rollingAvgInitTypeMenu");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitLabel2, "rollingAvgInitLabel2");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitHbox2, "rollingAvgInitHbox2");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitLabel3, "rollingAvgInitLabel3");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitStartSpinButton, "rollingAvgInitStartSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitLabel4, "rollingAvgInitLabel4");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, rollingAvgInitEndSpinButton, "rollingAvgInitEndSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, bottomSeparator, "bottomSeparator");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, biasVbox, "biasVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, biasLabel, "biasLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, biasHbox, "biasHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, biasSlider, "biasSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, biasSpinner, "biasSpinner");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingFrame, "resultScalingFrame");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingAlignment, "resultScalingAlignment");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingVbox, "resultScalingVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingAddHbox, "resultScalingAddHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingAddLabel, "resultScalingAddLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingAddSpinButton, "resultScalingAddSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMultHbox, "resultScalingMultHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMultLabel, "resultScalingMultLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMultSpinButton, "resultScalingMultSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMinVbox, "resultScalingMinVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMinLabel, "resultScalingMinLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMinHbox, "resultScalingMinHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMinSlider, "resultScalingMinSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMinSpinner, "resultScalingMinSpinner");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMaxVbox, "resultScalingMaxVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMaxLabel, "resultScalingMaxLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMaxHbox, "resultScalingMaxHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMaxSlider, "resultScalingMaxSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingMaxSpinner, "resultScalingMaxSpinner");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, resultScalingFrameLabel, "resultScalingFrameLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, hseparator2, "hseparator2");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, currentConfigDescription, "currentConfigDescription");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, hseparator1, "hseparator1");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, histogramHbox, "histogramHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, histogramLabel, "histogramLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, histogramSpinButton, "histogramSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, debugHbox, "debugHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, debugLabel, "debugLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, debugSpinButton, "debugSpinButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewVboxOuter, "previewVboxOuter");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewFrame, "previewFrame");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewAlignment, "previewAlignment");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewVbox, "previewVbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewControlHbox, "previewControlHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewOutputMenu, "previewOutputMenu");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewSlider, "previewSlider");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewVideo, "previewVideo");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewDisabled, "previewDisabled");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewLabel, "previewLabel");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewEnableButtonHbox, "previewEnableButtonHbox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, previewEnableButton, "previewEnableButton");
  GLADE_HOOKUP_OBJECT_NO_REF (swiss_army_knife_dialog, dialogButtonBox, "dialogButtonBox");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, cancelButton, "cancelButton");
  GLADE_HOOKUP_OBJECT (swiss_army_knife_dialog, okButton, "okButton");

  return swiss_army_knife_dialog;
}

#endif

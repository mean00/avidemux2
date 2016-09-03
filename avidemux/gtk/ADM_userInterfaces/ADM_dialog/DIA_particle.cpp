/***************************************************************************
                          DIA_particle.cpp  -  configuration dialog for
						particle filter
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
#include "fourcc.h"

#include "prefs.h"

#include "avi_vars.h"

#ifdef HAVE_ENCODER

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_videoFilter/ADM_vidParticle.h"

#include "DIA_fileSel.h"

#include "DIA_flyDialog.h"
#include "DIA_flyParticle.h"
#include "DIA_factory.h"


#undef _
#define _(_s) QT_TR_NOOP(_s)

/********************************************************************/
static GtkWidget *create_particle_dialog (void);
static GtkWidget *dialog = 0;

static volatile int lock = 0;

/********************************************************************/

static gboolean previewButtonEvent (GtkWidget *,
                                    GdkEventButton * event,
                                    gpointer data)
{
    if (event->type != GDK_BUTTON_PRESS)
        return FALSE;

    flyParticle * myFly = static_cast <flyParticle *> (data);

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

static void frame_changed (GtkRange *, gpointer user_data)
{
    flyParticle * myDialog = static_cast <flyParticle *> (user_data);
    myDialog->sliderChanged();
}

static void gui_update (GtkObject *, gpointer user_data)
{
    if (lock)
        return;

    flyParticle * myDialog = static_cast <flyParticle *> (user_data);
    myDialog->getInputImage(); // refresh the input data
    myDialog->update();
}

static gboolean gui_draw (GtkWidget * widget, GdkEventExpose * event,
                          gpointer user_data)
{
    flyParticle * myDialog = static_cast <flyParticle *> (user_data);
    myDialog->display();
    return TRUE;
}

static void browse_button_clicked (GtkButton *, gpointer user_data)
{
    flyParticle * myDialog = static_cast <flyParticle *> (user_data);

    myDialog->getMenuValues();  // so we know the output format

    // First, determine a default output file if we don't already have an
    // output file.

    const char * filename
        = gtk_entry_get_text (GTK_ENTRY(WID(outputFileEntry)));
    const int MAX_SEL = 2048;
    char buffer [MAX_SEL + 1];
    const char * lastfilename;
    const char * defaultSuffix
        = (myDialog->param.output_format
           == ADMVideoParticle::OUTPUTFMT_FORMAT_AB_ODU) ? "csv" : "pts";
    if ((!filename || !*filename)
        && prefs->get (LASTFILES_FILE1, (ADM_filename **)&lastfilename))
    {
        strcpy (buffer, lastfilename);
        char * cptr = buffer + strlen (buffer);
        while (cptr > buffer)
        {
            if (*cptr == '.')
            {
                strcpy (cptr + 1, defaultSuffix);
                filename = buffer;
                printf ("Default output filename is %s based on %s + %s\n",
                        filename, lastfilename, defaultSuffix);
                break;
            }
            --cptr;
        }
    }
    else if (!filename)
        filename = "";

    if (FileSel_SelectWrite ("Write Particle List to File",
                             buffer, MAX_SEL, filename))
    {
        gtk_entry_set_text (GTK_ENTRY(WID(outputFileEntry)), buffer);
    }
}

/********************************************************************/

static void previewOutputMenuChange (GtkComboBox * combo, gpointer user_data)
{
    flyParticle * myFly = static_cast <flyParticle *> (user_data);
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
        flyParticle::PreviewMode mode;
        if (index == myFly->this_filter_index)
            mode = flyParticle::PREVIEWMODE_THIS_FILTER;
        else if (index > myFly->this_filter_index)
            mode = flyParticle::PREVIEWMODE_LATER_FILTER;
        else // if (index < myFly->this_filter_index)
            mode = flyParticle::PREVIEWMODE_EARLIER_FILTER;

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

    flyParticle * myFly = static_cast <flyParticle *> (user_data);
    myFly->recomputeSize();

    return FALSE;
}

/********************************************************************/

uint8_t DIA_particle (AVDMGenericVideoStream *in,
                      ADMVideoParticle * particlep,
                      PARTICLE_PARAM * param,
                      const MenuMapping * menu_mapping,
                      uint32_t menu_mapping_count)
{
    // Allocate space for preview video
    uint32_t width = in->getInfo()->width;
    uint32_t height = in->getInfo()->height;

    dialog = create_particle_dialog();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
								GTK_RESPONSE_OK,
								GTK_RESPONSE_CANCEL,
								-1);
    gtk_register_dialog (dialog);
    gtk_window_set_title (GTK_WINDOW (dialog),
                          QT_TR_NOOP("Particle Analysis Configuration"));
    gtk_widget_show (dialog);	

    // Fix up a bunch of things that Glade can't do.  This is less efficient
    // than just editing the Glade output, but it's not that big a deal and
    // doing it this way makes it MUCH easier to use Glade to tweak the layout
    // later.

    // actually, nothing to fix up in this one.  But if there ever is, this is
    // the place to do it...

    flyParticle * myDialog
        = new flyParticle (width, height, in,
                           WID(previewVideo), WID(previewSlider),
                           GTK_DIALOG(dialog), particlep, param,
                           menu_mapping, menu_mapping_count);

    g_signal_connect (GTK_OBJECT (WID(previewVideo)), "configure-event",
                      GTK_SIGNAL_FUNC (preview_video_configured),
                      gpointer (myDialog));

    myDialog->upload();
    myDialog->sliderChanged();

    g_signal_connect (GTK_OBJECT(WID(outputFileBrowseButton)), "clicked",
                      GTK_SIGNAL_FUNC(browse_button_clicked),
                      gpointer(myDialog));

    // update things when settings are changed
#define CNX(_widg,_signame) \
    g_signal_connect (GTK_OBJECT(WID(_widg)), _signame,                \
                      GTK_SIGNAL_FUNC(gui_update), gpointer(myDialog));

    CNX (minAreaSpinButton, "value_changed");
    CNX (maxAreaSpinButton, "value_changed");
    CNX (leftCropSpinButton, "value_changed");
    CNX (rightCropSpinButton, "value_changed");
    CNX (topCropSpinButton, "value_changed");
    CNX (bottomCropSpinButton, "value_changed");

    g_signal_connect (GTK_OBJECT(WID(previewSlider)), "value_changed",
                      GTK_SIGNAL_FUNC(frame_changed), gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "expose_event",
                      GTK_SIGNAL_FUNC(gui_draw), gpointer(myDialog));

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

    delete myDialog;

    return ret;
}

/**************************************/

uint8_t flyParticle::upload (void)
{
    lock++;

    gtk_entry_set_text (GTK_ENTRY(WID(outputFileEntry)),
                        param.output_file.c_str());

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(minAreaSpinButton)), param.min_area);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(maxAreaSpinButton)), param.max_area);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(leftCropSpinButton)), param.left_crop);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(rightCropSpinButton)), param.right_crop);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(topCropSpinButton)), param.top_crop);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(bottomCropSpinButton)), param.bottom_crop);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(cameraNumberSpinButton)), param.camera_number);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(debugSpinButton)), param.debug);

    lock--;
    return 1;
}

uint8_t flyParticle::download (void)
{
    getMenuValues();

    const char * filename =
        gtk_entry_get_text (GTK_ENTRY(WID(outputFileEntry)));

    // Note that the documentation states clearly that the result of
    // gtk_entry_get_text() must NOT be freed, modified, or stored.  This is
    // different than gtk_file_chooser_get_filename(), which returns a pointer
    // which must eventually be passed to g_free().

    if (filename)
        param.output_file = filename;
    else
        param.output_file = "";

    param.min_area
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(minAreaSpinButton)));
    param.max_area
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(maxAreaSpinButton)));
    param.left_crop
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(leftCropSpinButton)));
    param.right_crop
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(rightCropSpinButton)));
    param.top_crop
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(topCropSpinButton)));
    param.bottom_crop
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(bottomCropSpinButton)));
    param.camera_number
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(cameraNumberSpinButton)));
    param.debug
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(debugSpinButton)));

    return 1;
}

// The following was generated by glade from particle.glade once upon a time
// (in fact, several times, since it took a few tries to get it right!  :-).
// Thus far, I've avoided editing it, so regenerating the code from Glade (I
// used 2.12.1) and swapping it in here should not lose anything.  All the
// tweaking and adjusting and signal-connecting is done in code above, which
// is maintained by hand, and which has some knowledge of the names and layout
// of the widgets (so be cautious about renaming things!).

GtkWidget*
create_particle_dialog (void)
{
  GtkWidget *particle_dialog;
  GtkWidget *dialogVbox;
  GtkWidget *dialogHbox;
  GtkWidget *allButButtonsHbox;
  GtkWidget *settingsOuterHbox;
  GtkWidget *settingsOuterVbox;
  GtkWidget *minAreaHbox;
  GtkWidget *minAreaLabel;
  GtkObject *minAreaSpinButton_adj;
  GtkWidget *minAreaSpinButton;
  GtkWidget *maxAreaHbox;
  GtkWidget *maxAreaLabel;
  GtkObject *maxAreaSpinButton_adj;
  GtkWidget *maxAreaSpinButton;
  GtkWidget *aboveCropSeparator;
  GtkWidget *leftCropHbox;
  GtkWidget *leftCropLabel;
  GtkObject *leftCropSpinButton_adj;
  GtkWidget *leftCropSpinButton;
  GtkWidget *rightCropHbox;
  GtkWidget *rightCropLabel;
  GtkObject *rightCropSpinButton_adj;
  GtkWidget *rightCropSpinButton;
  GtkWidget *topCropHbox;
  GtkWidget *topCropLabel;
  GtkObject *topCropSpinButton_adj;
  GtkWidget *topCropSpinButton;
  GtkWidget *bottomCropHbox;
  GtkWidget *bottomCropLabel;
  GtkObject *bottomCropSpinButton_adj;
  GtkWidget *bottomCropSpinButton;
  GtkWidget *belowCropSeparator;
  GtkWidget *outputFormatHbox;
  GtkWidget *outputFormatLabel;
  GtkWidget *outputFormatMenu;
  GtkWidget *outputFileHbox;
  GtkWidget *outputFileLabel;
  GtkWidget *outputFileEntry;
  GtkWidget *outputFileBrowseButton;
  GtkWidget *cameraNumberHbox;
  GtkWidget *cameraNumberLabel;
  GtkObject *cameraNumberSpinButton_adj;
  GtkWidget *cameraNumberSpinButton;
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
  GtkWidget *previewLabel;
  GtkWidget *dialogButtonBox;
  GtkWidget *cancelButton;
  GtkWidget *okButton;

  particle_dialog = gtk_dialog_new ();
  //                                                NO NEED TO "FIX" THAT _("...")!!
  //                                                see handy macro near top of file.
  gtk_window_set_title (GTK_WINDOW (particle_dialog), _("Particle Detection"));
  gtk_window_set_type_hint (GTK_WINDOW (particle_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialogVbox = GTK_DIALOG (particle_dialog)->vbox;
  gtk_widget_show (dialogVbox);

  dialogHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (dialogHbox);
  gtk_box_pack_start (GTK_BOX (dialogVbox), dialogHbox, TRUE, TRUE, 0);

  allButButtonsHbox = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (allButButtonsHbox);
  gtk_box_pack_start (GTK_BOX (dialogHbox), allButButtonsHbox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (allButButtonsHbox), 8);

  settingsOuterHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (settingsOuterHbox);
  gtk_box_pack_start (GTK_BOX (allButButtonsHbox), settingsOuterHbox, FALSE, TRUE, 0);

  settingsOuterVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (settingsOuterVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterHbox), settingsOuterVbox, FALSE, TRUE, 0);

  minAreaHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (minAreaHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), minAreaHbox, FALSE, TRUE, 0);

  minAreaLabel = gtk_label_new_with_mnemonic (_("Mi_nimum area for a particle to be detected:    "));
  gtk_widget_show (minAreaLabel);
  gtk_box_pack_start (GTK_BOX (minAreaHbox), minAreaLabel, FALSE, FALSE, 0);

  minAreaSpinButton_adj = gtk_adjustment_new (5, 1, 999999, 1, 1, 0);
  minAreaSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (minAreaSpinButton_adj), 1, 0);
  gtk_widget_show (minAreaSpinButton);
  gtk_box_pack_start (GTK_BOX (minAreaHbox), minAreaSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (minAreaSpinButton), TRUE);

  maxAreaHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (maxAreaHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), maxAreaHbox, FALSE, TRUE, 0);

  maxAreaLabel = gtk_label_new_with_mnemonic (_("Ma_ximum area for a particle to be detected:   "));
  gtk_widget_show (maxAreaLabel);
  gtk_box_pack_start (GTK_BOX (maxAreaHbox), maxAreaLabel, FALSE, FALSE, 0);

  maxAreaSpinButton_adj = gtk_adjustment_new (50000, 1, 999999, 1, 1, 0);
  maxAreaSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (maxAreaSpinButton_adj), 1, 0);
  gtk_widget_show (maxAreaSpinButton);
  gtk_box_pack_start (GTK_BOX (maxAreaHbox), maxAreaSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (maxAreaSpinButton), TRUE);

  aboveCropSeparator = gtk_hseparator_new ();
  gtk_widget_show (aboveCropSeparator);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), aboveCropSeparator, FALSE, TRUE, 0);

  leftCropHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (leftCropHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), leftCropHbox, FALSE, TRUE, 0);

  leftCropLabel = gtk_label_new_with_mnemonic (_("_Left side crop (ignore this many pixels on the left):        "));
  gtk_widget_show (leftCropLabel);
  gtk_box_pack_start (GTK_BOX (leftCropHbox), leftCropLabel, FALSE, FALSE, 0);

  leftCropSpinButton_adj = gtk_adjustment_new (0, 0, 2048, 1, 1, 0);
  leftCropSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (leftCropSpinButton_adj), 1, 0);
  gtk_widget_show (leftCropSpinButton);
  gtk_box_pack_start (GTK_BOX (leftCropHbox), leftCropSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (leftCropSpinButton), TRUE);

  rightCropHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (rightCropHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), rightCropHbox, FALSE, TRUE, 0);

  rightCropLabel = gtk_label_new_with_mnemonic (_("_Right side crop (ignore this many pixels on the right):     "));
  gtk_widget_show (rightCropLabel);
  gtk_box_pack_start (GTK_BOX (rightCropHbox), rightCropLabel, FALSE, FALSE, 0);

  rightCropSpinButton_adj = gtk_adjustment_new (0, 0, 2048, 1, 1, 0);
  rightCropSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (rightCropSpinButton_adj), 1, 0);
  gtk_widget_show (rightCropSpinButton);
  gtk_box_pack_start (GTK_BOX (rightCropHbox), rightCropSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (rightCropSpinButton), TRUE);

  topCropHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (topCropHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), topCropHbox, FALSE, TRUE, 0);

  topCropLabel = gtk_label_new_with_mnemonic (_("_Top crop (ignore this many pixels on the top):                  "));
  gtk_widget_show (topCropLabel);
  gtk_box_pack_start (GTK_BOX (topCropHbox), topCropLabel, FALSE, FALSE, 0);

  topCropSpinButton_adj = gtk_adjustment_new (0, 0, 2048, 1, 1, 0);
  topCropSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (topCropSpinButton_adj), 1, 0);
  gtk_widget_show (topCropSpinButton);
  gtk_box_pack_start (GTK_BOX (topCropHbox), topCropSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (topCropSpinButton), TRUE);

  bottomCropHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (bottomCropHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), bottomCropHbox, FALSE, TRUE, 0);

  bottomCropLabel = gtk_label_new_with_mnemonic (_("_Bottom crop (ignore this many pixels on the bottom):   "));
  gtk_widget_show (bottomCropLabel);
  gtk_box_pack_start (GTK_BOX (bottomCropHbox), bottomCropLabel, FALSE, FALSE, 0);

  bottomCropSpinButton_adj = gtk_adjustment_new (0, 0, 2048, 1, 1, 0);
  bottomCropSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (bottomCropSpinButton_adj), 1, 0);
  gtk_widget_show (bottomCropSpinButton);
  gtk_box_pack_start (GTK_BOX (bottomCropHbox), bottomCropSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (bottomCropSpinButton), TRUE);

  belowCropSeparator = gtk_hseparator_new ();
  gtk_widget_show (belowCropSeparator);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), belowCropSeparator, FALSE, TRUE, 0);

  outputFormatHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (outputFormatHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), outputFormatHbox, FALSE, TRUE, 0);

  outputFormatLabel = gtk_label_new_with_mnemonic (_("Output _Format:   "));
  gtk_widget_show (outputFormatLabel);
  gtk_box_pack_start (GTK_BOX (outputFormatHbox), outputFormatLabel, FALSE, FALSE, 0);

  outputFormatMenu = gtk_combo_box_new_text ();
  gtk_widget_show (outputFormatMenu);
  gtk_box_pack_start (GTK_BOX (outputFormatHbox), outputFormatMenu, TRUE, TRUE, 0);

  outputFileHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (outputFileHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), outputFileHbox, FALSE, TRUE, 0);

  outputFileLabel = gtk_label_new_with_mnemonic (_("_Output File:"));
  gtk_widget_show (outputFileLabel);
  gtk_box_pack_start (GTK_BOX (outputFileHbox), outputFileLabel, FALSE, FALSE, 0);

  outputFileEntry = gtk_entry_new ();
  gtk_widget_show (outputFileEntry);
  gtk_box_pack_start (GTK_BOX (outputFileHbox), outputFileEntry, TRUE, TRUE, 0);
  gtk_entry_set_invisible_char (GTK_ENTRY (outputFileEntry), 8226);
  gtk_entry_set_width_chars (GTK_ENTRY (outputFileEntry), 40);

  outputFileBrowseButton = gtk_button_new_with_mnemonic (_("_Browse..."));
  gtk_widget_show (outputFileBrowseButton);
  gtk_box_pack_start (GTK_BOX (outputFileHbox), outputFileBrowseButton, FALSE, FALSE, 0);

  cameraNumberHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (cameraNumberHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), cameraNumberHbox, FALSE, TRUE, 0);

  cameraNumberLabel = gtk_label_new_with_mnemonic (_("_Camera Number for output file:      "));
  gtk_widget_show (cameraNumberLabel);
  gtk_box_pack_start (GTK_BOX (cameraNumberHbox), cameraNumberLabel, FALSE, FALSE, 0);

  cameraNumberSpinButton_adj = gtk_adjustment_new (1, 0, 99999, 1, 1, 0);
  cameraNumberSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (cameraNumberSpinButton_adj), 1, 0);
  gtk_widget_show (cameraNumberSpinButton);
  gtk_box_pack_start (GTK_BOX (cameraNumberHbox), cameraNumberSpinButton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (cameraNumberSpinButton), TRUE);

  debugHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (debugHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), debugHbox, FALSE, TRUE, 0);

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
  gtk_box_pack_start (GTK_BOX (allButButtonsHbox), previewVboxOuter, TRUE, TRUE, 0);

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

  previewLabel = gtk_label_new (_("Preview"));
  gtk_widget_show (previewLabel);
  gtk_frame_set_label_widget (GTK_FRAME (previewFrame), previewLabel);

  dialogButtonBox = GTK_DIALOG (particle_dialog)->action_area;
  gtk_widget_show (dialogButtonBox);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialogButtonBox), GTK_BUTTONBOX_END);

  cancelButton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (particle_dialog), cancelButton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelButton, GTK_CAN_DEFAULT);

  okButton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (particle_dialog), okButton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okButton, GTK_CAN_DEFAULT);

  gtk_label_set_mnemonic_widget (GTK_LABEL (minAreaLabel), minAreaSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (maxAreaLabel), maxAreaSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (leftCropLabel), leftCropSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (rightCropLabel), rightCropSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (topCropLabel), topCropSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (bottomCropLabel), bottomCropSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (outputFileLabel), outputFileEntry);
  gtk_label_set_mnemonic_widget (GTK_LABEL (cameraNumberLabel), cameraNumberSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (debugLabel), debugSpinButton);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (particle_dialog, particle_dialog, "particle_dialog");
  GLADE_HOOKUP_OBJECT_NO_REF (particle_dialog, dialogVbox, "dialogVbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, dialogHbox, "dialogHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, allButButtonsHbox, "allButButtonsHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, settingsOuterHbox, "settingsOuterHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, settingsOuterVbox, "settingsOuterVbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, minAreaHbox, "minAreaHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, minAreaLabel, "minAreaLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, minAreaSpinButton, "minAreaSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, maxAreaHbox, "maxAreaHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, maxAreaLabel, "maxAreaLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, maxAreaSpinButton, "maxAreaSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, aboveCropSeparator, "aboveCropSeparator");
  GLADE_HOOKUP_OBJECT (particle_dialog, leftCropHbox, "leftCropHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, leftCropLabel, "leftCropLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, leftCropSpinButton, "leftCropSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, rightCropHbox, "rightCropHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, rightCropLabel, "rightCropLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, rightCropSpinButton, "rightCropSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, topCropHbox, "topCropHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, topCropLabel, "topCropLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, topCropSpinButton, "topCropSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, bottomCropHbox, "bottomCropHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, bottomCropLabel, "bottomCropLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, bottomCropSpinButton, "bottomCropSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, belowCropSeparator, "belowCropSeparator");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFormatHbox, "outputFormatHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFormatLabel, "outputFormatLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFormatMenu, "outputFormatMenu");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFileHbox, "outputFileHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFileLabel, "outputFileLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFileEntry, "outputFileEntry");
  GLADE_HOOKUP_OBJECT (particle_dialog, outputFileBrowseButton, "outputFileBrowseButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, cameraNumberHbox, "cameraNumberHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, cameraNumberLabel, "cameraNumberLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, cameraNumberSpinButton, "cameraNumberSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, debugHbox, "debugHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, debugLabel, "debugLabel");
  GLADE_HOOKUP_OBJECT (particle_dialog, debugSpinButton, "debugSpinButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewVboxOuter, "previewVboxOuter");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewFrame, "previewFrame");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewAlignment, "previewAlignment");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewVbox, "previewVbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewControlHbox, "previewControlHbox");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewOutputMenu, "previewOutputMenu");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewSlider, "previewSlider");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewVideo, "previewVideo");
  GLADE_HOOKUP_OBJECT (particle_dialog, previewLabel, "previewLabel");
  GLADE_HOOKUP_OBJECT_NO_REF (particle_dialog, dialogButtonBox, "dialogButtonBox");
  GLADE_HOOKUP_OBJECT (particle_dialog, cancelButton, "cancelButton");
  GLADE_HOOKUP_OBJECT (particle_dialog, okButton, "okButton");

  return particle_dialog;
}

#endif

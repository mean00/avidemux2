/***************************************************************************
                          DIA_eraser.cpp  -  configuration dialog for
						Eraser filter
                              -------------------
                         Chris MacGregor, December 2007
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
#include "prefs.h"

#include "avi_vars.h"

#ifdef HAVE_ENCODER

#include <algorithm>

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_videoFilter/ADM_vidEraser.h"

#include "DIA_fileSel.h"

#include "DIA_flyDialog.h"
#include "DIA_flyEraser.h"
#include "DIA_factory.h"


using namespace std;

#undef _
#define _(_s) QT_TR_NOOP(_s)

/********************************************************************/
static GtkWidget * create_eraser_dialog (void);
static GtkWidget * dialog = 0;

static gboolean gui_draw (GtkWidget * widget,
			  GdkEventExpose * event, gpointer user_data);
static void gui_update (GtkObject * button, gpointer user_data);
static void frame_changed (GtkRange *, gpointer user_data);

static volatile int lock = 0;

static const int MAX_FRAME_NUM = 99999999;

/********************************************************************/

class flyEraserGtk : public flyEraser
{
protected:
    GtkWidget * dialog;
    GtkListStore * list_store;
    GtkTreeModel * tree_model;
    GtkWidget * tree_view_widget;
    GtkTreeView * tree_view;
public:
    GtkTreeSelection * tree_sel;
    bool warned_about_scaling;

public:
    uint8_t    download();
    uint8_t    upload();
    uint8_t    upload_masklist (bool set_preview_frame);
    void       wipeOutputBuffer();

    flyEraserGtk (uint32_t width, uint32_t height,
                  AVDMGenericVideoStream * in,
                  void * canvas, void * slider, GtkWidget * dialog,
                  ADMVideoEraser * eraserp, ERASER_PARAM * in_param,
                  const MenuMapping * menu_mapping,
                  uint32_t menu_mapping_count)
        : flyEraser (width, height, in, canvas, slider, dialog,
                     eraserp, in_param, menu_mapping, menu_mapping_count),
          dialog (dialog),
          list_store (gtk_list_store_new (2, G_TYPE_INT, G_TYPE_INT)),
          tree_model (GTK_TREE_MODEL (list_store)),
          tree_view_widget (WID (rangeListTreeview)),
          tree_view (GTK_TREE_VIEW (tree_view_widget)),
          tree_sel (gtk_tree_view_get_selection (tree_view)),
          warned_about_scaling (false)
    {
        // printf ("flyEraserGtk::flyEraserGtk: "
        //         "in_param %p, &param %p\n", in_param, &param);

        gtk_tree_view_set_model (tree_view, tree_model);
        gtk_tree_view_columns_autosize ((tree_view));
        gtk_tree_selection_set_mode (tree_sel, GTK_SELECTION_BROWSE);
    };

    ~flyEraserGtk ()
    {
        g_object_unref (list_store);
    }

    void selection_changed ();
};

static gboolean previewSomeEvent (GtkWidget * widget,
                                  uint8_t button, float fx, float fy,
                                  guint state, gpointer data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (data);

    uint32_t x, y;

    float zoom = myFly->getZoom();
    if (zoom == 1.0)
    {
        x = static_cast <uint32_t> (fx);
        y = static_cast <uint32_t> (fy);
    }
    else
    {
        if (!myFly->warned_about_scaling)
        {
            fprintf (stderr, "******* WARNING: you are erasing in a "
                     "scaled-down window (%.5f) - all pixel positions are "
                     "therefore APPROXIMATE!!!\n", zoom);
            myFly->warned_about_scaling = true;
        }

        zoom = 1 / zoom;
        x = static_cast <uint32_t> (fx * zoom + .5);
        y = static_cast <uint32_t> (fy * zoom + .5);
    }

    bool erasing = (button == 1) ? myFly->param.brush_mode
                   : !myFly->param.brush_mode;
    ADMImage * image = myFly->getOutputImage();

    uint16_t width = image->_width;
    uint16_t height = image->_height;

    if (x >= width || y >= height)
        return FALSE;

    uint8_t brush_size = myFly->param.brush_size;
    if (state & GDK_SHIFT_MASK && brush_size > 0)
        --brush_size;
    else if (state & GDK_CONTROL_MASK)
        ++brush_size;
    brush_size = brush_size * 2 + 1;

    uint8_t halfrange = brush_size / 2;

    uint32_t xmin = (x > halfrange) ? (x - halfrange) : 0;
    uint32_t ymin = (y > halfrange) ? (y - halfrange) : 0;

    uint32_t xmax = x + halfrange;
    uint32_t ymax = y + halfrange;

    if (xmax >= width)
        xmax = width - 1;
    brush_size = xmax - xmin + 1;
    if (ymax >= height)
        ymax = height - 1;

    Eraser::LineVec & lines = myFly->current_mask->lines;
    Eraser::LineVec::iterator lineit = lines.begin();

    // HERE: We could improve performance by doing some kind of binary search
    // to find the line with the first y.  After that it probably wouldn't
    // help all that much, if at all.  However, unless they're masking a lot
    // of pixels, and tweaking them frequently and/or on slow machines, it may
    // not be worth the trouble.

    for (y = ymin; y <= ymax; y++)
    {
        while (lineit != lines.end() && lineit->y < y)
            ++lineit;

        if (erasing)
        {
            bool need_insert = true;
            while (lineit != lines.end() && lineit->y == y)
            {
                if (xmax + 1 < lineit->x)
                {
                    // This line is to the right of and is not touching the new
                    // line.  Since the lines are sorted and we haven't stopped
                    // before this, that means that there are no lines with which
                    // we can connect.  Break out and insert a separate new line.
                    break;
                }

                Eraser::Line & line = *lineit++;

                uint16_t currlinexmaxplus1 = line.x + line.count;
                if (currlinexmaxplus1 < xmin)
                {
                    // This line is to the left of and is not touching the new
                    // line.  Skip it and continue looking.
                    continue;
                }

                // We get here whenever, and only if, the new line touches the
                // current line in some way.  It may overlap the head (left end),
                // or the tail (right end), or both, or neither (in which case no
                // change is made); in any case, there is no need to insert a new
                // line object.  If we overlap the tail of the current line, then
                // it is possible that we also will overlap (and can connect with,
                // and thus delete) the following line.  (It is also possible that
                // we extend the tail of it, and perhaps even connect to the
                // following one, etc., so we iterate to be sure we handle all
                // such cases.  Having trouble picturing how that could happen?
                // Here's one way:
                //                     |0    5    
                //                     |..*.*.*.*..|
                // 
                // Now imagine a 9x9 brush covering coordinates 1 through 9.)

                if (xmin < line.x)
                {
                    // The new line overlaps (and extends) the beginning of this
                    // line.  No insertion or deletion is required.
                    line.x = xmin;
                    line.count = currlinexmaxplus1 - xmin;
                }

                if (xmax >= currlinexmaxplus1)
                {
                    // The new line overlaps (and extends) the end of this line.
                    line.count = xmax - line.x + 1;

                    // Possibly we can also merge with the head of the next line,
                    // in which case we can delete that line.  As described above,
                    // there may be more than one such, and so we iterate until
                    // there are none.

                    while (lineit != lines.end() && xmax + 1 >= lineit->x
                           && lineit->y == y)
                    {
                        // Okay, here we go.  We fold the next line into the
                        // current line, and delete that line.
                        line.count = max (lineit->x + lineit->count,
                                          int (xmax + 1))
                                     - line.x;
                        lineit = lines.erase (lineit);
                    }
                }

                need_insert = false;
                break;
            }

            if (need_insert)
                lineit = lines.insert (lineit,
                                       Eraser::Line (xmin, y, brush_size));
        }
        else // if (!erasing) (unerasing)
        {
            while (lineit != lines.end() && lineit->y == y)
            {
                Eraser::Line & line = *lineit;

                if (xmax < line.x)
                {
                    // This line is to the right of and does not overlap the
                    // area we are un-erasing.  Since the lines are sorted and
                    // we haven't stopped before this, that means that there
                    // is nothing (more) for us to delete, so we break out.
                    break;
                }

                uint16_t currlinexmaxplus1 = line.x + line.count;
                if (currlinexmaxplus1 <= xmin)
                {
                    // This line is to the left of and does not overlap the
                    // area we are un-erasing.  Skip it and continue looking.
                    ++lineit;
                    continue;
                }

                // We get here whenever, and only if, we are un-erasing some
                // part of the current line.  We may be un-erasing the head
                // (left end), or the tail (right end), or both (the entire
                // line, in which case we delete it from the list).  If we are
                // un-erasing the middle of the current line (but not either
                // end), then we will need to split it into two separate lines
                // (thus inserting one new one).  If we are un-erasing the
                // tail (or more) of the current line, then it is possible
                // that we also will need to do something with the following
                // line, and perhaps also the one after that, etc., so we
                // allow the loop to continue to be sure we handle all such
                // cases.  Having trouble picturing how that could happen?
                // Here's one way:
                //
                //                     |0    5    
                //                     |..*.*.*.*..|
                // 
                // Now imagine a 9x9 brush covering coordinates 1 through 9.)

                if (xmax + 1 >= currlinexmaxplus1)
                {
                    // The area to erase overlaps (at least) the tail of the
                    // current line.  Let's check the beginning...

                    if (xmin <= line.x)
                    {
                        // We overlap the head and the tail, so we can just
                        // delete this entire line and continue on to examine
                        // the next one.
                        lineit = lines.erase (lineit);
                        continue;
                    }

                    // We're just making this line shorter.  However, we might
                    // overlap the following line(s), too, so we need to keep
                    // looking.

                    line.count = xmin - line.x;
                }
                else if (xmin <= line.x)
                {
                    // We're erasing the head of this line, but not the tail.
                    // We know that we don't need to look at the following
                    // line, either.
                    line.x = xmax + 1;
                    line.count = currlinexmaxplus1 - line.x;
                    break;
                }
                else
                {
                    // We must be erasing a chunk of the middle of this line,
                    // which means we need to split it (inserting one new
                    // segment).  We know that we don't need to look at the
                    // following line, either.

                    Eraser::Line new_line (line.x, y, xmin - line.x);
                    line.x = xmax + 1;
                    line.count = currlinexmaxplus1 - line.x;
                    lineit = lines.insert (lineit, new_line);
                    break;
                }

                ++lineit;
            }
        }
    }

    if (myFly->param.debug & 0x40)
    {
        printf ("---------------------------------------------\n");
        for (Eraser::LineVec::const_iterator lineit = lines.begin();
             lineit != lines.end();
             ++lineit)
        {
            printf ("line: %d %d %d\n", lineit->x, lineit->y, lineit->count);
        }
    }

    myFly->update();

    return TRUE;
}

/********************************************************************/

static gboolean previewButtonEvent (GtkWidget * widget,
                                    GdkEventButton * event,
                                    gpointer data)
{
    if (event->type != GDK_BUTTON_PRESS)
        return FALSE;

    return previewSomeEvent (widget, event->button,
                             event->x, event->y, event->state, data);
}

/********************************************************************/

static gboolean previewMotionEvent (GtkWidget * widget,
                                    GdkEventMotion * event,
                                    gpointer data)
{
    if (event->type != GDK_MOTION_NOTIFY)
        return FALSE;

    uint8_t button;
    if (event->state & GDK_BUTTON1_MASK)
        button = 1;
    else if (event->state & GDK_BUTTON2_MASK)
        button = 2;
    else if (event->state & GDK_BUTTON3_MASK)
        button = 3;
    else
        return FALSE;

    return previewSomeEvent (widget, button, event->x, event->y,
                             event->state, data);
}

/********************************************************************/

static void browse_button_clicked (GtkButton *, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    // First, determine a default output file if we don't already have an
    // output file.

    const char * filename
        = gtk_entry_get_text (GTK_ENTRY(WID(eraserDataFileEntry)));
    const int MAX_SEL = 2048;
    char buffer [MAX_SEL + 1];
    const char * lastfilename;
    const char * defaultSuffix = "eraser_data";
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

    if (FileSel_SelectWrite ("Store Eraser Data in File",
                             buffer, MAX_SEL, filename))
    {
        gtk_entry_set_text (GTK_ENTRY(WID(eraserDataFileEntry)), buffer);
    }
}

/********************************************************************/

static void jump_first_button_clicked (GtkButton *, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    myFly->sliderSet (myFly->current_mask->first_frame);
    myFly->sliderChanged();
}

/********************************************************************/

static void jump_last_button_clicked (GtkButton *, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    myFly->sliderSet (myFly->current_mask->last_frame);
    myFly->sliderChanged();
}

/********************************************************************/

void flyEraserGtk::selection_changed ()
{
    if (lock)
        return;

    uint32_t sel_num = 99999;
    if (!getSelectionNumber (eraserp->getMasks().size(),
                             GTK_WIDGET (tree_view), list_store, &sel_num))
    {
        printf ("ugh, failed to determine which row is selected!!\n");
        sel_num = 0; // best we can do
    }
    else if (param.debug)
        printf ("row %d is selected\n", sel_num);

    ++lock;

    uint32_t current_sel = current_mask - eraserp->getMasks().begin();
    if (sel_num != current_sel)
    {
        current_mask = eraserp->getMasks().begin() + sel_num;
        gtk_spin_button_set_value
            (GTK_SPIN_BUTTON(WID(frameRangeFirstSpinButton)),
             current_mask->first_frame);
        gtk_spin_button_set_value
            (GTK_SPIN_BUTTON(WID(frameRangeLastSpinButton)),
             current_mask->last_frame);
    }

    uint32_t middle_frame = (current_mask->first_frame
                             + current_mask->last_frame) / 2;
    if (middle_frame >= _in->getInfo()->nb_frames)
        middle_frame = current_mask->first_frame;

    if (sliderGet() != middle_frame)
    {
        sliderSet (middle_frame);
        sliderChanged();
    }

    --lock;
}

/********************************************************************/

static void treeview_selection_changed (GtkTreeSelection * sel,
                                        gpointer user_data)
{
    if (lock)
        return;

    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    myFly->selection_changed();
}

/********************************************************************/

static void frame_range_changed (GtkSpinButton * spinbutton,
                                 gpointer user_data)
{
    if (lock)
        return;

    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    bool changed = false;

    Eraser::MaskVec::iterator & current_mask = myFly->current_mask;
    Eraser::MaskVec & masks = myFly->eraserp->getMasks();
    typedef Eraser::MaskVec::iterator MaskIter;
    typedef Eraser::MaskVec::reverse_iterator MaskRiter;

    int32_t value = gtk_spin_button_get_value_as_int
                    (GTK_SPIN_BUTTON(WID(frameRangeFirstSpinButton)));
    if (value != current_mask->first_frame)
    {
        current_mask->first_frame = value;
        changed = true;
        int32_t curr_value = value;
        for (MaskRiter maskit (current_mask); maskit != masks.rend(); ++maskit)
        {
            if (maskit->last_frame >= curr_value)
                maskit->last_frame = max (curr_value - 1, 0);
            else
                break;

            if (maskit->first_frame >= curr_value)
                maskit->first_frame = max (curr_value - 1, 0);
            else
                break;

            curr_value = max (curr_value - 1, 0);
        }

        // we deliberately include the current mask in the following loop

        curr_value = value;
        for (MaskIter maskit (current_mask); maskit != masks.end(); ++maskit)
        {
            if (maskit->last_frame < curr_value)
                maskit->last_frame = curr_value;
            else
                break;

            if (maskit->first_frame < curr_value)
                maskit->first_frame = curr_value;
            else
                break;

            ++curr_value;
        }
    }

    value = gtk_spin_button_get_value_as_int
            (GTK_SPIN_BUTTON(WID(frameRangeLastSpinButton)));
    if (value != myFly->current_mask->last_frame)
    {
        myFly->current_mask->last_frame = value;
        changed = true;

        // we deliberately include the current mask in the following loop

        int32_t curr_value = value;
        for (MaskRiter maskit (current_mask + 1);
             maskit != masks.rend();
             ++maskit)
        {
            if (maskit->first_frame > curr_value)
                maskit->first_frame = curr_value;
            else
                break;

            if (maskit->last_frame > curr_value)
                maskit->last_frame = curr_value;
            else
                break;

            curr_value = max (curr_value - 1, 0);
        }

        curr_value = value;
        for (MaskIter maskit (current_mask + 1);
             maskit != masks.end();
             ++maskit)
        {
            if (maskit->first_frame <= curr_value)
                maskit->first_frame = curr_value + 1;
            else
                break;

            if (maskit->last_frame <= curr_value)
                maskit->last_frame = curr_value + 1;
            else
                break;

            ++curr_value;
        }
    }

    if (!changed)
        return;

    myFly->upload_masklist (false);
    gui_update (GTK_OBJECT (spinbutton), user_data);
}

/********************************************************************/

static void insert_button_clicked (GtkButton * button, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    bool current_changed = false;

    Eraser::MaskVec::iterator & current_mask = myFly->current_mask;
    Eraser::MaskVec & masks = myFly->eraserp->getMasks();
    typedef Eraser::MaskVec::iterator MaskIter;

    int32_t frame = current_mask->last_frame + 1;
    if (button == GTK_BUTTON (WID (duplicateButton)))
        current_mask = masks.insert (current_mask + 1,
                                     Eraser::Mask (frame, frame,
                                                   current_mask->lines));
    else
        current_mask = masks.insert (current_mask + 1,
                                     Eraser::Mask (frame, frame));

    for (MaskIter maskit (current_mask + 1); maskit != masks.end(); ++maskit)
    {
        if (maskit->first_frame <= frame)
            maskit->first_frame = frame + 1;
        else
            break;

        if (maskit->last_frame <= frame)
            maskit->last_frame = frame + 1;
        else
            break;

        ++frame;
    }

    myFly->upload_masklist (true);
    gui_update (GTK_OBJECT (button), user_data);
}

/********************************************************************/

static void delete_button_clicked (GtkButton * button, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);

    bool current_changed = false;

    Eraser::MaskVec::iterator & current_mask = myFly->current_mask;
    Eraser::MaskVec & masks = myFly->eraserp->getMasks();

    current_mask = masks.erase (current_mask);
    if (current_mask == masks.end())
    {
        if (masks.empty())
            masks.push_back (Eraser::Mask (0, MAX_FRAME_NUM));
        current_mask = masks.end() - 1;
    }

    myFly->upload_masklist (true);
    gui_update (GTK_OBJECT (button), user_data);
}

/********************************************************************/

static void previewOutputMenuChange (GtkComboBox * combo, gpointer user_data)
{
    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);
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
        flyEraserGtk::PreviewMode mode;
        if (index == myFly->this_filter_index)
            mode = flyEraserGtk::PREVIEWMODE_THIS_FILTER;
        else if (index > myFly->this_filter_index)
            mode = flyEraserGtk::PREVIEWMODE_LATER_FILTER;
        else // if (index < myFly->this_filter_index)
            mode = flyEraserGtk::PREVIEWMODE_EARLIER_FILTER;

        myFly->changeSource (filter->filter, mode);
    }

    g_free (activestr);
}

/********************************************************************/

gboolean preview_video_configured (GtkWidget * widget, GdkEventConfigure * event,
                                   gpointer user_data)
{
    fprintf (stderr, "preview_configured: now %dx%d @ +%d+%d\n",
             event->width, event->height, event->x, event->y);

    flyEraserGtk * myFly = static_cast <flyEraserGtk *> (user_data);
    myFly->recomputeSize();

    return FALSE;
}

/********************************************************************/

uint8_t DIA_eraser (AVDMGenericVideoStream * in, ADMVideoEraser * eraserp,
                    ERASER_PARAM * param, const MenuMapping * menu_mapping,
                    uint32_t menu_mapping_count)
{
    // Allocate space for preview video
    uint32_t width = in->getInfo()->width;
    uint32_t height = in->getInfo()->height;

    // We never have an empty list - there is always at least one mask,
    // and if we create it the frame range is the entire video.

    Eraser::MaskVec & masks = eraserp->getMasks();
    if (masks.empty())
    {
        masks.push_back (Eraser::Mask (0, MAX_FRAME_NUM));
    }

    dialog = create_eraser_dialog();
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
								GTK_RESPONSE_OK,
								GTK_RESPONSE_CANCEL,
								-1);
    gtk_register_dialog (dialog);

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

    JOIN_SPINPAIR (color);

    // 2. Create the flyDialog.

    gtk_widget_show (dialog);

    flyEraserGtk * myDialog
        = new flyEraserGtk (width, height, in,
                            WID(previewVideo), WID(previewSlider),
                            dialog, eraserp, param,
                            menu_mapping, menu_mapping_count);

    g_signal_connect (GTK_OBJECT (WID(previewVideo)), "configure-event",
                      GTK_SIGNAL_FUNC (preview_video_configured),
                      gpointer (myDialog));

    myDialog->upload();
    myDialog->sliderChanged();

    // 3. Set up the treeview stuff.  This is waaaay more complicated than I
    // needed for my simple little list, but that's just how it is in GTK, I
    // guess.

    GtkCellRenderer * renderer = gtk_cell_renderer_text_new();

    gtk_tree_view_append_column
        (GTK_TREE_VIEW(WID(rangeListTreeview)),
         gtk_tree_view_column_new_with_attributes (_("First Frame"),
                                                   renderer, "text", 0,
                                                   NULL));
    gtk_tree_view_append_column
        (GTK_TREE_VIEW(WID(rangeListTreeview)),
         gtk_tree_view_column_new_with_attributes (_("Last Frame"),
                                                   renderer, "text", 1,
                                                   NULL));

    g_signal_connect (G_OBJECT(myDialog->tree_sel), "changed",
                      GTK_SIGNAL_FUNC(treeview_selection_changed),
                      gpointer(myDialog));

    // 4. Connect up all the other signals and stuff.

    g_signal_connect (GTK_OBJECT(WID(eraserDataFileBrowseButton)), "clicked",
                      GTK_SIGNAL_FUNC(browse_button_clicked),
                      gpointer(myDialog));

#define CNX(_widg,_signame) \
    g_signal_connect (GTK_OBJECT(WID(_widg)), _signame,                \
                      GTK_SIGNAL_FUNC(gui_update), gpointer(myDialog));

    CNX (brushModeMenu, "changed");
    CNX (brushSizeMenu, "changed");
    CNX (colorSpinner, "value_changed");
    // CNX (frameRangeFirstSpinButton, "value_changed");
    // CNX (frameRangeLastSpinButton, "value_changed");

    g_signal_connect (GTK_OBJECT(WID(frameRangeFirstSpinButton)),
                      "value_changed",
                      GTK_SIGNAL_FUNC(frame_range_changed),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(frameRangeLastSpinButton)),
                      "value_changed",
                      GTK_SIGNAL_FUNC(frame_range_changed),
                      gpointer(myDialog));

    g_signal_connect (GTK_OBJECT(WID(insertButton)), "clicked",
                      GTK_SIGNAL_FUNC(insert_button_clicked),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(duplicateButton)), "clicked",
                      GTK_SIGNAL_FUNC(insert_button_clicked),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(deleteButton)), "clicked",
                      GTK_SIGNAL_FUNC(delete_button_clicked),
                      gpointer(myDialog));

    // preview stuff:

    g_signal_connect (GTK_OBJECT(WID(previewSlider)), "value_changed",
                      GTK_SIGNAL_FUNC(frame_changed), gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "expose_event",
                      GTK_SIGNAL_FUNC(gui_draw), gpointer(myDialog));

    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "button_press_event",
                      GTK_SIGNAL_FUNC(previewButtonEvent),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(previewVideo)), "motion_notify_event",
                      GTK_SIGNAL_FUNC(previewMotionEvent),
                      gpointer(myDialog));

    g_signal_connect (GTK_OBJECT(WID(previewJumpFirstButton)), "clicked",
                      GTK_SIGNAL_FUNC(jump_first_button_clicked),
                      gpointer(myDialog));
    g_signal_connect (GTK_OBJECT(WID(previewJumpLastButton)), "clicked",
                      GTK_SIGNAL_FUNC(jump_last_button_clicked),
                      gpointer(myDialog));

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
        // Don't let them leave without asking for a filename - if they leave
        // with no filename selected, they will lose all their mask data!

        const char * filename =
            gtk_entry_get_text (GTK_ENTRY(WID(eraserDataFileEntry)));
        if (filename == NULL || *filename == '\0')
            browse_button_clicked (NULL, gpointer (myDialog));

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

void frame_changed (GtkRange *, gpointer user_data)
{
    flyEraserGtk * myDialog = static_cast <flyEraserGtk *> (user_data);

    myDialog->sliderChanged();
}

void gui_update (GtkObject *, gpointer user_data)
{
    if (lock)
        return;

    flyEraserGtk * myDialog = static_cast <flyEraserGtk *> (user_data);
    myDialog->update();
}

gboolean gui_draw (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
    flyEraserGtk * myDialog
        = static_cast <flyEraserGtk *> (user_data);
    myDialog->display();
    return TRUE;
}

/**************************************/

uint8_t flyEraserGtk::upload (void)
{
    lock++;

    gtk_entry_set_text (GTK_ENTRY(WID(eraserDataFileEntry)),
                        param.data_file.c_str());

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(colorSpinner)), param.output_color);

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(debugSpinButton)), param.debug);

    lock--;

    return upload_masklist (true);
}

uint8_t flyEraserGtk::upload_masklist (bool set_preview_frame)
{
    lock++;

    Eraser::MaskVec & masks = eraserp->getMasks();

    gtk_tree_view_set_model (tree_view, NULL);

    // There may be a more efficient way, but for the likely numbers of rows
    // we'll be dealing with, I don't know that it's worth the trouble to
    // implement it.
    gtk_list_store_clear (list_store);

    for (Eraser::MaskVec::const_iterator maskit = masks.begin();
         maskit != masks.end();
         ++maskit)
    {
        const Eraser::Mask & mask = *maskit;

        GtkTreeIter iter;
        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, mask.first_frame,
                            1, mask.last_frame, -1);
//        printf ("uploaded mask %d (%d - %d) to treeview\n",
//                maskit - masks.begin(), mask.first_frame, mask.last_frame);
    }

    gtk_tree_view_set_model (tree_view, tree_model);

    setSelectionNumber (masks.size(), GTK_WIDGET (tree_view), list_store,
                        current_mask - masks.begin());

    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(frameRangeFirstSpinButton)),
         current_mask->first_frame);
    gtk_spin_button_set_value
        (GTK_SPIN_BUTTON(WID(frameRangeLastSpinButton)),
         current_mask->last_frame);

    if (set_preview_frame)
    {
        uint32_t middle_frame = (current_mask->first_frame
                                 + current_mask->last_frame) / 2;
        if (middle_frame >= _in->getInfo()->nb_frames)
            middle_frame = current_mask->first_frame;
        sliderSet (middle_frame);
        sliderChanged();
    }

    lock--;
    return 1;
}

uint8_t flyEraserGtk::download (void)
{
    getMenuValues();

    const char * filename =
        gtk_entry_get_text (GTK_ENTRY(WID(eraserDataFileEntry)));

    // Note that the documentation states clearly that the result of
    // gtk_entry_get_text() must NOT be freed, modified, or stored.  This is
    // different than gtk_file_chooser_get_filename(), which returns a pointer
    // which must eventually be passed to g_free().

    if (filename)
        param.data_file = filename;
    else
        param.data_file = "";

    param.output_color
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(colorSpinner)));

    param.debug
        = gtk_spin_button_get_value_as_int
        (GTK_SPIN_BUTTON(WID(debugSpinButton)));

    return 1;
}

void flyEraserGtk::wipeOutputBuffer ()
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

// The following was generated by glade from eraser.glade once upon a time (in
// fact, several times :-).  Thus far, I've avoided editing it, so
// regenerating the code from Glade (I used 2.12.1) and swapping it in here
// should not lose anything.  All the tweaking and adjusting and
// signal-connecting is done in code above, which is maintained by hand, and
// which has much knowledge of the names and layout of the widgets (so be
// cautious about renaming things!).

GtkWidget*
create_eraser_dialog (void)
{
  GtkWidget *eraser_dialog;
  GtkWidget *dialogVbox;
  GtkWidget *dialogHbox;
  GtkWidget *allSettingsVbox;
  GtkWidget *settingsOuterHbox;
  GtkWidget *settingsOuterVbox;
  GtkWidget *brushSettingsHbox;
  GtkWidget *brushModeHbox;
  GtkWidget *brushModeLabel;
  GtkWidget *brushModeMenu;
  GtkWidget *brushSizeHbox;
  GtkWidget *brushSizeLabel;
  GtkWidget *brushSizeMenu;
  GtkWidget *colorVbox;
  GtkWidget *colorLabel;
  GtkWidget *colorHbox;
  GtkWidget *colorSlider;
  GtkObject *colorSpinner_adj;
  GtkWidget *colorSpinner;
  GtkWidget *eraserDataFileHbox;
  GtkWidget *eraserDataFileLabel;
  GtkWidget *eraserDataFileEntry;
  GtkWidget *debugHbox;
  GtkWidget *eraserDataFileBrowseButton;
  GtkWidget *browse_debug_spacer;
  GtkWidget *debugLabel;
  GtkObject *debugSpinButton_adj;
  GtkWidget *debugSpinButton;
  GtkWidget *frameRangeHbox;
  GtkWidget *frameRangeStartHbox;
  GtkWidget *frameRangeStartLabel;
  GtkObject *frameRangeFirstSpinButton_adj;
  GtkWidget *frameRangeFirstSpinButton;
  GtkWidget *frameRangeLastHbox;
  GtkWidget *frameRangeLastLabel;
  GtkObject *frameRangeLastSpinButton_adj;
  GtkWidget *frameRangeLastSpinButton;
  GtkWidget *rangeListScrolledWindow;
  GtkWidget *rangeListTreeview;
  GtkWidget *rangeListHButtonBox;
  GtkWidget *insertButton;
  GtkWidget *duplicateButton;
  GtkWidget *deleteButton;
  GtkWidget *previewVboxOuter;
  GtkWidget *previewJumpButtonHbox;
  GtkWidget *previewJumpLastButton;
  GtkWidget *previewJumpFirstButton;
  GtkWidget *previewJumpLabel;
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
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  eraser_dialog = gtk_dialog_new ();
  //                                                NO NEED TO "FIX" THAT _("...")!!
  //                                                see handy macros near top of file.
  gtk_window_set_title (GTK_WINDOW (eraser_dialog), _("Eraser Configuration"));
  gtk_window_set_type_hint (GTK_WINDOW (eraser_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialogVbox = GTK_DIALOG (eraser_dialog)->vbox;
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
  gtk_box_pack_start (GTK_BOX (allSettingsVbox), settingsOuterHbox, TRUE, TRUE, 0);

  settingsOuterVbox = gtk_vbox_new (FALSE, 12);
  gtk_widget_show (settingsOuterVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterHbox), settingsOuterVbox, TRUE, TRUE, 0);

  brushSettingsHbox = gtk_hbox_new (FALSE, 15);
  gtk_widget_show (brushSettingsHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), brushSettingsHbox, FALSE, TRUE, 0);

  brushModeHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (brushModeHbox);
  gtk_box_pack_start (GTK_BOX (brushSettingsHbox), brushModeHbox, TRUE, TRUE, 0);

  brushModeLabel = gtk_label_new_with_mnemonic (_("Brush _Mode:  "));
  gtk_widget_show (brushModeLabel);
  gtk_box_pack_start (GTK_BOX (brushModeHbox), brushModeLabel, FALSE, FALSE, 0);

  brushModeMenu = gtk_combo_box_new_text ();
  gtk_widget_show (brushModeMenu);
  gtk_box_pack_start (GTK_BOX (brushModeHbox), brushModeMenu, TRUE, TRUE, 0);

  brushSizeHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (brushSizeHbox);
  gtk_box_pack_start (GTK_BOX (brushSettingsHbox), brushSizeHbox, TRUE, TRUE, 0);

  brushSizeLabel = gtk_label_new_with_mnemonic (_("Brush _Size:  "));
  gtk_widget_show (brushSizeLabel);
  gtk_box_pack_start (GTK_BOX (brushSizeHbox), brushSizeLabel, FALSE, FALSE, 0);

  brushSizeMenu = gtk_combo_box_new_text ();
  gtk_widget_show (brushSizeMenu);
  gtk_box_pack_start (GTK_BOX (brushSizeHbox), brushSizeMenu, TRUE, TRUE, 0);

  colorVbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (colorVbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), colorVbox, FALSE, TRUE, 0);

  colorLabel = gtk_label_new_with_mnemonic (_("Output \"_Color\" for all masked pixels:"));
  gtk_widget_show (colorLabel);
  gtk_box_pack_start (GTK_BOX (colorVbox), colorLabel, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (colorLabel), 0, 0.5);

  colorHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (colorHbox);
  gtk_box_pack_start (GTK_BOX (colorVbox), colorHbox, TRUE, TRUE, 0);

  colorSlider = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 255, 1, 1, 0)));
  gtk_widget_show (colorSlider);
  gtk_box_pack_start (GTK_BOX (colorHbox), colorSlider, TRUE, TRUE, 0);
  gtk_scale_set_draw_value (GTK_SCALE (colorSlider), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (colorSlider), GTK_POS_LEFT);
  gtk_scale_set_digits (GTK_SCALE (colorSlider), 0);

  colorSpinner_adj = gtk_adjustment_new (0, 0, 255, 1, 1, 0);
  colorSpinner = gtk_spin_button_new (GTK_ADJUSTMENT (colorSpinner_adj), 1, 0);
  gtk_widget_show (colorSpinner);
  gtk_box_pack_start (GTK_BOX (colorHbox), colorSpinner, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (colorSpinner), TRUE);

  eraserDataFileHbox = gtk_hbox_new (FALSE, 5);
  gtk_widget_show (eraserDataFileHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), eraserDataFileHbox, FALSE, TRUE, 0);

  eraserDataFileLabel = gtk_label_new_with_mnemonic (_("Eraser _Data File:"));
  gtk_widget_show (eraserDataFileLabel);
  gtk_box_pack_start (GTK_BOX (eraserDataFileHbox), eraserDataFileLabel, FALSE, FALSE, 0);

  eraserDataFileEntry = gtk_entry_new ();
  gtk_widget_show (eraserDataFileEntry);
  gtk_box_pack_start (GTK_BOX (eraserDataFileHbox), eraserDataFileEntry, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, eraserDataFileEntry, _("File in which eraser mask information (the list below and the pixels erased for each range) will be stored"), NULL);
  gtk_entry_set_invisible_char (GTK_ENTRY (eraserDataFileEntry), 8226);
  gtk_entry_set_width_chars (GTK_ENTRY (eraserDataFileEntry), 35);

  debugHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (debugHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), debugHbox, FALSE, TRUE, 0);

  eraserDataFileBrowseButton = gtk_button_new_with_mnemonic (_("_Browse..."));
  gtk_widget_show (eraserDataFileBrowseButton);
  gtk_box_pack_start (GTK_BOX (debugHbox), eraserDataFileBrowseButton, FALSE, FALSE, 0);

  browse_debug_spacer = gtk_label_new ("");
  gtk_widget_show (browse_debug_spacer);
  gtk_box_pack_start (GTK_BOX (debugHbox), browse_debug_spacer, TRUE, FALSE, 0);

  debugLabel = gtk_label_new_with_mnemonic (_("_Debugging settings (bits):   "));
  gtk_widget_show (debugLabel);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugLabel, FALSE, FALSE, 0);

  debugSpinButton_adj = gtk_adjustment_new (0, 0, 16777215, 1, 10, 0);
  debugSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (debugSpinButton_adj), 1, 0);
  gtk_widget_show (debugSpinButton);
  gtk_box_pack_start (GTK_BOX (debugHbox), debugSpinButton, FALSE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (debugSpinButton), TRUE);

  frameRangeHbox = gtk_hbox_new (FALSE, 32);
  gtk_widget_show (frameRangeHbox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), frameRangeHbox, FALSE, TRUE, 0);

  frameRangeStartHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (frameRangeStartHbox);
  gtk_box_pack_start (GTK_BOX (frameRangeHbox), frameRangeStartHbox, TRUE, TRUE, 0);

  frameRangeStartLabel = gtk_label_new_with_mnemonic (_("_First frame:   "));
  gtk_widget_show (frameRangeStartLabel);
  gtk_box_pack_start (GTK_BOX (frameRangeStartHbox), frameRangeStartLabel, FALSE, FALSE, 0);

  frameRangeFirstSpinButton_adj = gtk_adjustment_new (0, 0, 100000000, 1, 10, 0);
  frameRangeFirstSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (frameRangeFirstSpinButton_adj), 1, 0);
  gtk_widget_show (frameRangeFirstSpinButton);
  gtk_box_pack_start (GTK_BOX (frameRangeStartHbox), frameRangeFirstSpinButton, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, frameRangeFirstSpinButton, _("First frame to which currently selected eraser mask applies; 0 means first frame of video"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (frameRangeFirstSpinButton), TRUE);

  frameRangeLastHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (frameRangeLastHbox);
  gtk_box_pack_start (GTK_BOX (frameRangeHbox), frameRangeLastHbox, TRUE, TRUE, 0);

  frameRangeLastLabel = gtk_label_new_with_mnemonic (_("_Last frame:   "));
  gtk_widget_show (frameRangeLastLabel);
  gtk_box_pack_start (GTK_BOX (frameRangeLastHbox), frameRangeLastLabel, FALSE, FALSE, 0);

  frameRangeLastSpinButton_adj = gtk_adjustment_new (100000000, 0, 100000000, 1, 10, 0);
  frameRangeLastSpinButton = gtk_spin_button_new (GTK_ADJUSTMENT (frameRangeLastSpinButton_adj), 1, 0);
  gtk_widget_show (frameRangeLastSpinButton);
  gtk_box_pack_start (GTK_BOX (frameRangeLastHbox), frameRangeLastSpinButton, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, frameRangeLastSpinButton, _("Last frame to which currently selected eraser mask applies; use e.g. 99999999 to represent the last frame of video"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (frameRangeLastSpinButton), TRUE);

  rangeListScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (rangeListScrolledWindow);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), rangeListScrolledWindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (rangeListScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (rangeListScrolledWindow), GTK_SHADOW_IN);

  rangeListTreeview = gtk_tree_view_new ();
  gtk_widget_show (rangeListTreeview);
  gtk_container_add (GTK_CONTAINER (rangeListScrolledWindow), rangeListTreeview);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (rangeListTreeview), TRUE);
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (rangeListTreeview), FALSE);

  rangeListHButtonBox = gtk_hbutton_box_new ();
  gtk_widget_show (rangeListHButtonBox);
  gtk_box_pack_start (GTK_BOX (settingsOuterVbox), rangeListHButtonBox, FALSE, TRUE, 0);

  insertButton = gtk_button_new_from_stock ("gtk-new");
  gtk_widget_show (insertButton);
  gtk_container_add (GTK_CONTAINER (rangeListHButtonBox), insertButton);
  GTK_WIDGET_SET_FLAGS (insertButton, GTK_CAN_DEFAULT);
  gtk_button_set_focus_on_click (GTK_BUTTON (insertButton), FALSE);

  duplicateButton = gtk_button_new_with_mnemonic (_("Duplicate"));
  gtk_widget_show (duplicateButton);
  gtk_container_add (GTK_CONTAINER (rangeListHButtonBox), duplicateButton);
  GTK_WIDGET_SET_FLAGS (duplicateButton, GTK_CAN_DEFAULT);
  gtk_tooltips_set_tip (tooltips, duplicateButton, _("Make a copy of the currently selected eraser mask and insert it as the following row"), NULL);
  gtk_button_set_focus_on_click (GTK_BUTTON (duplicateButton), FALSE);

  deleteButton = gtk_button_new_from_stock ("gtk-delete");
  gtk_widget_show (deleteButton);
  gtk_container_add (GTK_CONTAINER (rangeListHButtonBox), deleteButton);
  GTK_WIDGET_SET_FLAGS (deleteButton, GTK_CAN_DEFAULT);
  gtk_button_set_focus_on_click (GTK_BUTTON (deleteButton), FALSE);

  previewVboxOuter = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (previewVboxOuter);
  gtk_box_pack_start (GTK_BOX (dialogHbox), previewVboxOuter, FALSE, TRUE, 0);

  previewJumpButtonHbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (previewJumpButtonHbox);
  gtk_box_pack_start (GTK_BOX (previewVboxOuter), previewJumpButtonHbox, FALSE, TRUE, 5);

  previewJumpLastButton = gtk_button_new_with_mnemonic (_("Last Frame in Range"));
  gtk_widget_show (previewJumpLastButton);
  gtk_box_pack_end (GTK_BOX (previewJumpButtonHbox), previewJumpLastButton, FALSE, FALSE, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (previewJumpLastButton), FALSE);

  previewJumpFirstButton = gtk_button_new_with_mnemonic (_("First Frame in Range"));
  gtk_widget_show (previewJumpFirstButton);
  gtk_box_pack_end (GTK_BOX (previewJumpButtonHbox), previewJumpFirstButton, FALSE, FALSE, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (previewJumpFirstButton), FALSE);

  previewJumpLabel = gtk_label_new (_("Jump to:  "));
  gtk_widget_show (previewJumpLabel);
  gtk_box_pack_end (GTK_BOX (previewJumpButtonHbox), previewJumpLabel, FALSE, FALSE, 0);

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
  gtk_widget_set_events (previewVideo, GDK_BUTTON1_MOTION_MASK | GDK_BUTTON2_MOTION_MASK | GDK_BUTTON3_MOTION_MASK | GDK_BUTTON_PRESS_MASK);

  previewLabel = gtk_label_new (_("Preview"));
  gtk_widget_show (previewLabel);
  gtk_frame_set_label_widget (GTK_FRAME (previewFrame), previewLabel);

  dialogButtonBox = GTK_DIALOG (eraser_dialog)->action_area;
  gtk_widget_show (dialogButtonBox);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialogButtonBox), GTK_BUTTONBOX_END);

  cancelButton = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (eraser_dialog), cancelButton, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelButton, GTK_CAN_DEFAULT);

  okButton = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okButton);
  gtk_dialog_add_action_widget (GTK_DIALOG (eraser_dialog), okButton, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okButton, GTK_CAN_DEFAULT);

  gtk_label_set_mnemonic_widget (GTK_LABEL (eraserDataFileLabel), eraserDataFileEntry);
  gtk_label_set_mnemonic_widget (GTK_LABEL (debugLabel), debugSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (frameRangeStartLabel), debugSpinButton);
  gtk_label_set_mnemonic_widget (GTK_LABEL (frameRangeLastLabel), debugSpinButton);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (eraser_dialog, eraser_dialog, "eraser_dialog");
  GLADE_HOOKUP_OBJECT_NO_REF (eraser_dialog, dialogVbox, "dialogVbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, dialogHbox, "dialogHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, allSettingsVbox, "allSettingsVbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, settingsOuterHbox, "settingsOuterHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, settingsOuterVbox, "settingsOuterVbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushSettingsHbox, "brushSettingsHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushModeHbox, "brushModeHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushModeLabel, "brushModeLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushModeMenu, "brushModeMenu");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushSizeHbox, "brushSizeHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushSizeLabel, "brushSizeLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, brushSizeMenu, "brushSizeMenu");
  GLADE_HOOKUP_OBJECT (eraser_dialog, colorVbox, "colorVbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, colorLabel, "colorLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, colorHbox, "colorHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, colorSlider, "colorSlider");
  GLADE_HOOKUP_OBJECT (eraser_dialog, colorSpinner, "colorSpinner");
  GLADE_HOOKUP_OBJECT (eraser_dialog, eraserDataFileHbox, "eraserDataFileHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, eraserDataFileLabel, "eraserDataFileLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, eraserDataFileEntry, "eraserDataFileEntry");
  GLADE_HOOKUP_OBJECT (eraser_dialog, debugHbox, "debugHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, eraserDataFileBrowseButton, "eraserDataFileBrowseButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, browse_debug_spacer, "browse_debug_spacer");
  GLADE_HOOKUP_OBJECT (eraser_dialog, debugLabel, "debugLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, debugSpinButton, "debugSpinButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeHbox, "frameRangeHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeStartHbox, "frameRangeStartHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeStartLabel, "frameRangeStartLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeFirstSpinButton, "frameRangeFirstSpinButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeLastHbox, "frameRangeLastHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeLastLabel, "frameRangeLastLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, frameRangeLastSpinButton, "frameRangeLastSpinButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, rangeListScrolledWindow, "rangeListScrolledWindow");
  GLADE_HOOKUP_OBJECT (eraser_dialog, rangeListTreeview, "rangeListTreeview");
  GLADE_HOOKUP_OBJECT (eraser_dialog, rangeListHButtonBox, "rangeListHButtonBox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, insertButton, "insertButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, duplicateButton, "duplicateButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, deleteButton, "deleteButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewVboxOuter, "previewVboxOuter");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewJumpButtonHbox, "previewJumpButtonHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewJumpLastButton, "previewJumpLastButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewJumpFirstButton, "previewJumpFirstButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewJumpLabel, "previewJumpLabel");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewFrame, "previewFrame");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewAlignment, "previewAlignment");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewVbox, "previewVbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewControlHbox, "previewControlHbox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewOutputMenu, "previewOutputMenu");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewSlider, "previewSlider");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewVideo, "previewVideo");
  GLADE_HOOKUP_OBJECT (eraser_dialog, previewLabel, "previewLabel");
  GLADE_HOOKUP_OBJECT_NO_REF (eraser_dialog, dialogButtonBox, "dialogButtonBox");
  GLADE_HOOKUP_OBJECT (eraser_dialog, cancelButton, "cancelButton");
  GLADE_HOOKUP_OBJECT (eraser_dialog, okButton, "okButton");
  GLADE_HOOKUP_OBJECT_NO_REF (eraser_dialog, tooltips, "tooltips");

  return eraser_dialog;
}

#endif

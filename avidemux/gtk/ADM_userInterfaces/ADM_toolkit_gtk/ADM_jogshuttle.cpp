// $Id: jogshuttle.cc,v 1.22 2007/02/20 06:32:05 ddennedy Exp $
/*
 * Copyright (C) 2001 Tomoaki Hayasaka <hayasakas@postman.riken.go.jp>
 * Copyright (C) 2001-2007 Dan Dennedy <dan@dennedy.org>
 * Taken from Kino 1.1.1, with permission, and hacked severely by
 * Chris MacGregor <chris-avidemux@bouncingdog.dom>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/// Support for special USB Jog/Shuttle input controllers

// TODO:
// * What happens for someone who has a device not listed in the
//   supported_devices[] table in mediactrl.c?  They should at least be able
//   to try pretending that it is one of the supported devices, in case that
//   works, and really they should be able to specify various parameters in a
//   config file of some sort (device code and name; key mappings; input
//   ranges (min/max wheel range, etc.); which translation function to use
//   (contour or compliant)).
// * There should be a mode or something where the user can explore their
//   device (see what code is returned for each button, test the value ranges,
//   etc.) so they have the data for the config file.
// * Button mapping: the ideal would be a dialog where they can match buttons
//   to actions, and then save it to a config file.  I don't have time to
//   implement that now.  The next best thing would be just the config file,
//   and they have to edit it directly.  Maybe I can do that...
#if 0
#include "config.h"

#ifdef USE_JOG
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <string>
using std::string;
#include <iostream>
#include <fstream>
using std::ifstream;
#include <errno.h>

#include "gui_action.hxx"
#include "jogshuttle.h"
#include "ADM_jogshuttle.h"
#include "ADM_image.h"

/// PhysicalJogShuttle

/** The global PhysicalJogShuttle object.
	 
    It is accessible to other classes via the getInstance method.
*/

PhysicalJogShuttle * PhysicalJogShuttle::_instance = NULL;

/** Singleton getInstance method. Standard pattern. Non-threadsafe, but
    first instance gets called from the commands.cc which should be OK.
*/

PhysicalJogShuttle & PhysicalJogShuttle::getInstance()
{
    if (_instance == 0)
        _instance = new PhysicalJogShuttle();
    return *_instance;
}

/** Constructor
 */
PhysicalJogShuttle::PhysicalJogShuttle() :
    input_ (-1),
    monitorTag_ (-1),
    _modifier_code (0),
    show_buttons (getenv ("SHOW_BUTTONS") != 0)
{
    _ctrl.device = 0;
    if (!start())
        return;

    const char * home = getenv ("HOME");
    if (!home)
        return;

    string buttonfile = home;
    buttonfile += "/.avidemux/button_mapping";
    const char * filename = buttonfile.c_str();
    ifstream inputStream (filename);
    if (!inputStream)
    {
        if (errno != ENOENT)
            perror (filename);
        return;
    }

    printf ("Reading jog shuttle device button mappings from %s:\n",
            filename);

    int linenum = 0;
    while (inputStream)
    {
        const int BUFFER_SIZE = 100;
        char buffer [BUFFER_SIZE];
        inputStream.getline (buffer, BUFFER_SIZE);
        char * cptr = buffer;
        ++linenum;
        while (isspace (*cptr))
            ++cptr;
        if (*cptr == '\0' || *cptr == '#')
            continue;
        if (strncmp (cptr, "help", 4) == 0)
        {
            dumpActionNames (filename);
            continue;
        }
        if (!isdigit (*cptr))
        {
            printf ("%s line %d: expecting a button number first, "
                    "found \"%s\"\n", filename, linenum, cptr);
            continue;
        }
        int button_num = atoi (cptr);
        if (button_num == 0)
        {
            printf ("%s line %d: %d is not a valid button number\n",
                    filename, linenum, button_num);
            continue;
        }
        while (isdigit (*cptr))
            ++cptr;
        while (isspace (*cptr))
            ++cptr;
        if (!isalpha (*cptr))
        {
            printf ("%s line %d: expecting an action name following button "
                    "number, found \"%s\"\n", filename, linenum, cptr);
            continue;
        }
        const char * name = cptr;
        while (isalnum (*cptr) || *cptr == '_')
            ++cptr;
        char * name_end = cptr;
        while (isspace (*cptr))
            ++cptr;
        if (*cptr)
            printf ("%s line %d: ignoring unexpected stuff (\"%s\") "
                    "following action name\n", filename, linenum, cptr);
        *name_end = '\0';

        Action act = lookupActionByName (name);
        if (act == ACT_INVALID)
        {
            if (strcasecmp (name, "help") == 0)
                dumpActionNames (filename);
            else
                printf ("%s line %d: %s is not a valid action name "
                        "(try \"help\")\n", filename, linenum, name);
        }
        else
        {
            // printf ("mapping button %d to action %s (%d)\n",
            printf ("mapping button %d to action %s\n",
                    button_num, getActionName (act), act);
            button_action_map [button_num] = act;
        }
    }
}

/** Destructor
 
    Remove the callback function from the GDK input handler.
*/
PhysicalJogShuttle::~PhysicalJogShuttle ()
{
    stop();
    if (input_ >= 0)
    {
        gdk_input_remove (monitorTag_);
        media_ctrl_close (&_ctrl);
    }
}

void PhysicalJogShuttle::NoButtonCB (void *, unsigned short, Action)
{
}

void PhysicalJogShuttle::NoJogDialCB (void *, signed short)
{
}

void PhysicalJogShuttle::NoShuttleRingCB (void *, gfloat)
{
}

/// A static wrappper for the GDK input handler callback
void PhysicalJogShuttle::inputCallback (gpointer data, gint source,
                                        GdkInputCondition condition)
{
    PhysicalJogShuttle * js = static_cast <PhysicalJogShuttle *> (data);
    g_return_if_fail (js != NULL);
    js->inputCallback (source, condition);
}

/** Start the GDK input handler if enabled.

    \return a boolean indicating if not started due to not enabled or device
    open failed.
*/
bool PhysicalJogShuttle::start ()
{
    stop();
    media_ctrl_open (&_ctrl);
    if (_ctrl.device) 
    {
        monitorTag_ = gdk_input_add (_ctrl.fd, GDK_INPUT_READ,
                                     inputCallback, (gpointer) this);
        printf ("Physical Jog/Shuttle device enabled.\n");
        return true;
    }

    printf ("No physical Jog/Shuttle device found.\n");
    return false;
}

/** Stop the GDK input handler.
 */
void PhysicalJogShuttle::stop()
{
    if (monitorTag_ != -1)
    {
        gdk_input_remove (monitorTag_);
        monitorTag_ = -1;
    }
    if (_ctrl.device)
        media_ctrl_close (&_ctrl);
}

/** Register callback
 
    Register an interest in getting notification when buttons are pressed or
    the jog dial or shuttle ring are manipulated
*/
void PhysicalJogShuttle::registerCBs (void * their_data,
                                      ButtonCallback button_cb,
                                      JogDialCallback jogdial_cb,
                                      ShuttleRingCallback shuttlering_cb)
{
    cbstack.push (Callbacks (their_data, button_cb, jogdial_cb, shuttlering_cb));
}

/** Deregister callback
 
    Deregister an interest in getting notification when buttons are pressed
*/
void PhysicalJogShuttle::deregisterCBs (void * their_data)
{
    if (cbstack.empty())
        g_warning( "PhysicalJogShuttle::deregister (callback) - none registered\n");
    else
    {
        // if the data doesn't match, we could in theory search down the stack
        // to see if someone skipped a deregister() call.  But more than one
        // may have passed NULL as the data...
        if (their_data != cbstack.top().their_data)
            g_warning( "PhysicalJogShuttle::deregister (callback) - data doesn't match!!\n");
        cbstack.pop();
    }
}

struct media_ctrl_key *PhysicalJogShuttle::getKeyset ()
{
    if (_ctrl.device != 0)
        return _ctrl.device->keys;
    else
        return 0;
}


/** Handle movement on the jog dial.
 
    \param dir A number from -x to x to specify the offset from the current frame.
    A negative number moves backward. Typically, x is 1 to step frame-by-frame.
*/
void PhysicalJogShuttle::jog (int offs)
{
    gdk_threads_enter();
    // printf ("jog %d\n", offs);

    if (!cbstack.empty())
    {
        Callbacks & cb = cbstack.top();
        if (cb.jogdial_cb)
            cb.jogdial_cb (cb.their_data, offs);
    }

    gdk_threads_leave();
}

/** Handle movement of the shuttle ring.
 
    \param angle A number from -1.0 to +1.0 that specifies a direction and speed.
*/
void PhysicalJogShuttle::shuttle (gfloat angle)
{
    gdk_threads_enter();
    // printf ("shuttle %f\n", angle);

    if (!cbstack.empty())
    {
        Callbacks & cb = cbstack.top();
        if (cb.shuttlering_cb)
            cb.shuttlering_cb (cb.their_data, angle);
    }

    gdk_threads_leave();
}

/** Handle key press
 
    \param ev The key event
    Modifier keys - we maintain the state of the first key that is
    pressed down. This is a "poor man"s way of doing modifier
    keys. It means that we can press a single key, keep it down, and
    press a number of others. Any key can be used as a modifier key,
    although you would probably want to not assign an action to a
    modifier key when first pressed. There is no way to react to multiple
    modifier keys though.

*/
void PhysicalJogShuttle::button (struct media_ctrl_event *ev)
{
    /* Figure out what codes to use */
    unsigned short first;
    unsigned short second;

    // Note: the modifier stuff is carried over from kino, but not currently
    // in use here.  I've left it here just in case someone decides to enable
    // it some day.  However, it doesn't seem to work the way I would expect.
    // Probably the best thing would be to implement full modifier support
    // wherein the standard keyboard modifiers work (Shift, Control, Alt,
    // Meta, etc.) and (ideally) you can also select buttons on the device to
    // act as true modifiers (that is, they work like the
    // Shift/Control/etc. keys), rather than the hackish thing that is
    // half-implemented here.  Or maybe it's easier on the user to have it be
    // a button-sequence thing where you hit the "modifier" and then the other
    // button, and don't have to hold one down.  I suppose the best thing
    // would be to give the user the option...

    /* 
       Release may need to clear a modifier key
    */
    if (ev->value == KEY_RELEASE && _modifier_code != 0)
    {
        _modifier_code = 0;
        return;
    }

    /* This is a key press - if there are no modifiers, make sure that
       this is saved */
    if (_modifier_code == 0)
        _modifier_code = ev->code;

    if (_modifier_code != ev->code)
    {
        first = _modifier_code;
        second = ev->code;
    }
    else
    {
        first = ev->code; /* Same as modifier */
        second = 0;
    }
		
    gdk_threads_enter();

    ButtonActionIter it = button_action_map.find (first);
    Action actn;
    const char * name;
    if (it == button_action_map.end())
    {
        actn = ACT_INVALID;
        name = "(not mapped)";
    }
    else
    {
        actn = it->second;
        name = getActionName (actn);
    }

    if (show_buttons)
        printf ("button pressed: %d -> %d: %s\n", first, actn, name);

    if (!cbstack.empty())
    {
        Callbacks & cb = cbstack.top();
        if (cb.button_cb)
            cb.button_cb (cb.their_data, first, actn);
    }

    gdk_threads_leave();
}


/** The GDK input callback function.
 
    GDK calls this whenever input is received. It is hooked into the system
    during object construction.
*/
void PhysicalJogShuttle::inputCallback (gint source, GdkInputCondition condition)
{
    g_return_if_fail (this != 0);
    if (condition != GDK_INPUT_READ)
        stop();
    g_return_if_fail (condition == GDK_INPUT_READ);

    struct media_ctrl_event ev;
	
    ev.type = MEDIA_CTRL_EVENT_NONE;
    media_ctrl_read_event (&_ctrl, &ev);

    /* We can get four "kinds" of events:

       ev.type == MEDIA_CTRL_EVENT_NONE (0x00)
       An event that actually isn't one... Since we read every event
       from the input subsystem, we sometimes get double events 
       (e.g. the shuttlepro reports its state approx. every second.)
       libmediactrl translates these events to none-events.
	
	
       ev.type == MEDIA_CTRL_EVENT_KEY (0x01)
       This is a button press. The whole event will be handled in the
       button method..

       ev.type == MEDIA_CTRL_EVENT_JOG (0x02)
       This event is issued when ever the dial position changes. 
       ev.value is the offset to the current jog position.


       - ev.code == MEDIA_CTRL_EVENT_SHUTTLE (0x02) 
       the outer wheel ev.value indicates the position, with positive 
       values from 0x01 to 0x0f (inclusive) indicating clockwise twist, 
       and negative values from -1 to -15 indicating counterclockwise 
       twist.  A 0 value is reported. 

    */

    if (ev.type == MEDIA_CTRL_EVENT_NONE)
        return;
#if 0
    printf ("PhysicalJogShuttle: %02x %02x %02d\n", ev.type, ev.code, ev.value);
#endif

    if (ev.type == MEDIA_CTRL_EVENT_JOG) 
    {
        this->jog (ev.value);
    } 
    else if (ev.type == MEDIA_CTRL_EVENT_SHUTTLE)
    {
        this->shuttle (ev.fvalue);
    }
    else if (ev.type == MEDIA_CTRL_EVENT_KEY)
    {
        this->button (&ev);
    }
    else
    {
        return;
    }
}
#endif
#endif
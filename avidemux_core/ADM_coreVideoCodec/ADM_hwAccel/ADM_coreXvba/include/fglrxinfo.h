/*
 *  fglrxinfo.h - FGLRX driver info
 *
 *  xvba-video (C) 2009-2011 Splitted-Desktop Systems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef FGLRXINFO_H
#define FGLRXINFO_H

#include <X11/Xlib.h>



Bool fglrx_is_dri_capable(Display *dpy, int screen);
   // attribute_hidden;

Bool fglrx_get_version(
    Display    *dpy,
    int         screen,
    int        *ddxDriverMajorVersion,
    int        *ddxDriverMinorVersion,
    int        *ddxDriverPatchVersion
);// attribute_hidden;

int fglrx_check_version(int major, int minor, int micro);
    //attribute_hidden;

Bool fglrx_get_device_id(
    Display      *dpy,
    int           screen,
    unsigned int *deviceID
); //attribute_hidden;

#endif /* FGLRXINFO_H */

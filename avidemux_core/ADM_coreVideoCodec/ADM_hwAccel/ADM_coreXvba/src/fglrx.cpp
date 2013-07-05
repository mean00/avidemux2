/*
 *  fglrxinfo.c - FGLRX driver info
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

//#include "sysdeps.h"
#define NEED_REPLIES
extern "C"
{
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
}
#include "../include/fglrxinfo.h"

#define ATIFGL_EXTENSION_NAME "ATIFGLEXTENSION"
#define ATIFGL_EXTENSION_EVENTS 0

typedef struct _FGLGetDriverData {
    CARD8   reqType;
    CARD8   fireglReqType;
    CARD16  length B16;
    CARD32  screen B32;
    CARD16  size B16;
    CARD16  pad1;
} xFGLGetDriverDataReq;
#define sz_xFGLGetDriverDataReq sizeof(xFGLGetDriverDataReq)

typedef struct {
    BYTE    type;
    BYTE    pad1;
    CARD16  sequenceNumber B16;
    CARD32  length B32;
    CARD8   majorVersion;
    CARD8   minorVersion;
    CARD8   patchlevel B16;
    CARD8   BIOSVersionMajor;
    CARD8   BIOSVersionMinor;
    CARD8   HasSecondary;
    CARD16  pad3 B16;
    CARD16  pad4 B16;
    CARD16  deviceID B16;
    CARD32  pad5 B32;
    CARD32  pad6 B32;
    CARD32  pad7 B32;
    // ... there are more fields
} xFGLGetDriverDataReply;
#define sz_xFGLGetDriverDataReply sizeof(xFGLGetDriverDataReply)

#define X_FGLGetDriverData 0

static XExtensionInfo _fglext_ext_info_data;
static XExtensionInfo *fglext_ext_info = &_fglext_ext_info_data;
static /* const */ char *fglext_extension_name = ATIFGL_EXTENSION_NAME;

#define xFGLCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, fglext_extension_name, val)

static int close_display(Display *dpy, XExtCodes *codes);
static /* const */ XExtensionHooks fglext_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    close_display,                      /* close_display */
    NULL,                               /* wire_to_event */
    NULL,                               /* event_to_wire */
    NULL,                               /* error */
    NULL,                               /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, fglext_ext_info,
                                   fglext_extension_name, 
                                   &fglext_extension_hooks,
                                   ATIFGL_EXTENSION_EVENTS, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, fglext_ext_info)

Bool fglrx_is_dri_capable(Display *dpy, int screen)
{
    char **extensions;
    int i, n_extensions, has_fglext = 0, has_fglrxdri = 0;

    extensions = XListExtensions(dpy, &n_extensions);
    if (!extensions)
        return False;

    for (i = 0; i < n_extensions; i++) {
        if (strcmp(extensions[i], ATIFGL_EXTENSION_NAME) == 0)
            has_fglext = 1;
        if (strcmp(extensions[i], "ATIFGLRXDRI") == 0)
            has_fglrxdri = 1;
    }
    XFreeExtensionList(extensions);
    return has_fglext && has_fglrxdri;
}

Bool fglrx_get_version(
    Display    *dpy,
    int         screen,
    int        *ddxDriverMajorVersion,
    int        *ddxDriverMinorVersion,
    int        *ddxDriverPatchVersion
)
{
    XExtDisplayInfo *info = find_display (dpy);
    xFGLGetDriverDataReply rep;
    xFGLGetDriverDataReq  *req;

    if (ddxDriverMajorVersion)
        *ddxDriverMajorVersion = 0;
    if (ddxDriverMinorVersion)
        *ddxDriverMinorVersion = 0;
    if (ddxDriverPatchVersion)
        *ddxDriverPatchVersion = 0;

    if(!XextHasExtension(info))
        return False;

    xFGLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (FGLGetDriverData, req);
    req->reqType = info->codes->major_opcode;
    req->fireglReqType = X_FGLGetDriverData;
    req->screen = screen;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    UnlockDisplay (dpy);
    SyncHandle ();

    if (ddxDriverMajorVersion)
        *ddxDriverMajorVersion = rep.majorVersion;
    if (ddxDriverMinorVersion)
        *ddxDriverMinorVersion = rep.minorVersion;
    if (ddxDriverPatchVersion)
        *ddxDriverPatchVersion = rep.patchlevel;

    return True;
}

int fglrx_check_version(int major, int minor, int micro)
{
    static int is_initialized = -1;
    static int fglrx_version_major, fglrx_version_minor, fglrx_version_micro;

    if (is_initialized < 0) {
        Display *dpy = XOpenDisplay(NULL);
        if (!dpy)
            is_initialized = 0;
        else {
            is_initialized = fglrx_get_version(dpy, DefaultScreen(dpy),
                                               &fglrx_version_major,
                                               &fglrx_version_minor,
                                               &fglrx_version_micro);
            XCloseDisplay(dpy);
        }
    }

    return (is_initialized &&
            ((fglrx_version_major > major) ||
             (fglrx_version_major == major &&
              fglrx_version_minor > minor) ||
             (fglrx_version_major == major &&
              fglrx_version_minor == minor &&
              fglrx_version_micro >= micro)));
}

Bool fglrx_get_device_id(
    Display      *dpy,
    int           screen,
    unsigned int *deviceID
)
{
    XExtDisplayInfo *info = find_display(dpy);
    xFGLGetDriverDataReply rep;
    xFGLGetDriverDataReq  *req;

    if (deviceID)
        *deviceID = 0;

    if(!XextHasExtension(info))
        return False;

    xFGLCheckExtension(dpy, info, False);

    LockDisplay(dpy);
    GetReq(FGLGetDriverData, req);
    req->reqType       = info->codes->major_opcode;
    req->fireglReqType = X_FGLGetDriverData;
    req->screen        = screen;
    if (!_XReply(dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return False;
    }
    UnlockDisplay(dpy);
    SyncHandle();

    if (deviceID)
        *deviceID = rep.deviceID;
    return True;
}

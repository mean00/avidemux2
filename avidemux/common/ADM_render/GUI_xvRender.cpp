/***************************************************************************
    \file GUI_xvRender.cpp
    \author mean fixounet@free.fr, This file is strongly derivated from xine/mplayer/mpeg2dec
    
    copyright            : (C) 2002/2016 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERBOSE_XV


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>

#include "ADM_default.h"
#include "GUI_render.h"

#include "GUI_accelRender.h"
#include "GUI_xvRender.h"

// Instantiator
//
VideoRenderBase *spawnXvRender()
{
    return new XvRender();
}

/**
    \fn XvRender
 *  \brief Ctor
*/
XvRender::XvRender( void )
{

}
/**
 * \fn XvRender dtor
 */
XvRender::~XvRender()
{

}
/**
    \fn init
*/
bool XvRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
    ADM_info("[Xvideo]Xv start\n");
    info=*window;
    baseInit(w,h,zoom);
    return  lowLevelXvInit( window,  w,  h);
}
/**
    \fn stop
*/
bool XvRender::stop(void)
{
     ADM_assert(xv_port);
     ADM_assert(xv_display);


    ADM_info("[Xvideo] Releasing Xv Port\n");
    XLockDisplay (xv_display);
    if(XvUngrabPort(xv_display,xv_port,0)!=Success)
    {
        ADM_warning("[Xvideo] Trouble releasing port...\n");
    }
    XUnlockDisplay (xv_display);
    xvimage=NULL;
    xv_display=NULL;
    xv_port=0;
    return true;
}
/**
    \fn displayImage
*/
bool XvRender::displayImage(ADMImage *src)
{
    if (xvimage)
    {
        XLockDisplay (xv_display);
        int plane=imageWidth*imageHeight;
        BitBlit((uint8_t *)xvimage->data, imageWidth,src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y),imageWidth,imageHeight);
        BitBlit((uint8_t *)xvimage->data+plane, imageWidth/2,src->GetReadPtr(PLANAR_U),src->GetPitch(PLANAR_U),imageWidth/2,imageHeight/2);
        BitBlit((uint8_t *)xvimage->data+(plane*5)/4, imageWidth/2,src->GetReadPtr(PLANAR_V),src->GetPitch(PLANAR_V),imageWidth/2,imageHeight/2);
        XUnlockDisplay (xv_display);
        xvDraw(imageWidth,imageHeight,displayWidth,displayHeight);
    }
    return 1;    
}

/**
    \fn changeZoom
*/
bool XvRender::changeZoom(renderZoom newZoom)
{
        ADM_info("changing zoom, xv render.\n");
        calcDisplayFromZoom(newZoom);
        currentZoom=newZoom;
        return true;
}
/**
    \fn refresh
*/
bool XvRender::refresh(void)
{
    // since we dont know how to redraw without help, ask above
    ADM_info("XV:refresh\n");
    xvDraw(imageWidth,imageHeight,displayWidth,displayHeight);
    return true;
}

/**
    \fn xvDraw
*/
bool XvRender::xvDraw(uint32_t w,uint32_t h,uint32_t destW,uint32_t destH)
{
    if(!xvimage) return false;
    XLockDisplay (xv_display);
    XvShmPutImage(xv_display, xv_port, xv_win, xv_gc, xvimage, 0, 0, w, h, 0, 0, destW, destH,  False);
    XUnlockDisplay (xv_display);
    XSync(xv_display, False);
    return true;
}
/**
    \fn lowLevelXvInit
*/
#define WDN xv_display
bool XvRender::lowLevelXvInit(GUI_WindowInfo * window, uint32_t w, uint32_t h)
{
    
    unsigned int port, adaptors;
    static XvAdaptorInfo *ai;
    static XvAdaptorInfo *curai;
    static XShmSegmentInfo Shminfo;


    xv_display=(Display *)window->display;
    xv_win=window->systemWindowId;
    xv_port = 0;
    {
        unsigned int ver, rel, req, ev, err;
        if (Success != XvQueryExtension(WDN, &ver, &rel, &req, &ev, &err))
        {
            ADM_info("[Xvideo] Query Extension failed\n");
            return false;
        }
    }
    /* check for Xvideo support */
    if (Success != XvQueryAdaptors(WDN,
                   DefaultRootWindow(WDN), &adaptors, &ai))
    {
        ADM_info("[Xvideo] Query Adaptor failed\n");
        return false;
    }
    curai = ai;

    // Dump infos
    port = 0;
    for (int  i = 0; (!port) && (i < adaptors); i++)
    {
        XvFormat *formats;
        displayAdaptorInfo(i,curai);
        formats = curai->formats;
        for (int k = 0; (k < curai->num_ports) && !port; k++)
        {
          if (lookupYV12(WDN, k + curai->base_id, &xv_format))
          {
              port = k + curai->base_id;
              break;
          }
        }
        curai++;
    }
    //
    if (!port)
    {
      ADM_info("[Xvideo] no port found\n");
      return false;
    }
    ADM_info("[Xvideo] Xv YV12 found at port :%d, format : %" PRIi32"\n", port, xv_format);
    if (Success != XvGrabPort(WDN, port, 0))
    {
        ADM_warning("Grabbing port failed\n");
        return false;
    }
    {
    xv_port = port;

    XSetWindowAttributes xswa;
    XWindowAttributes attribs;
    static Atom xv_atom;
    unsigned long xswamask;
    int erCode;

    /* check if colorkeying is needed */
    xv_atom = getAtom( "XV_AUTOPAINT_COLORKEY" ,xv_display,xv_port);
    if(xv_atom!=None)
    {
            XvSetPortAttribute( xv_display, xv_port, xv_atom, 1 );
    }
    else
    {
        ADM_warning("No autopaint \n");
    }

    /* if we have to deal with colorkeying ... */

    xvimage = XvShmCreateImage(WDN, xv_port,
                   xv_format, 0, w, h, &Shminfo);

    Shminfo.shmid = shmget(IPC_PRIVATE, xvimage->data_size,
                   IPC_CREAT | 0777);
    if(Shminfo.shmid<=0)
    {
            ADM_warning("shmget failed\n");
            return false;
    }
    Shminfo.shmaddr = (char *) shmat(Shminfo.shmid, 0, 0);
    Shminfo.readOnly = False;
    if(Shminfo.shmaddr==(char *)-1)
    {
            ADM_warning("Shmat failed\n");
            return false;
    }
    xvimage->data = Shminfo.shmaddr;
    XShmAttach(WDN, &Shminfo);
    XSync(WDN, False);
    erCode=shmctl(Shminfo.shmid, IPC_RMID, 0);
    if(erCode)
    {
            ADM_warning("Shmctl failed :%d\n",erCode);
            return false;
    }
    memset(xvimage->data, 0, xvimage->data_size);
    xv_xgc.graphics_exposures = False;
    xv_gc = XCreateGC(xv_display, xv_win, 0L, &xv_xgc);
    }
    ADM_info("[Xvideo] Xv init succeedeed\n");
    return true;
}

/**
    \fn lookupYV12
 *  \brief search for YV12 compatible port, returns it in fmt
*/
bool  XvRender::lookupYV12(Display * dis, uint32_t port, uint32_t * fmt)
{
    XvImageFormatValues *formatValues;
    int imgfmt;
    int k;
    bool found = false;

    formatValues = XvListImageFormats(dis, port, &imgfmt);
// when "formatValues" is NULL, imgfmt should be zero, too
//    if (formatValues)
// this will run endless or segfault if the colorspace searched for isn't found
    for (k = 0; !found && (k < imgfmt); k++)
    {
        ADM_info("[Xvideo]%d/%d: %" PRIx32" %d --> %s\n", k,imgfmt,port, formatValues[k].id,  formatValues[k].guid);
        if (!strcmp(formatValues[k].guid, "YV12"))
        {
            found = true;
            *fmt = formatValues[k].id;
        }
    }
    if (formatValues)           //checking if it's no NULL-pointer won't hurt
        XFree(formatValues);
    return found;
}
/**
    \fn getAtom
*/
Atom XvRender::getAtom(const char *string,Display *xv_display,unsigned int  xv_port)
{
XvAttribute * attributes;
int attrib_count,i;
Atom xv_atom = None;

    attributes = XvQueryPortAttributes( xv_display, xv_port, &attrib_count );
    if(! attributes )
        return None;
    
    for ( i = 0; i < attrib_count; ++i )
    {
      if (! strcmp(attributes[i].name, string ))
      {
        xv_atom = XInternAtom( xv_display, string, False );
        XFree( attributes );
        return xv_atom;
      }
    }
    XFree( attributes );
    return None;

}
/**
 * 
 * @param curai
 */
void XvRender::displayAdaptorInfo(int num, XvAdaptorInfo *curai)
{
#ifdef VERBOSE_XV
        ADM_info("[Xvideo]_______________________________\n");
        ADM_info("[Xvideo] Adaptor           : %d\n", num);
        ADM_info("[Xvideo] Base ID           : %ld\n", curai->base_id);
        ADM_info("[Xvideo] Nb Port           : %lu\n", curai->num_ports);
        ADM_info("[Xvideo] Type              : %d ,", curai->type);
  #define CHECK(x) if(curai->type & x) ADM_info("|"#x);
        CHECK(XvInputMask);
        CHECK(XvOutputMask);
        CHECK(XvVideoMask);
        CHECK(XvStillMask);
        CHECK(XvImageMask);

        ADM_info("\n");
        ADM_info("[Xvideo] Name              : %s\n", curai->name);
        ADM_info("[Xvideo] Num Adap          : %lu\n", curai->num_adaptors);
        ADM_info("[Xvideo] Num fmt           : %lu\n", curai->num_formats);
#endif
}
//

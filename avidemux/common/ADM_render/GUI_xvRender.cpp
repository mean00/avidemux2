/***************************************************************************
                          gui_xv.cpp  -  description
                             -------------------

	This part is strongly derivated from xine/mplayer/mpeg2dec

    begin                : Tue Jan 1 2002
    copyright            : (C) 2002 by mean
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

#ifdef USE_XV
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
#include "ADM_assert.h"

static uint8_t 	GUI_XvList(Display * dis, uint32_t port, uint32_t * fmt);
static uint8_t 	GUI_XvInit(GUI_WindowInfo * window, uint32_t w, uint32_t h);
static void 	GUI_XvEnd( void );
static uint8_t GUI_XvDisplay(uint8_t * src, uint32_t w, uint32_t h,uint32_t destW,uint32_t destH);
static uint8_t 	GUI_XvSync(void);
static uint8_t GUI_XvRedraw( void );
static uint8_t getAtom(const char *string);
//________________Wrapper around Xv_______________
XvRender::XvRender( void )
{

}
bool XvRender::init( GUI_WindowInfo * window, uint32_t w, uint32_t h,renderZoom zoom)
{
	ADM_info("[Xvideo]Xv start\n");
    info=*window;
    baseInit(w,h,zoom);
	return  GUI_XvInit( window,  w,  h);
}
bool XvRender::stop(void)
{
	 GUI_XvEnd( );
	 printf("[Xvideo]Xv end\n");
	 return 1;
}

bool XvRender::displayImage(ADMImage *pic)
{

	return GUI_XvDisplay(pic->data, imageWidth, imageHeight,displayWidth,displayHeight);
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
    GUI_XvRedraw();
    return true;
}
//________________Wrapper around Xv_______________

static unsigned int xv_port;
static uint32_t xv_format;
static Display *xv_display;
static XvImage *xvimage = NULL;
static GC xv_gc;
static XGCValues xv_xgc;
static Window xv_win;
static XShmSegmentInfo Shminfo;
static uint8_t GUI_XvExpose( void );
//
//	Free all ressources allocated by xv
//


void GUI_XvEnd( void )
{
	ADM_assert(xv_port);
 	ADM_assert(xv_display);


  	printf("[Xvideo] Releasing Xv Port\n");
  	XLockDisplay (xv_display);
	if(XvUngrabPort(xv_display,xv_port,0)!=Success)
 				printf("[Xvideo] Trouble releasing port...\n");
	XUnlockDisplay (xv_display);


     xvimage=NULL;
     xv_display=NULL;
     xv_port=0;

}

//------------------------------------
uint8_t GUI_XvDisplay(uint8_t * src, uint32_t w, uint32_t h,uint32_t destW,uint32_t destH)
{
    
    if (xvimage)
      {

	  // put image in shared segment

	  // for YV12, 4 bits for Y 4 bits for u, 4 bits for v
	  // total 1.5*
          XLockDisplay (xv_display);
          memcpy(xvimage->data, src, (w*h*3)>>1);
        //printf("%u x %u => %u x %u\n",w,h,destW,destH);
        // And display it !
#if 1
	  XvShmPutImage(xv_display, xv_port, xv_win, xv_gc, xvimage, 0, 0, w, h,	// src
			0, 0, destW, destH,	// dst
			False);
#else
	 XvPutImage(xv_display, xv_port, xv_win, xv_gc, xvimage, 0, 0, w, h,	// src
			0, 0, w, h	// dst
			);

#endif
	  //XSetForeground (xv_display, xv_gc, 0);

	  XSync(xv_display, False);
	  XUnlockDisplay (xv_display);
      }
    return 1;
}
uint8_t GUI_XvSync(void)
{
	if(xv_display)
	  	XSync(xv_display, False);
	return 1;
}
//------------------------------------
//
//------------------------------------
uint8_t GUI_XvInit(GUI_WindowInfo * window, uint32_t w, uint32_t h)
{


    unsigned int ver, rel, req, ev, err;
    unsigned int port, adaptors;
    static XvAdaptorInfo *ai;
    static XvAdaptorInfo *curai;

#if 0
    win = gtk_widget_get_parent_window(window);
    xv_display = GDK_WINDOW_XDISPLAY(win);
//      xv_win= RootWindow(xv_display,0);
    xv_win = GDK_WINDOW_XWINDOW(GTK_WIDGET(window)->window);
#endif

    xv_display=(Display *)window->display;
    xv_win=window->window;


#define WDN xv_display
    xv_port = 0;

    if (Success != XvQueryExtension(WDN, &ver, &rel, &req, &ev, &err))
      {
	  printf("[Xvideo] Query Extension failed\n");
	  goto failed;
      }
    /* check for Xvideo support */
    if (Success != XvQueryAdaptors(WDN,
				   DefaultRootWindow(WDN), &adaptors, &ai))
      {
	  printf("[Xvideo] Query Adaptor failed\n");
	  goto failed;
      }
    curai = ai;
    XvFormat *formats;
    // Dump infos
    port = 0;
    for (uint16_t i = 0; (!port) && (i < adaptors); i++)
      {
/*
XvPortID base_id;
unsigned long num_ports;
char type;
char *name;
unsigned long num_formats;
XvFormat *formats;
unsigned long num_adaptors;
*/
#ifdef VERBOSE_XV
	  printf("[Xvideo]_______________________________\n");
	  printf("[Xvideo] Adaptor 		: %d\n", i);
	  printf("[Xvideo] Base ID		: %ld\n", curai->base_id);
	  printf("[Xvideo] Nb Port	 	: %lu\n", curai->num_ports);
	  printf("[Xvideo] Type			 	: %d,", curai->type);
#define CHECK(x) if(curai->type & x) printf("|"#x);
	  CHECK(XvInputMask);
	  CHECK(XvOutputMask);
	  CHECK(XvVideoMask);
	  CHECK(XvStillMask);
	  CHECK(XvImageMask);

	  printf("\n[Xvideo] Name			 	: %s\n",
		 curai->name);
	  printf("[Xvideo] Num Adap	 	: %lu\n", curai->num_adaptors);
	  printf("[Xvideo] Num fmt	 	: %lu\n", curai->num_formats);
#endif
	  formats = curai->formats;

	  //
	  uint16_t k;

	  for (k = 0; (k < curai->num_ports) && !port; k++)
	    {
		if (GUI_XvList(WDN, k + curai->base_id, &xv_format))
		    port = k + curai->base_id;
	    }


	  curai++;
      }
    //
    if (!port)
      {
	  printf("[Xvideo] no port found\n");
	  goto failed;
      }
#ifdef 	COLORSPACE_YV12
    printf("[Xvideo] Xv YV12 found at port :%d, format : %"LD"\n", port, xv_format);
#else
    printf("[Xvideo] Xv YUY2 found at port :%d, format : %"LD"\n", port, xv_format);
#endif

    if (Success != XvGrabPort(WDN, port, 0))
	goto failed;
    {

	xv_port = port;
/*
   Display *display,
   XvPortID port,
   int id,
   char* data,
   int width,
   int height,
   XShmSegmentInfo *shminfo

*/

        XSetWindowAttributes xswa;
        XWindowAttributes attribs;
        static Atom xv_atom;
        unsigned long xswamask;
        int erCode;

        /* check if colorkeying is needed */
        xv_atom = getAtom( "XV_AUTOPAINT_COLORKEY" );
        if(xv_atom!=None)
        {
                XvSetPortAttribute( xv_display, xv_port, xv_atom, 1 );
        }
        else printf("No autopaint \n");

        /* if we have to deal with colorkeying ... */

	xvimage = XvShmCreateImage(WDN, xv_port,
				   xv_format, 0, w, h, &Shminfo);

	Shminfo.shmid = shmget(IPC_PRIVATE, xvimage->data_size,
			       IPC_CREAT | 0777);
        if(Shminfo.shmid<=0)
        {
                printf("shmget failed\n");
        }
	Shminfo.shmaddr = (char *) shmat(Shminfo.shmid, 0, 0);
	Shminfo.readOnly = False;
        if(Shminfo.shmaddr==(char *)-1)
        {
                printf("Shmat failed\n");
        }
	xvimage->data = Shminfo.shmaddr;
	XShmAttach(WDN, &Shminfo);
	XSync(WDN, False);
	erCode=shmctl(Shminfo.shmid, IPC_RMID, 0);
        if(erCode)
        {
                printf("Shmctl failed :%d\n",erCode);
        }
	memset(xvimage->data, 0, xvimage->data_size);

	xv_xgc.graphics_exposures = False;

	xv_gc = XCreateGC(xv_display, xv_win, 0L, &xv_xgc);

	//ADM_assert(BadWindow!=XSelectInput(xv_display, xv_win, ExposureMask | VisibilityChangeMask));

    }
    printf("[Xvideo] Xv init succeedeed\n");

    return 1;
  failed:
    printf("[Xvideo] Xv init failed..\n");
    return 0;
}

// _________________________________________________
//
// _________________________________________________
uint8_t GUI_XvList(Display * dis, uint32_t port, uint32_t * fmt)
{
    XvImageFormatValues *formatValues;
    int imgfmt;
    int k, f = 0;

    formatValues = XvListImageFormats(dis, port, &imgfmt);
// when "formatValues" is NULL, imgfmt should be zero, too
//    if (formatValues)

// this will run endless or segfault if the colorspace searched for isn't found
//	for (k = 0; !f || (k < imgfmt); k++)
	for (k = 0; !f && (k < imgfmt); k++)
	  {
#ifdef VERBOSE_XV
	      printf("[Xvideo]%d/%d: %"LX" %d --> %s\n", k,imgfmt,port, formatValues[k].id,  formatValues[k].guid);
#endif

	      if (!strcmp(formatValues[k].guid, "YV12"))
		{
		    f = 1;
		    *fmt = formatValues[k].id;
		}
    }// else
     //	f = 0; // f has already been initialized zero
	if (formatValues) //checking if it's no NULL-pointer won't hurt
	    XFree(formatValues);
    return f;
}
uint8_t getAtom(const char *string)
{
  XvAttribute * attributes;
  int attrib_count,i;
  Atom xv_atom = None;

  attributes = XvQueryPortAttributes( xv_display, xv_port, &attrib_count );
  if( attributes!=NULL )
  {
    for ( i = 0; i < attrib_count; ++i )
    {
      if ( strcmp(attributes[i].name, string ) == 0 )
      {
        xv_atom = XInternAtom( xv_display, string, False );
        break; // found what we want, break out
      }
    }
    XFree( attributes );
  }

  return xv_atom;

}
void GUI_XvBuildAtom(Display * dis, Atom * atom, char *string)
{
    UNUSED_ARG(dis);
    UNUSED_ARG(atom);
    UNUSED_ARG(string);
}


uint8_t GUI_XvRedraw( void )
{
	printf("Xv need redraw !\n");

}
#endif

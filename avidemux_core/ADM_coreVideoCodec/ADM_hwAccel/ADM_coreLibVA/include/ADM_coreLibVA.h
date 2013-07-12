/***************************************************************************
    \file             : ADM_coreLibva.h
    \brief            : Wrapper around libva functions
    \author           : (C) 2013 by mean fixounet@free.fr, derived from xbmc_pvr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_CORE_LIBVA_H
#define ADM_CORE_LIBVA_H

#ifdef USE_LIBVA
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/vaapi.h"
}

#include "ADM_windowInfo.h"
#include "X11/Xlib.h"
#include "va/va.h"
#include "ADM_image.h"

#define VA_INVALID -1
class ADM_vaImage;


/**
    \class admLibVA
*/

class admLibVA
{
protected:
    static GUI_WindowInfo      myWindowInfo;
public:
    static bool         init(GUI_WindowInfo *x);
    static bool         isOperationnal(void);
    static bool         cleanup(void);
    
    /* Setup   */
    static bool         setupConfig(void);
    static bool         setupImageFormat(void);
    /* Surface */   
    
static  VAContextID createDecoder(int width, int height, int nbSurface, VASurfaceID *surfaces);
static  bool        destroyDecoder(VAContextID decoder);

static  VASurfaceID allocateSurface( int w, int h);
static  void        destroySurface(  VASurfaceID surface);

static  VAImage    *allocateYV12Image( int w, int h);
static  void       destroyImage(  VAImage *image);


static bool        transfer(VAContextID session, int w, int h,VASurfaceID surface, ADMImage *img,VAImage *tmp,uint8_t *yv12);
static bool        fillContext(vaapi_context *c);
static bool        surfaceToImage(VASurfaceID id,ADM_vaImage *img);

};

/**
 * \class ADM_vaImage
 */
class decoderFFLIBVA;
class ADM_vaImage
{
public:
    VAImage             *image;
    int                 refCount;
    decoderFFLIBVA      *parent;
    int                 w,h;
    ADM_vaImage(decoderFFLIBVA *parent,int w, int h)
    {
        this->parent=parent;
        image=NULL;
        refCount=0;
        this->w=w;
        this->h=h;
    }
    ~ADM_vaImage()
    {
        if(image)
        {
           admLibVA::destroyImage(NULL);
           image=NULL;
        }
    }
    
    
};

#endif
#endif

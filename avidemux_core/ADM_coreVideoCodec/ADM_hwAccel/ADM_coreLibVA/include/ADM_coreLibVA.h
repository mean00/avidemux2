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
}

#include "ADM_windowInfo.h"
#include "X11/Xlib.h"
#include "va/va.h"
#include "ADM_image.h"

#define VA_INVALID VA_INVALID_ID
class ADM_vaSurface;

/**
    \class admLibVA
*/

class admLibVA
{
protected:
    static GUI_WindowInfo      myWindowInfo;
public:
    typedef enum {
        ADM_LIBVA_DRIVER_QUIRK_NONE = 0,
        ADM_LIBVA_DRIVER_QUIRK_USER_SET = (1 << 0),
        ADM_LIBVA_DRIVER_QUIRK_RENDER_PARAM_BUFFERS = (1 << 1),
        ADM_LIBVA_DRIVER_QUIRK_ATTRIB_MEMTYPE = (1 << 2),
        ADM_LIBVA_DRIVER_QUIRK_SURFACE_ATTRIBUTES = (1 << 3)
    } LIBVA_DRIVER_QUIRK; // see libavutil/hwcontext_vaapi.h
    typedef enum 
    {
        ADM_LIBVA_NONE,
        ADM_LIBVA_DIRECT, ADM_LIBVA_INDIRECT_NV12, ADM_LIBVA_INDIRECT_YV12
    }LIBVA_TRANSFER_MODE;

    
    static bool         init(GUI_WindowInfo *x);
    static bool         isOperationnal(void);
    static bool         cleanup(void);
    static VADisplay    getVADisplay();
    /* Setup   */
    static bool         setupConfig(void);
    static bool         setupImageFormat(void);
    /* Encoding - setup */
    static bool         setupEncodingConfig(void);
    /* Surface */   
    
static  VAContextID createDecoder(VAProfile profile,int width, int height, int nbSurface, VASurfaceID *surfaces);
static  bool        destroyDecoder(VAContextID decoder);

static  VASurfaceID allocateSurface( int w, int h, int fmt=VA_RT_FORMAT_YUV420);
static  void        destroySurface(  VASurfaceID surface);

static  VAImage    *allocateNV12Image( int w, int h);
static  VAImage    *allocateYV12Image( int w, int h);
static  VAImage    *allocateP010Image( int w, int h);
static  void       destroyImage(  VAImage *image);
static  VAImage    *allocateImage(int w, int h, int bpp);

static bool        transfer(VAContextID session, int w, int h,VASurfaceID surface, ADMImage *img,VAImage *tmp,uint8_t *yv12);

// Indirect access through image
static bool        uploadToImage(ADMImage *src,VAImage *dest );
static bool        downloadFromImage(ADMImage *src,VAImage *dest,ADM_vaSurface *face=NULL);
static bool        imageToSurface(VAImage *src,ADM_vaSurface *dest);
static bool        surfaceToImage(ADM_vaSurface *dst,VAImage *src );

// Display

static bool        putX11Surface(ADM_vaSurface *img,int widget,int sourceWidth,int sourceHeight,int displayWidth,int displayHeight);

// Direct access

static bool        admImageToSurface( ADMImage *src,ADM_vaSurface *dest);
static bool        surfaceToAdmImage(ADMImage *dest,ADM_vaSurface *src);

//
static bool        supported(VAProfile profile, int width = -1, int height = -1);

//-- config for filters --
static VAConfigID  createFilterConfig();
static bool        destroyFilterConfig(VAConfigID &id);

//-- config for filters --
static VAContextID createFilterContext();
static bool        destroyFilterContext(VAContextID &id);


static VADisplay   getDisplay();
};
/**
 * \class admLibVAEnc
 */
class ADMBitstream;
/**
 * \class ADM_vaSurface
 */
class decoderFFLIBVA;
class ADM_vaSurface
{
public:
    VASurfaceID         surface;
    int                 refCount;
    int                 refCountInternal; // held by lavcodec
    int                 refCountExternal; // held by decoder and users
    VAImage             *image;
    int                 w,h;
    ADMColorScalerSimple *fromNv12ToYv12;
    ADMColorScalerSimple *color10bits;
    ADM_vaSurface(int w, int h, int bpp = 8);
    ~ADM_vaSurface();

    bool hasValidSurface() {return !(surface==VA_INVALID);}
    bool toAdmImage(ADMImage *image);
    bool fromAdmImage(ADMImage *image);
    
    static ADM_vaSurface *allocateWithSurface(int w,int h,int fmt=VA_RT_FORMAT_YUV420);
    
};
#endif
#endif

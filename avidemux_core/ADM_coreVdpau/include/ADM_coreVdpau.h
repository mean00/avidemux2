/***************************************************************************
    \file             : ADM_coreVdpau.cpp
    \brief            : Wrapper around vdpau functions
    \author           : (C) 2010 by mean fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_CORE_VDPAU_H
#define ADM_CORE_VDPAU_H
#include "vdpau/vdpau_x11.h"
#include "vdpau/vdpau.h"
#include "ADM_windowInfo.h"
/**
    \class admVdpau
*/
class admVdpau
{
protected:
    static GUI_WindowInfo      myWindowInfo;
    static bool queryYUVPutBitSupport(VdpRGBAFormat rgb,VdpYCbCrFormat yuv);
public:
    static bool         init(GUI_WindowInfo *x);
    static const char  *getErrorString(VdpStatus er);
    static bool         isOperationnal(void);
    /* Surface */
    static  VdpStatus   surfaceCreate(uint32_t width,uint32_t height,VdpVideoSurface *surface);
    static  VdpStatus   surfaceDestroy(VdpVideoSurface surface);
    static  VdpStatus   getDataSurface(VdpVideoSurface surface,uint8_t *planes[3],uint32_t stride[3]);
    /* Decoder */
    static  VdpStatus   decoderCreate( VdpDecoderProfile profile,    uint32_t          width,    uint32_t          height,    uint32_t          max_references,       VdpDecoder *      decoder);
    static  VdpStatus   decoderDestroy(VdpDecoder decoder);

    static  VdpStatus   decoderRender(
            VdpDecoder                 decoder,
            VdpVideoSurface            target,
            const void                 *info,
            uint32_t                   bitstream_buffer_count,
            VdpBitstreamBuffer const * bitstream_buffers);

    /* Output surface */
    static VdpStatus outputSurfaceCreate(
            VdpRGBAFormat      rgba_format,
            uint32_t           width,
            uint32_t           height,
            VdpOutputSurface * surface);

    static VdpStatus outputSurfaceDestroy(VdpOutputSurface surface);
    static VdpStatus outPutSurfacePutBitsYV12( 
            VdpOutputSurface     surface,
            uint8_t *planes[3],
            uint32_t pitches[3]);
    /* Presentation queue */
    static VdpStatus presentationQueueCreate(VdpPresentationQueue *queue);
    static VdpStatus presentationQueueDestroy(VdpPresentationQueue queue);
    static VdpStatus presentationQueueDisplay(VdpPresentationQueue queue,VdpOutputSurface outputSurface);
};
#endif
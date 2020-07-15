/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_image.h"
#include <CoreServices/CoreServices.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/videotoolbox.h"
#include "libavutil/frame.h"
#include "libavutil/pixdesc.h"
}

#include "../include/ADM_coreVideoToolbox.h"

#define PREFER_NV12 0

static AVVideotoolboxContext *vtctx;

int admCoreVideoToolbox::copyData(AVCodecContext *s, AVFrame *src, ADMImage *dest)
{
    CVPixelBufferRef pixbuf = (CVPixelBufferRef)src->data[3];
    OSType pixel_format = CVPixelBufferGetPixelFormatType(pixbuf);
    CVReturn err;
    uint8_t *data[4] = { 0 };
    int linesize[4] = { 0 };
    int planes, i;

    switch (pixel_format)
    {
#if PREFER_NV12
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
#else
        case kCVPixelFormatType_420YpCbCr8Planar:
#endif
            //ADM_info("pixel format: %s (%d)\n", (pixel_format == kCVPixelFormatType_420YpCbCr8Planar)? "YUV420P" : "NV12", pixel_format);
            break;
        default:
            ADM_error("%s: Unsupported pixel format: %d\n", av_fourcc2str(s->codec_tag), pixel_format);
            return AVERROR(ENOSYS);
    }

    dest->_colorspace = ADM_COLOR_YV12; // we handle only YUV420P for now
    dest->_width  = CVPixelBufferGetWidth(pixbuf);
    dest->_height = CVPixelBufferGetHeight(pixbuf);

    err = CVPixelBufferLockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
    if (err != kCVReturnSuccess)
    {
        ADM_error("Error locking the pixbuf base address.\n");
        return AVERROR_UNKNOWN;
    }

    if (CVPixelBufferIsPlanar(pixbuf))
    {
        planes = CVPixelBufferGetPlaneCount(pixbuf);
        for (i = 0; i < planes; i++)
        {
            data[i]     = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixbuf, i);
            linesize[i] = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, i);
        }
    } else
    {
        data[0] = (uint8_t *)CVPixelBufferGetBaseAddress(pixbuf);
        linesize[0] = CVPixelBufferGetBytesPerRow(pixbuf);
    }

    ADMImageRefWrittable ref(dest->_width, dest->_height);
    for (i = 0; i < 3; i++)
    {
        ref._planes[i] = data[i];
        ref._planeStride[i] = linesize[i];
    }
#if PREFER_NV12
    dest->convertFromNV12(ref._planes[0], ref._planes[1], ref._planeStride[0], ref._planeStride[1]);
#else
    dest->duplicateMacro(&ref, true);
#endif

    CVPixelBufferUnlockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);

    return 0;
}

void admCoreVideoToolbox::uninit(AVCodecContext *s)
{
    av_videotoolbox_default_free(s);
    if (vtctx)
    {
        av_free(vtctx);
        vtctx = NULL;
    }
}

int admCoreVideoToolbox::initVideoToolbox(AVCodecContext *s)
{
    int ret = 0;

    AVVideotoolboxContext *vtctx = av_videotoolbox_alloc_context();
    ADM_assert(vtctx);
#if PREFER_NV12 == 0
    vtctx->cv_pix_fmt_type = kCVPixelFormatType_420YpCbCr8Planar; // lavc defaults to NV12, request YUV420P
#endif
    ret = av_videotoolbox_default_init2(s, vtctx);

    if (ret < 0)
    {
        ADM_error("Error %d creating VideoToolbox decoder\n", ret);
        goto fail;
    }

    return 0;
fail:
    uninit(s);
    return ret;
}

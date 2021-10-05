/**

    \file ADM_uyvy
    \author mean fixounet@free.fr, 2004-1010
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_uyvy.h"


/**
    \fn uncompress
*/
bool   decoderUYVY::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    
    if(in->dataLength!=_w*_h*2)
    {
        return false;
    }
    
    ADMImageRef *ref=out->castToRef();
    out->flags = AVI_KEY_FRAME;
    out->_pixfrmt = ADM_PIXFRMT_UYVY422;

    ref->_planes[0] = in->data;
    ref->_planes[1] = NULL;
    ref->_planes[2] = NULL;

    ref->_planeStride[0] = _w*2;
    ref->_planeStride[1] = 0;
    ref->_planeStride[2] = 0;
    out->Pts=in->demuxerPts;
    // make sure the output is not marked as a hw image
    int count = 0;
    while(out->refType != ADM_HW_NONE && count < 32 /* arbitrary limit */)
    {
        out->hwDecRefCount();
        count++;
    }
    return true;
}

/**
    \fn uncompress
*/
bool   decoderYUY2::uncompress  (ADMCompressedImage * in, ADMImage * out)
{
    if(in->dataLength!=_w*_h*2)
    {
        return false;
    }


    ADMImageRef *ref=out->castToRef();
    out->flags = AVI_KEY_FRAME;
    out->_pixfrmt = ADM_PIXFRMT_YUV422;

    ref->_planes[0] = in->data;
    ref->_planes[1] = NULL;
    ref->_planes[2] = NULL;

    ref->_planeStride[0] = _w*2;
    ref->_planeStride[1] = 0;
    ref->_planeStride[2] = 0;
    out->Pts=in->demuxerPts;
    int count = 0;
    while(out->refType != ADM_HW_NONE && count < 32 /* arbitrary limit */)
    {
        out->hwDecRefCount();
        count++;
    }
    return true;
}

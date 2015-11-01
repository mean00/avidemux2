/***************************************************************************
                          \fn ADM_VideoEncoders
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#include "ADM_default.h"
#include "ADM_yv12Encoder.h"

/**
        \fn ADM_yv12Encoder
*/
ADM_yv12Encoder::ADM_yv12Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[YV12Encoder] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImageDefault(w,h);
    plane=(w*h*3)/2;
}
/** 
    \fn ~ADM_yv12Encoder
*/
ADM_yv12Encoder::~ADM_yv12Encoder()
{
    ADM_info("[YV12Encoder] Destroying.\n");
}


/**
    \fn encode
*/
bool         ADM_yv12Encoder::encode (ADMBitstream * out)
{
    uint32_t fn;
    if(source->getNextFrame(&fn,image)==false)
    {
        ADM_info("[YV12] Cannot get next image\n");
        return false;
    }
    ADM_assert(out->bufferSize>plane);
    //--
    int w=image->GetWidth(PLANAR_Y);
    int h=image->GetHeight(PLANAR_Y);
    
    ADMImageRefWrittable packed(w,h);
    uint32_t square=w*h;
    packed._planes[0]=out->data;
    packed._planes[1]=out->data+square;
    packed._planes[2]=out->data+((square*5)>>2);
    
    packed._planeStride[0]=w;
    packed._planeStride[1]=packed._planeStride[2]=w>>1;
    
    packed.duplicate(image);
    out->len=plane;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}

// EOF

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
#include "ADM_pngEncoder.h"

/**
        \fn ADM_pngEncoder
*/
ADM_pngEncoder::ADM_pngEncoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoder(src)
{
    printf("[YV12Encoder] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImage(w,h);
    plane=(w*h*3)/2;
}
/** 
    \fn ~ADM_pngEncoder
*/
ADM_pngEncoder::~ADM_pngEncoder()
{
    printf("[YV12Encoder] Destroying.\n");
}
/**
    \fn getDisplayName
*/

const char *ADM_pngEncoder::getDisplayName(void)
{
    return "YV12 Encoder";

}
/**
    \fn getCodecName
*/

const char *ADM_pngEncoder::getCodecName(void)
{
    return "yv12";
}
/**
    \fn getFCCHandler
*/

const char *ADM_pngEncoder::getFCCHandler(void)
{
    return "yv12";

}
/**
    \fn configure
*/
bool        ADM_pngEncoder::configure(void)
{
    return true;
}

/**
    \fn getConfiguration
*/
bool        ADM_pngEncoder::getConfiguration(uint32_t *l,uint8_t **d)
{
        *l=0;
        *d=NULL;
        return true;
}

/**
    \fn setConfiguration
*/
bool        ADM_pngEncoder::setConfiguration(uint32_t l,uint8_t *d)
{
        return true;
}

/**
    \fn encode
*/
bool         ADM_pngEncoder::encode (ADMBitstream * out)
{
    if(source->getNextFrame(image)==false)
    {
        printf("[YV12] Cannot get next image\n");
        return false;
    }
    ADM_assert(out->bufferSize>plane);
    memcpy(out->data,image->data,plane);
    out->len=plane;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}

// EOF
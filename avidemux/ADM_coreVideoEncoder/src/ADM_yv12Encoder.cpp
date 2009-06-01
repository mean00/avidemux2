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
ADM_yv12Encoder::ADM_yv12Encoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoder(src)
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
    \fn ~ADM_yv12Encoder
*/
ADM_yv12Encoder::~ADM_yv12Encoder()
{
    printf("[YV12Encoder] Destroying.\n");
}
/**
    \fn getDisplayName
*/

const char *ADM_yv12Encoder::getDisplayName(void)
{
    return "YV12 Encoder";

}
/**
    \fn getCodecName
*/

const char *ADM_yv12Encoder::getCodecName(void)
{
    return "yv12";
}
/**
    \fn getFCCHandler
*/

const char *ADM_yv12Encoder::getFCCHandler(void)
{
    return "yv12";

}
/**
    \fn configure
*/
bool        ADM_yv12Encoder::configure(void)
{
    return true;
}

/**
    \fn getConfiguration
*/
bool        ADM_yv12Encoder::getConfiguration(uint32_t *l,uint8_t **d)
{
        *l=0;
        *d=NULL;
        return true;
}

/**
    \fn setConfiguration
*/
bool        ADM_yv12Encoder::setConfiguration(uint32_t l,uint8_t *d)
{
        return true;
}

/**
    \fn encode
*/
bool         ADM_yv12Encoder::encode (ADMBitstream * out)
{
    if(source->getNextFrame(image)==false)
    {
        printf("[YV12] Cannot get next image\n");
        return false;
    }
    memcpy(out->data,image->data,plane);
    out->pts=out->dts=image->Pts;
    return true;
}

// EOF
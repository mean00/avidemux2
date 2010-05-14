/***************************************************************************
                          \file nullEncoder.cpp
                          \brief dummy encoder
                             -------------------
    
    copyright            : (C) 2002/2010 by mean
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
#include "nullEncoder.h"
#include "DIA_factory.h"

//
/**
        \fn ADM_nullEncoder
*/
ADM_nullEncoder::ADM_nullEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[null] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImageDefault(w,h);
   
}
/** 
    \fn ~ADM_nullEncoder
*/
ADM_nullEncoder::~ADM_nullEncoder()
{
    ADM_info("[null] Destroying.\n");
    if(image) delete image;
    image=NULL;
    
}
/**
    \fn getFourcc
*/
const  char        *ADM_nullEncoder::getFourcc(void)
{
    return "null";
}
/**
    \fn encode
*/
bool         ADM_nullEncoder::encode (ADMBitstream * out)
{
    uint32_t fn;
    if(source->getNextFrame(&fn,image)==false)
    {
        printf("[null] Cannot get next image\n");
        return false;
    }
    out->len=1;
    out->data[0]=0;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}
// EOF

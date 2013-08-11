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
#include "ADM_libvaEncoder.h"

/**
        \fn ADM_libvaEncoder
*/
ADM_libvaEncoder::ADM_libvaEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    printf("[LibVAEncoder] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImageDefault(w,h);
    plane=(w*h*3)/2;
    for(int i=0;i<VA_ENC_NB_SURFACE;i++)    
        vaSurface[i]=NULL;
    context=NULL;
    encodingBuffer=NULL;
       

}
bool         ADM_libvaEncoder::setup(void)
{
    int width=getWidth();
    int height=getHeight();
    
    // Allocate VAImage

    for(int i=0;i<VA_ENC_NB_SURFACE;i++)
    {
        vaSurface[i]=new ADM_vaSurface(NULL,width,height);
        if(vaSurface[i]->image) 
        {
            ADM_warning("Cannot allocate surface\n");
            return false;
        }
    }
    context=new ADM_vaEncodingContext();
    if(!context->init(width,height,VA_ENC_NB_SURFACE,vaSurface))
    {
        ADM_warning("Cannot initialize vaEncoder context\n");
        return false;
    }
    encodingBuffer=new ADM_vaEncodingBuffer(context,(width*height*400)/256);
    return true;
}
/** 
    \fn ~ADM_libvaEncoder
*/
ADM_libvaEncoder::~ADM_libvaEncoder()
{
    ADM_info("[LibVAEncoder] Destroying.\n");
    for(int i=0;i<VA_ENC_NB_SURFACE;i++)
    {
        if(vaSurface[i])
        {
            delete vaSurface[i];
            vaSurface[i]=NULL;
        }
    }
    if(context)
    {
        delete context;
        context=NULL;
    }
    if(encodingBuffer)
    {
        delete encodingBuffer;
        encodingBuffer=NULL;
    }
}


/**
    \fn encode
*/
bool         ADM_libvaEncoder::encode (ADMBitstream * out)
{
    uint32_t fn;
    if(source->getNextFrame(&fn,image)==false)
    {
        ADM_warning("[LIBVA] Cannot get next image\n");
        return false;
    }
    if(!vaSurface[0]->fromAdmImage(image))
    {
        ADM_warning("Cannot upload image to surface\n");
        return false;
    }
    //
    if(!context->encode(vaSurface[0],out,encodingBuffer))
    {
        ADM_warning("Error encoding picture\n");
        return false;
    }
    
    out->len=plane;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}

// EOF

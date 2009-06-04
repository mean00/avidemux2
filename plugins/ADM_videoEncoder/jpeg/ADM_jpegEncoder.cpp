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
#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "ADM_jpegEncoder.h"


static int Quantizer=2;
static ADM_colorspace color=ADM_COLOR_RGB24; //ADM_COLOR_YUV422;
/**
        \fn ADM_jpegEncoder
*/
ADM_jpegEncoder::ADM_jpegEncoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoder(src)
{
    printf("[jpegEncoder] Creating.\n");
    int w,h;
    FilterInfo *info=src->getInfo();
    w=info->width;
    h=info->height;
    image=new ADMImage(w,h);
    plane=(w*h*3)/2;
    _context = avcodec_alloc_context ();
    ADM_assert (_context);
    memset (&_frame, 0, sizeof (_frame));
    _frame.pts = AV_NOPTS_VALUE;
    _context->width = w;
    _context->height = h;
    _context->strict_std_compliance = -1;
    // YV12
    switch(color)
    {
        case ADM_COLOR_YUV422: _frame.linesize[0] = w*2; _context->pix_fmt =PIX_FMT_YUYV422;break;
        case ADM_COLOR_RGB24 : _frame.linesize[0] = w*3; _context->pix_fmt =PIX_FMT_RGB24;break;
        default: ADM_assert(0);

    }
    _frame.linesize[1] = 0;//w >> 1;
    _frame.linesize[2] = 0;//w >> 1;
    //   
    _frame.quality = (int) floor (FF_QP2LAMBDA * Quantizer+ 0.5);
    rgbBuffer=new uint8_t [(w+7)*(h+7)*4];
}
/**
    \fn setup
*/
bool ADM_jpegEncoder::setup(void)
{
int res;
AVCodec *codec=avcodec_find_encoder(CODEC_ID_MJPEG);
if(!codec) 
    {
        printf("[Jpeg] Cannot find codec\n");
        return false;
    }
   // capabilities = codec->capabilities & CODEC_CAP_DELAY ? ADM_ENC_REQ_NULL_FLUSH : 0; 
   res=avcodec_open(_context, codec); 
   if(res<0) 
    {   printf("[Jpeg] Cannot open codec\n");
        return false;
    }
    _context->flags |= CODEC_FLAG_QSCALE;
    // Now allocate colorspace
    int w,h;
    FilterInfo *info=source->getInfo();
    w=info->width;
    h=info->height;
    
    colorSpace=new ADMColorspace(w,h,ADM_COLOR_YV12,color);
    if(!colorSpace)
    {
        printf("[ADM_jpegEncoder] Cannot allocate colorspace\n");
        return false;
    }
    return true;
}


/** 
    \fn ~ADM_jpegEncoder
*/
ADM_jpegEncoder::~ADM_jpegEncoder()
{
    printf("[jpegEncoder] Destroying.\n");
    if (_context)
    {
        avcodec_close (_context);
        ADM_dealloc (_context);
        _context = NULL;
    }
    if(colorSpace)
    {
        delete colorSpace;
        colorSpace=NULL;
    }   
    if(rgbBuffer)
    {
        delete [] rgbBuffer;
        rgbBuffer=NULL;
    }
}

/**
    \fn encode
*/
bool         ADM_jpegEncoder::encode (ADMBitstream * out)
{
    if(source->getNextFrame(image)==false)
    {
        printf("[jpeg] Cannot get next image\n");
        return false;
    }
    // 1- Convert to RGB24
     if(!colorSpace->convert(image->data,rgbBuffer))
        {
            printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
            return false;
        }
           
    _frame.data[0] = rgbBuffer;
    _frame.data[2] = NULL;
    _frame.data[1] = NULL;

    int sz=0;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
    {
        printf("[jpeg] Error %d encoding video\n",sz);
        return false;
    }
    
    out->len=sz;
    out->pts=out->dts=image->Pts;
    out->flags=AVI_KEY_FRAME;
    return true;
}

// EOF

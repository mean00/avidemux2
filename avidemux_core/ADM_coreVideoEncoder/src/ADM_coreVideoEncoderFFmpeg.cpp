/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
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
#define __STDC_CONSTANT_MACROS
#include "ADM_default.h"
#include "ADM_coreVideoEncoderFFmpeg.h"
#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME
/**
    \fn ADM_coreVideoEncoderFFmpeg
    \brief Constructor

*/

ADM_coreVideoEncoderFFmpeg::ADM_coreVideoEncoderFFmpeg(ADM_coreVideoFilter *src) : ADM_coreVideoEncoder(src)
{
uint32_t w,h;
    targetColorSpace=ADM_COLOR_YV12;
    w=getWidth();
    h=getHeight();

    image=new ADMImage(w,h);
    _context = avcodec_alloc_context2 (CODEC_TYPE_VIDEO);
    ADM_assert (_context);
    memset (&_frame, 0, sizeof (_frame));
    _frame.pts = AV_NOPTS_VALUE;
    _context->width = w;
    _context->height = h;
    _context->strict_std_compliance = -1;
    rgbBuffer=new uint8_t [(w+7)*(h+7)*4];

}
/**
    \fn ADM_coreVideoEncoderFFmpeg
    \brief Destructor
*/
ADM_coreVideoEncoderFFmpeg::~ADM_coreVideoEncoderFFmpeg()
{
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
    \fn prolog
*/

bool             ADM_coreVideoEncoderFFmpeg::prolog(void)
{
    int w=getWidth();
    int h=getHeight();

  switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:    _frame.linesize[0] = w; 
                                _frame.linesize[1] = w>>1; 
                                _frame.linesize[2] = w>>1; 
                                _context->pix_fmt =PIX_FMT_YUV420P;break;
        case ADM_COLOR_YUV422P: _frame.linesize[0] = w; 
                                _frame.linesize[1] = w>>1;
                                _frame.linesize[2] = w>>1;
                                _context->pix_fmt =PIX_FMT_YUV422P;break;
        case ADM_COLOR_RGB32A : _frame.linesize[0] = w*4;
                                _frame.linesize[1] = 0;//w >> 1;
                                _frame.linesize[2] = 0;//w >> 1;
                                _context->pix_fmt =PIX_FMT_RGB32;break;
        default: ADM_assert(0);

    }
    
    // Eval fps
    uint64_t f=source->getInfo()->frameIncrement;
    // Let's put 100 us as time  base
    _context->time_base.den=10000LL;
    _context->time_base.num=f/100;
    //printf("[Time base] %d/%d\n", _context->time_base.num,_context->time_base.den);
    return true;
}
/**
    \fn pre-encoder
    
*/
bool             ADM_coreVideoEncoderFFmpeg::preEncode(void)
{

    uint8_t *from;
    if(source->getNextFrame(image)==false)
    {
        printf("[ff] Cannot get next image\n");
        return false;
    }
    prolog();
    // put a time stamp...
    if(image->Pts==ADM_NO_PTS) _frame.pts=AV_NOPTS_VALUE;
    _frame.pts=(image->Pts)/100;
    //
    switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:      
                _frame.data[0] = image->GetWritePtr(PLANAR_Y);
                _frame.data[2] = image->GetWritePtr(PLANAR_U);
                _frame.data[1] = image->GetWritePtr(PLANAR_V);
                break;

        case ADM_COLOR_YUV422P:
        {
              int w=getWidth();
              int h=getHeight();

                if(!colorSpace->convert(image->data,rgbBuffer))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                _frame.data[0] = rgbBuffer;
                _frame.data[2] = rgbBuffer+(w*h);
                _frame.data[1] = rgbBuffer+(w*h*3)/2;
                break;
        }
        case ADM_COLOR_RGB32A:
                if(!colorSpace->convert(image->data,rgbBuffer))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                _frame.data[0] = rgbBuffer;
                _frame.data[2] = NULL;
                _frame.data[1] = NULL;
                break;
        default:
                ADM_assert(0);
    }
    return true;
}
/**
    \fn setup
    \brief put flags before calling setup!
*/
bool ADM_coreVideoEncoderFFmpeg::setup(CodecID codecId)
{
    int res;
    AVCodec *codec=avcodec_find_encoder(codecId);
    if(!codec) 
    {
        printf("[ff] Cannot find codec\n");
        return false;
    }
   prolog();
   res=avcodec_open(_context, codec); 
   if(res<0) 
    {   printf("[ff] Cannot open codec\n");
        return false;
    }
   
    // Now allocate colorspace
    int w,h;
    FilterInfo *info=source->getInfo();
    w=info->width;
    h=info->height;
    if(targetColorSpace!=ADM_COLOR_YV12)
    {
        colorSpace=new ADMColorspace(w,h,ADM_COLOR_YV12,targetColorSpace);
        if(!colorSpace)
        {
            printf("[ADM_jpegEncoder] Cannot allocate colorspace\n");
            return false;
        }
    }
    return true;
}
/**
    \fn getExtraData
    \brief

*/
bool             ADM_coreVideoEncoderFFmpeg::getExtraData(uint32_t *l,uint8_t **d)
{
    *l=_context->extradata_size;
    *d=_context->extradata;
    return true;

}
// EOF

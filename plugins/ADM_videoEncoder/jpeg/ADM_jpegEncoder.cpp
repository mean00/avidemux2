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
#include "DIA_factory.h"

static uint32_t Quantizer=2;
static ADM_colorspace color=ADM_COLOR_YUV422P; //ADM_COLOR_YUV422P; //ADM_COLOR_YV12;
/**
        \fn ADM_jpegEncoder
*/
ADM_jpegEncoder::ADM_jpegEncoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoder(src)
{
    printf("[jpegEncoder] Creating.\n");
    int w,h;
   
    w=getWidth();
    h=getHeight();

    image=new ADMImage(w,h);
    plane=(w*h*3)/2;
    _context = avcodec_alloc_context ();
    ADM_assert (_context);
    memset (&_frame, 0, sizeof (_frame));
    _frame.pts = AV_NOPTS_VALUE;
    _context->width = w;
    _context->height = h;
    _context->strict_std_compliance = -1;
    // For RGB we need a temp buffer...  
    rgbBuffer=new uint8_t [(w+7)*(h+7)*4];
}
/**
    \fn prolog

*/

bool ADM_jpegEncoder::prolog(void)
{
  int w=getWidth();
  int h=getHeight();

  switch(color)
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
    _frame.quality = (int) floor (FF_QP2LAMBDA * Quantizer+ 0.5);
    // Eval fps
    uint64_t f=source->getInfo()->frameIncrement;
    if(!f) f=40000;
    _context->time_base.den=f;
    _context->time_base.num=1000000;
    
    return true;
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
   prolog();
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
    if(color!=ADM_COLOR_YV12)
    {
        colorSpace=new ADMColorspace(w,h,ADM_COLOR_YV12,color);
        if(!colorSpace)
        {
            printf("[ADM_jpegEncoder] Cannot allocate colorspace\n");
            return false;
        }
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
uint8_t *from;
    if(source->getNextFrame(image)==false)
    {
        printf("[jpeg] Cannot get next image\n");
        return false;
    }
    prolog();
    switch(color)
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
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/
static const diaMenuEntry colorMenus[2]=
    {
	{ADM_COLOR_YUV422P,QT_TR_NOOP("YUV422")},
	{ADM_COLOR_YV12,QT_TR_NOOP("YUV420")},
};

bool         jpegConfigure(void)
{
uint32_t colorM;
    printf("[jpeg] Configure\n");
    colorM=(uint32_t)color;

    diaElemUInteger  q(&(Quantizer),QT_TR_NOOP("_Quantizer:"),2,31);
    diaElemMenu      c(&colorM,QT_TR_NOOP("_ColorSpace:"),2,colorMenus);

    diaElem *elems[2]={&q,&c};
    
  if( diaFactoryRun(QT_TR_NOOP("Mjpeg Configuration"),2 ,elems))
  {
    color=(ADM_colorspace)colorM;
    return false;
  }
  return true;
}
// EOF

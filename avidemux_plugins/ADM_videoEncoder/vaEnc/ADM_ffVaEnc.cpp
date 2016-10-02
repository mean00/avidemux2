/***************************************************************************
                          \fn ADM_ffNvEnc
                          \brief Front end for libavcodec Mpeg4 asp encoder
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
#include "ADM_ffVaEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C"
{
    #include "libavutil/opt.h"
}

ffvaenc_encoder VaEncSettings = VAENC_CONF_DEFAULT;

/**
        \fn ADM_ffVaEncEncoder
*/
ADM_ffVaEncEncoder::ADM_ffVaEncEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    //targetColorSpace=ADM_COLOR_YUV422P;
    ADM_info("[ffNvEncEncoder] Creating.\n");
     frameContext=NULL;
     deviceContext=NULL;
}

/**
    \fn ~ADM_ffVaEncEncoder
*/
ADM_ffVaEncEncoder::~ADM_ffVaEncEncoder()
{
    ADM_info("[vaEncoder] Destroying.\n");
    if(frameContext)
    {
        av_buffer_unref(&frameContext);
    }
    if(deviceContext)
    {
        av_buffer_unref(&deviceContext);
    }
}
/**
    \fn pre-open
*/
bool ADM_ffVaEncEncoder::configureContext(void)
{
    ADM_info("ConfigureContext\n");
    _context->bit_rate   = VaEncSettings.bitrate*1000;
    _context->rc_max_rate= VaEncSettings.max_bitrate*1000;
    _context->pix_fmt    = AV_PIX_FMT_VAAPI;        
    
    // allocate device context
    deviceContext=av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VAAPI);
    if(!deviceContext)
    {
        ADM_warning("Cannot allocate device context\n");
        return false;
    }
    // setup device context
    AVHWDeviceContext *devContext=(AVHWDeviceContext *)deviceContext->data;
    AVVAAPIDeviceContext *vaContext=(AVVAAPIDeviceContext *)devContext->hwctx;
    
    vaContext->display=admLibVA::getVADisplay();  // if we get here VA is operationnal
    
    int er=av_hwdevice_ctx_init(deviceContext);
    if(er)
    {
        ADM_warning("Cannot setup vaapi hw encoder (er=%d)\n",er);
        return false;
    }
    
    // Now setup the frame context..
    
    frameContext=av_hwframe_ctx_alloc(deviceContext);
    if(!frameContext)
    {
        ADM_warning("Cannot allocate frameContext\n");
        return false;
    }
    AVHWFramesContext *c=(AVHWFramesContext *)(frameContext->data);
    c->format=AV_PIX_FMT_VAAPI;
    c->sw_format=AV_PIX_FMT_YUV420P;
    c->width=(getWidth()+15)&~15;
    c->height=(getHeight()+15)&~15;;
    
    for(int i=0;i<VAENC_NB_SURFACES;i++)
    {
        ADM_vaSurface *surface=new ADM_vaSurface(c->width,c->height); // roundup ??
        surface->surface= admLibVA::allocateSurface(c->width,c->height);
        freeSurface.pushBack(surface);
    }
    c->initial_pool_size=VAENC_NB_SURFACES;
    
    
    er=av_hwframe_ctx_init(frameContext);
    if(er)
    {
        ADM_warning("Cannot setup vaapi hw frame (er=%d)\n",er);
        return false;
    }
    
    _context->hw_frames_ctx=frameContext;
    
    int w= getWidth();
    int h= getHeight();
    
    w=(w+15)&~15;
    h=(h+15)&~15;
       
  
    
    
    ADM_info("configureContext ok\n");
    return true;             
}

/**
    \fn setup
*/
bool ADM_ffVaEncEncoder::setup(void)
{
    ADM_info("setup\n");
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("h264_vaapi"))
    {
        ADM_info("[vaEncoder] Setup failed\n");
        return false;
    }

    ADM_info("[vaEncoder] Setup ok\n");
    
  
    return true;
}



/**
    \fn encode
*/
bool         ADM_ffVaEncEncoder::encode (ADMBitstream * out)
{
int sz,q;
ADM_vaSurface *s;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);
        if (sz<= 0)
        {
            ADM_info("[vaEncoder] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[vaEncoder] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }
    q=image->_Qp;

    if(!q) q=2;
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);
  
    s=freeSurface.pop();
    printf("Surface=%d\n",s->surface);
    if(!s->fromAdmImage(image))
    {
        ADM_warning("Cannot upload to surface\n");
        freeSurface.pushBack(s);
        return false;
    }
    inUseSurface.pushBack(s);
    _frame->reordered_opaque=image->Pts;
    _frame->data[0]=_frame->data[1]=_frame->data[2]=NULL;
    _frame->data[3]=(uint8_t *)s->surface;
    _frame->width=image->GetWidth(PLANAR_Y);
    _frame->height=image->GetHeight(PLANAR_Y);
    _frame->format=  AV_PIX_FMT_VAAPI;    
    sz=encodeWrapper(_frame,out);
    
    if(sz<0)
    {
        ADM_warning("[vaEncoder] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    return postEncode(out,sz);
}

/**
    \fn isDualPass

*/
bool         ADM_ffVaEncEncoder::isDualPass(void)
{   
    return false;
}

/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffVaEncConfigure(void)
{

        ffvaenc_encoder *conf=&VaEncSettings;

#define PX(x) &(conf->x)

        
        diaElemUInteger  bitrate(PX(bitrate),QT_TRANSLATE_NOOP("vaenc","Bitrate (kbps):"),1,50000);
        diaElemUInteger  maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("vaenc","Max Bitrate (kbps):"),1,50000);
          /* First Tab : encoding mode */
        diaElem *diamode[]={&bitrate,&maxBitrate};

        if( diaFactoryRun(QT_TRANSLATE_NOOP("vaenc","VAAPI H264 Encoder"),2,diamode))
        {
          
          return true;
        }
         return false;
}
// EOF

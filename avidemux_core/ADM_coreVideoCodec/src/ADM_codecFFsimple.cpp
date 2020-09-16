/***************************************************************************
    \file ADM_codecFFSimple.cpp
    \brief Simple decoder class
    \author mean fixounet@free.fr (c) 2010

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_codecFFsimple.h"
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_coreCodecMapping.h"


/**
    \fn decoderFFSimple
*/
decoderFFSimple::decoderFFSimple (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        : decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    hasBFrame=false;
    if(!_frame)
        return;
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    if(!c)
        return;

    AVCodecID id=c->codecId;
    AVCodec *codec=avcodec_find_decoder(id);
    if(!codec)
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Codec"),QT_TRANSLATE_NOOP("adm","Internal error finding codec 0x%x"),fcc);
        return;
    }
    codecId=id;
    if(id==AV_CODEC_ID_NONE)
        return;

    _context = avcodec_alloc_context3(codec);
    if(!_context)
        return;

    if(true==c->extraData)
    {
         _context->extradata = (uint8_t *) _extraDataCopy;
         _context->extradata_size = (int) extraDataLen;
    }
    if(true==c->refCopy)
        _refCopy=1;
    if(true==c->hasBFrame)
        hasBFrame=true;
 
    _context->width = _w;
    _context->height = _h;
    _context->pix_fmt = AV_PIX_FMT_YUV420P;
    _context->codec_tag = fcc;
    _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; 
    _context->error_concealment=3; 
    // Hack
    if(codecId==AV_CODEC_ID_TSCC || codecId==AV_CODEC_ID_CSCD)
    {
        ADM_warning("Forcing bit per coded sample to %d\n",bpp);
         _context->bits_per_coded_sample = bpp;
    }
     _context->get_format=ADM_FFgetFormat; 
     _context->opaque=this;
    //
    if (avcodec_open2(_context, codec, NULL) < 0)
    {
        printf("[lavc] Decoder init: %x video decoder failed!\n",fcc);
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Codec"),QT_TRANSLATE_NOOP("adm","Internal error opening 0x%x"),fcc);
        return;
    }else
    {
        printf("[lavc] Decoder init: %x video decoder initialized! (%s)\n",fcc,codec->long_name);
    }
    _initCompleted=true;
}
/**
    \fn admCreateFFSimple
*/
decoders *admCreateFFSimple(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
{
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    if(!c) return NULL;
    AVCodecID id=c->codecId;
    if(id==AV_CODEC_ID_NONE) return NULL;
    decoderFFSimple *ffdec=new decoderFFSimple(w,h,fcc,extraDataLen,extraData,bpp);
    if(ffdec->initialized())
        return (decoders *)ffdec;
    delete ffdec;
    ffdec=NULL;
    return NULL;
}
/**
    \fn admCoreCodecSupports
    \brief returns true if core has been compiled with feature
*/
bool admCoreCodecSupports(ADM_CORE_CODEC_FEATURE feat)
{
#ifdef USE_VDPAU
    if(feat==ADM_CORE_CODEC_FEATURE_VDPAU)
    {
        return true;
    }
#endif
#ifdef USE_XVBA
    if(feat==ADM_CORE_CODEC_FEATURE_XVBA)
    {
        return true;
    }
#endif
#ifdef USE_LIBVA
    if(feat==ADM_CORE_CODEC_FEATURE_LIBVA)
    {
        return true;
    }
#endif
#ifdef USE_DXVA2
    if(feat==ADM_CORE_CODEC_FEATURE_DXVA2)
    {
        return true;
    }
#endif
    return false;
}
// EOF


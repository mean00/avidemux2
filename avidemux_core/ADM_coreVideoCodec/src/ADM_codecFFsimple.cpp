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
decoderFFSimple::decoderFFSimple (uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp, bool staged)
        : decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    hasBFrame=false;
    codec = NULL;
    if(!_frame)
        return;
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    if(!c)
        return;

    AVCodecID id=c->codecId;
    codec=avcodec_find_decoder(id);
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
    _context->get_format=ADM_FFgetFormat;
    _context->opaque=this;

    if(!staged)
    {
        applyQuirks(id);
        _initCompleted = finish();
    }
}
/**
    \fn finish
*/
bool decoderFFSimple::finish(void)
{
    if (!codec || !_context)
        return false;
    if (avcodec_open2(_context, codec, NULL) < 0)
    {
        printf("[lavc] Decoder init: %x video decoder failed!\n",_fcc);
        GUI_Error_HIG(QT_TRANSLATE_NOOP("adm","Codec"),QT_TRANSLATE_NOOP("adm","Internal error opening 0x%x"),_fcc);
        return false;
    }
    printf("[lavc] Decoder init: %x video decoder initialized with %d thread(s)! (%s)\n",_fcc,_context->thread_count,codec->long_name);
    _initCompleted=true;
    return true;
}
/**
    \fn applyQuirks
    \brief All the codec-specific stuff which cannot be handled elsewhere
*/
void decoderFFSimple::applyQuirks(AVCodecID id)
{
    switch(id)
    {
        case AV_CODEC_ID_TSCC:
        case AV_CODEC_ID_CSCD:
            ADM_warning("Forcing bit per coded sample to %d\n",_bpp);
            _context->bits_per_coded_sample = _bpp; // Hack
            break;
        case AV_CODEC_ID_PRORES:
        case AV_CODEC_ID_DNXHD:
        case AV_CODEC_ID_FFV1:
            decoderMultiThread();
            if(_usingMT)
            {
                if(codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
                {
                    _context->thread_count = _threads;
                    _context->thread_type = FF_THREAD_SLICE;
                    ADM_info("Enabling slice-based multi-threading.\n");
                    break;
                }
                ADM_warning("Multi-threadig requested, but slice-based multi-threading unavailable.\n");
            }
            break;
        default:break;
    }
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


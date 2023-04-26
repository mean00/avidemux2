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
#include "DIA_coreToolkit.h"
#include "ADM_coreCodecMapping.h"


/**
    \fn decoderFFSimple
*/
decoderFFSimple::decoderFFSimple (uint32_t w, uint32_t h, uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData, uint32_t bpp, bool staged)
        : decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    hasBFrame=false;
    if(!_frame || !_packet)
        return;
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    if(!c)
        return;

    hasBFrame = c->hasBFrame;
    if(c->refCopy)
        _refCopy = 1;

    _setFcc = true;

    if(false == lavSetupPrepare(c->codecId))
        return;

    if(!staged)
    {
        applyQuirks();
        _initCompleted = lavSetupFinish();
    }
}

/**
    \fn applyQuirks
    \brief All the codec-specific stuff which cannot be handled elsewhere
*/
void decoderFFSimple::applyQuirks(void)
{
    switch(codecId)
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
                if(_codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
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


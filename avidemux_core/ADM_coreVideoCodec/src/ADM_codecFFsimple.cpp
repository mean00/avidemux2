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
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    hasBFrame=false;
    ADM_assert(c);
    CodecID id=c->codecId;
    ADM_assert(id!=CODEC_ID_NONE);
    uint8_t *extraCopy=NULL;
    if(extraDataLen)
    {
            extraCopy=(uint8_t *)malloc(extraDataLen+FF_INPUT_BUFFER_PADDING_SIZE);
            memset(extraCopy,0,extraDataLen+FF_INPUT_BUFFER_PADDING_SIZE);
            memcpy(extraCopy,extraData,extraDataLen);
    }
    if(true==c->extraData)
    {
         _context->extradata = (uint8_t *) extraCopy;
         _context->extradata_size = (int) extraDataLen;
    }
    if(true==c->refCopy)
        _refCopy=1;
    if(true==c->hasBFrame)
        hasBFrame=true;
    AVCodec *codec=avcodec_find_decoder(id);
    if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error finding codec 0x%x"),fcc);ADM_assert(0);} 
    codecId=id; 
    _context->workaround_bugs=1*FF_BUG_AUTODETECT +0*FF_BUG_NO_PADDING; 
    _context->error_concealment=3; \
    if (avcodec_open(_context, codec) < 0)  
                      { 
                            printf("[lavc] Decoder init: %x video decoder failed!\n",fcc); 
                            GUI_Error_HIG("Codec","Internal error opening 0x%x",fcc); 
                            ADM_assert(0); 
                    } 
                    else 
                    { 
                            printf("[lavc] Decoder init: %x video decoder initialized! (%s)\n",fcc,codec->long_name); 
                    } 
    if(extraCopy) free(extraCopy);
}
/**
    \fn admCreateFFSimple
*/
decoders *admCreateFFSimple(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
{
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    if(!c) return NULL;
    CodecID id=c->codecId;
    if(id==CODEC_ID_NONE) return NULL;
    return new decoderFFSimple(w,h,fcc,extraDataLen,extraData,bpp);
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
    return false;
}
// EOF


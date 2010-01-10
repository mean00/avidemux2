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
typedef struct
{
    const char *string;
    CodecID    codecId;
    bool       extraData;
    bool       refCopy;
}ffVideoCodec;

const ffVideoCodec ffCodec[]=
{

  {"SNOW",  CODEC_ID_SNOW,      false, false},
  {"cvid",  CODEC_ID_CINEPAK,   false, false},
  {"CRAM",  CODEC_ID_MSVIDEO1,  false, false},
  {"VP6F",  CODEC_ID_VP6F,      false, false},
  {"VP6A",  CODEC_ID_VP6A,      false, false},
  {"SVQ1",  CODEC_ID_SVQ1,      false, false},
  {"FLV1",  CODEC_ID_FLV1,      false, false},
  {"AMV",   CODEC_ID_AMV,       false, false},
  {"MJPG",  CODEC_ID_MJPEG,     false, false},
  {"mjpa",  CODEC_ID_MJPEG,     false, false},
  {"MJPB",  CODEC_ID_MJPEGB,    false, false},
  {"FPS1",  CODEC_ID_FRAPS,     false, false},
  {"cvid",  CODEC_ID_CINEPAK,   false, false},
// Need extradata
  {"WMV2", CODEC_ID_WMV2,       true, false},
  {"WMV1", CODEC_ID_WMV1,       true, false},
  {"WMV3", CODEC_ID_WMV3,       true, false},
  {"WVC1", CODEC_ID_VC1,        true, false},
  {"WMVA", CODEC_ID_VC1,        true, false},

  {"WMVA", CODEC_ID_DVVIDEO,    true, false},
// RefCopy
  {"FFV1", CODEC_ID_FFV1,       true, true},
  {"H263", CODEC_ID_H263,       false, true},
  {"MP42", CODEC_ID_MSMPEG4V2,  true, true},
  {"SVQ3", CODEC_ID_SVQ3,       true, true},


  //{"MJPB", CODEC_ID_CYUV,       true},
 // {"MJPB", CODEC_ID_THEORA),    true}

};
/**
    \fn getCodecIdFromFourcc
*/
static const ffVideoCodec *getCodecIdFromFourcc(uint32_t fcc)
{
    uint32_t n=sizeof(ffCodec)/sizeof(ffVideoCodec);
    for(int i=0;i<n;i++)
    {
        const ffVideoCodec *c=ffCodec+i;
        if(fourCC::check(fcc,(const uint8_t*)c->string))
            return c;
    }
    return NULL;
}

/**
    \fn decoderFFSimple
*/
decoderFFSimple::decoderFFSimple (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        : decoderFF(w,h,fcc,extraDataLen,extraData,bpp)
{
    const ffVideoCodec *c=getCodecIdFromFourcc(fcc);
    ADM_assert(c);
    CodecID id=c->codecId;
    ADM_assert(id!=CODEC_ID_NONE);
    if(true==c->extraData)
    {
         _context->extradata = (uint8_t *) extraData;
         _context->extradata_size = (int) extraDataLen;
    }
    if(true==c->refCopy)
        _refCopy=1;

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

// EOF


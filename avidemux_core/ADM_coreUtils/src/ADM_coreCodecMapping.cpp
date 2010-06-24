/***************************************************************************
                          \fn ADM_coreCodecMapping
                          \brief Map lavcodec id to fourcc as uint32_t /string
                          \author mean fixounet@free.fr (c) 2010
    
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
#include "ADM_includeFfmpeg.h"

#include "ADM_default.h"
#include "ADM_coreCodecMapping.h"
#include "ADM_codecType.h"
#include "fourcc.h"
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
  {"FFVH", CODEC_ID_FFVHUFF,    true, true},
  {"HFYU", CODEC_ID_HUFFYUV,    true, true},
  {"VC1 ", CODEC_ID_VC1,        true, true},


 //{CODEC_ID_FFVHUFF,"FFVH"},
//    {CODEC_ID_HUFFYUV,"HFYU"},
  //{"MJPB", CODEC_ID_CYUV,       true},
 // {"MJPB", CODEC_ID_THEORA),    true}
  {"xxxx", CODEC_ID_NONE, false,false}
};

/**
    \fn getCodecIdFromFourcc
    \brief get fourcc and encoder settings from fourcc (used by encoder)
*/
const ffVideoCodec *getCodecIdFromFourcc(uint32_t fcc)
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
    \fn ADM_codecIdFindByFourcc
    \brief get lav codec if from fourcc (used by muxer)
*/
CodecID ADM_codecIdFindByFourcc(const char *fcc)
{
    uint32_t fid=fourCC::get((uint8_t *)fcc);
    // Special cases
 if (isMSMpeg4Compatible (fid) == 1)
    {
      return CODEC_ID_MSMPEG4V3;
    }
  if (isDVCompatible(fid))//"CDVC"))
    {
      return CODEC_ID_DVVIDEO;
    }
  if (isH264Compatible (fid))
    {
        return CODEC_ID_H264;
    }
  if (isMpeg4Compatible (fid) == 1)
    {
      return CODEC_ID_MPEG4;
    }

    uint32_t nb=sizeof(ffCodec)/sizeof(ffVideoCodec);
    for(int i=0;i<nb;i++)
    {
        if(!strcmp(fcc,ffCodec[i].string)) return ffCodec[i].codecId;
    }
    return CODEC_ID_NONE;
}

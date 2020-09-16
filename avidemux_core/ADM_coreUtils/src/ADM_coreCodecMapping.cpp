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

#include "ADM_default.h"
#include "ADM_coreCodecMapping.h"
#include "ADM_codecType.h"
#include "fourcc.h"
const ffVideoCodec ffCodec[]=
{
                            // extradata // Refcopy // Can have  B frame
  {"LAGS",  AV_CODEC_ID_LAGARITH,  false, false,    false},    
  {"SNOW",  AV_CODEC_ID_SNOW,      false, false,    false},
  {"cvid",  AV_CODEC_ID_CINEPAK,   false, false,    false},
  {"CRAM",  AV_CODEC_ID_MSVIDEO1,  false, false,    false},
  {"VP6F",  AV_CODEC_ID_VP6F,      false, false,    false},
  {"VP6A",  AV_CODEC_ID_VP6A,      false, false,    false},
  {"VP8 ",  AV_CODEC_ID_VP8,       false, false,    false},
  {"VP9 ",  AV_CODEC_ID_VP9,       false, false,    false},
  {"SVQ1",  AV_CODEC_ID_SVQ1,      false, false,    false},
  {"FLV1",  AV_CODEC_ID_FLV1,      false, false,    false},
  {"AMV",   AV_CODEC_ID_AMV,       false, false,    false},
  {"MJPG",  AV_CODEC_ID_MJPEG,     false, false,    false},
  {"jpeg",  AV_CODEC_ID_MJPEG,     false, false,    false},
  {"mjpa",  AV_CODEC_ID_MJPEG,     false, false,    false},
  {"MJPB",  AV_CODEC_ID_MJPEGB,    false, false,    false},
  {"FPS1",  AV_CODEC_ID_FRAPS,     false, false,    false},
  {"cvid",  AV_CODEC_ID_CINEPAK,   false, false,    false},
  {"FICV",  AV_CODEC_ID_FIC,       false, false,    false},
  {"AVdn",  AV_CODEC_ID_DNXHD,     false, false,    false},
  {"tscc",  AV_CODEC_ID_TSCC,      false, false,    false},
  {"CSCD",  AV_CODEC_ID_CSCD,      false, false,    false},
  {"apch",  AV_CODEC_ID_PRORES,    false, false,    false},
  {"apcn",  AV_CODEC_ID_PRORES,    false, false,    false},
  {"apcs",  AV_CODEC_ID_PRORES,    false, false,    false},
  {"apco",  AV_CODEC_ID_PRORES,    false, false,    false},
  {"ap4h",  AV_CODEC_ID_PRORES,    false, false,    false},
// Need extradata
  {"av01", AV_CODEC_ID_AV1,        true, false,    false},
  {"ULY0", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"ULY2", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"ULY4", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"UQY2", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"ULH0", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"ULH2", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"ULH4", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"UMH2", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"UMH4", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"UMY2", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"UMY4", AV_CODEC_ID_UTVIDEO,    true, false,    false},
  {"WMV2", AV_CODEC_ID_WMV2,       true, false,    false},
  {"WMV1", AV_CODEC_ID_WMV1,       true, false,    false},
  {"WMV3", AV_CODEC_ID_WMV3,       true, false,    true},
  {"WVC1", AV_CODEC_ID_VC1,        true, false,    true},
  {"WMVA", AV_CODEC_ID_VC1,        true, false,    true},

  
// RefCopy
  {"FFV1", AV_CODEC_ID_FFV1,       true, true,    false},
  {"H263", AV_CODEC_ID_H263,       false, true,   false},
  {"MP42", AV_CODEC_ID_MSMPEG4V2,  true, true,    false},
  {"SVQ3", AV_CODEC_ID_SVQ3,       true, true,    false},
  {"FFVH", AV_CODEC_ID_FFVHUFF,    true, true,    false},
  {"HFYU", AV_CODEC_ID_HUFFYUV,    true, true,    false},
  {"VC1 ", AV_CODEC_ID_VC1,        true, true,    true},


 //{AV_CODEC_ID_FFVHUFF,"FFVH"},
//    {AV_CODEC_ID_HUFFYUV,"HFYU"},
  //{"MJPB", AV_CODEC_ID_CYUV,       true},
 // {"MJPB", AV_CODEC_ID_THEORA),    true}
  {"xxxx", AV_CODEC_ID_NONE, false,false,    false}
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
AVCodecID ADM_codecIdFindByFourcc(const char *fcc)
{
    uint32_t fid=fourCC::get((uint8_t *)fcc);
    // Special cases
 if (isMSMpeg4Compatible (fid) == 1)
    {
      return AV_CODEC_ID_MSMPEG4V3;
    }
  if (isDVCompatible(fid))//"CDVC"))
    {
      return AV_CODEC_ID_DVVIDEO;
    }
  if (isH264Compatible (fid))
    {
        return AV_CODEC_ID_H264;
    }
  if (isH265Compatible (fid))
    {
      return AV_CODEC_ID_HEVC;
    }
  if (isMpeg4Compatible (fid) == 1)
    {
      return AV_CODEC_ID_MPEG4;
    }
if (isVP9Compatible (fid) == 1)
    {
      return AV_CODEC_ID_VP9;
    }

    uint32_t nb=sizeof(ffCodec)/sizeof(ffVideoCodec);
    for(int i=0;i<nb;i++)
    {
        if(!strcmp(fcc,ffCodec[i].string)) return ffCodec[i].codecId;
    }
    return AV_CODEC_ID_NONE;
}

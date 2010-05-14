/***************************************************************************
    copyright            : (C) 2006 by mean
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


#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"


#include "ADM_mkv.h"

#include "mkv_tags.h"

typedef struct 
{
    const char *name;
    uint32_t isVideo;
    uint32_t audioCC;
    const char *videoCC;
}MKVCC;

MKVCC mkvCC[]=
{
  {"A_MPEG/L3",0,WAV_MP3,""},
  {"A_AC3",0,WAV_AC3,""}, 
  {"A_AAC/MPEG2/LC",0,WAV_AAC,""},
  {"A_AAC/MPEG4/LC/SBR",0,WAV_AAC,""},
  {"A_AAC/MPEG4/LC",0,WAV_AAC,""},
  {"A_PCM/INT/LIT",0,WAV_PCM,""},

  {"A_AAC",0,WAV_AAC,""},
  {"A_VORBIS",0,WAV_OGG_VORBIS,""},
  {"A_DTS",0,WAV_DTS,""},
  {"A_MPEG/L2",0,WAV_MP2,""},
  {"A_MPEG/L1",0,WAV_MP2,""},
  // Video
  {"V_MPEG2",1,0,"MPEG"}, // Mpeg2
  {"V_MPEG1",1,0,"MPEG"}, // Mpeg1
  {"V_MPEG4/MS/V3",1,0,"DIV3"}, // MS MPEG4 (Divx3)
  {"V_MPEG4/ISO/AVC",1,0,"AVC1"}, //H264
  {"V_MS/VFW/FOURCC",1,0,"VFWX"}, // Divx 2.
  {"V_MPEG4/ISO/ASP",1,0,"DIVX"},
  // Filler
  {"AVIDEMUX_RULES",1,0,"DIV2"} // DUMMY
};

uint32_t ADM_mkvCodecToFourcc(const char *codec)
{
int nbEntry=sizeof(mkvCC)/sizeof(MKVCC);
    for(int i=0;i<nbEntry;i++)
    {
      MKVCC *cur=&(mkvCC[i]);
      if(!strcmp(cur->name,codec))
      {
         if(cur->isVideo) return fourCC::get((uint8_t *)cur->videoCC);
               else return cur->audioCC;
      }
    }
    printf("[MKV] Warning type <%s> unkown!!\n",codec);
    return 0;
}
  //EOF

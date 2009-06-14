/***************************************************************************
                          GUI_mux.h  -  description
                             -------------------
    begin                : Wed Jul 3 2002
    copyright            : (C) 2002 by mean
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
#include "config.h"

#include <math.h>

#include "ADM_lavcodec.h"
#include "ADM_default.h"

#include "DIA_fileSel.h"
#include "DIA_factory.h"
#include "ADM_coreDemuxerMpeg/include/dmx_mpegstartcode.h"
#include "ADM_coreDemuxerMpeg/include/dmx_demuxerTS.h"


#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_ffmp43.h"

/**
      \fn DIA_tsSubs
      \brief  Convert TS subs to something different
*/
uint8_t  DIA_tsSub(void)
{
uint8_t ret=0;
const char  *tsFileName="/capture/grey/Grey_anatomy_2007_05_22_Avec_Le_Temp.mpg";
uint32_t pid=0x96;
uint8_t  packetBuffer[64*1024];
uint32_t packetLen;
uint32_t pts,dts;

    // Prepare our decoder
    decoderFFSubs *decoder=new decoderFFSubs(1);

    //
    ADMCompressedImage *binary=new ADMCompressedImage;
    AVSubtitle  sub;
    
    // 
    MPEG_TRACK track;
    track.pid=pid;
    // First create our demuxer
     dmx_demuxerTS *demuxer=new dmx_demuxerTS(1,&track,0,DMX_MPG_TS);
     if(!demuxer->open(tsFileName)) goto nd;
     binary->data=packetBuffer+2;
     memset(&sub,0,sizeof(sub));
     while(demuxer->readPes(packetBuffer,&packetLen, &dts,&pts))
     {
       printf("Found packet : %u \n",packetLen);
       binary->dataLength=packetLen-3; // -2 for stream iD, -1 for ????
       if(packetLen>5)
       {
        decoder->uncompress(binary,&sub);
        
        // And now cleanup the subs
        printf("Found %d rects to process\n",sub.num_rects);
        for(int i=0;i<sub.num_rects;i++)
        {
/*
          AVSubtitleRect *r=&(sub.rects[i]); 
              printf("x :%d\n",r->x);
              printf("y :%d\n",r->y);
              printf("w :%d\n",r->w);
              printf("h :%d\n",r->h);
*/
              // rgba_palette
              // Bitmap
        }
       }
       
     }
// Inlined

nd:
      delete demuxer;
      demuxer=NULL;
      delete decoder;
      decoder=NULL;
      delete binary;
      binary=NULL;
      return ret;
}
//EOF


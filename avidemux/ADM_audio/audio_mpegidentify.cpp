/***************************************************************************
                          audio_mpegidentify.cpp  -  description

	Identify the mpegaudio features (layer/fq/bitrate etc...)

                             -------------------
    begin                : Wed Dec 19 2001
    copyright            : (C) 2001 by mean
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stream.h>
#include "ADM_assert.h"
#include <math.h>

#include "config.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"
#include "aviaudio.hxx"

#include "ADM_mp3info.h" 

   
uint8_t mpegAudioIdentify(uint8_t *ptr, uint32_t maxLookUp, WAVHeader *header, uint8_t *tokens)
{
  MpegAudioInfo info;
  uint32_t offset=0;
  
        memset(&info,0,sizeof(info));
        if(!getMpegFrameInfo(ptr,maxLookUp,&info,NULL,&offset))
        {
          printf("Could not get sync in MP2/3 audio header\n");
          return 0; 
        }
        // Fill up wavheader
        if(info.mode==3)  header->channels=1;
        else    header->channels=2;
        
        header->frequency = info.samplerate;
        header->byterate = (1000 * info.bitrate) >> 3;   
        header->bitspersample=16;
        header->blockalign = 4;
        if(info.layer==3) header->encoding=WAV_MP3;
                else header->encoding=WAV_MP2;

        if(tokens)
        {
                tokens[0]=ptr[offset+1];
                tokens[1]=ptr[offset+2]&0xfd;
                tokens[2]=ptr[offset+3];
        }
#define DUMPX(x) printf(#x": %d\n",info.x);
                DUMPX( level);                    
                DUMPX( layer);
                DUMPX( samplerate);
                DUMPX( bitrate);
                DUMPX( level);
                                  
 
    return 1;
 }

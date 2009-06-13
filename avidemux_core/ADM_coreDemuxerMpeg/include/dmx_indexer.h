/***************************************************************************
                          Base class for Mpeg Demuxer
                             -------------------
                
    copyright            : (C) 2005 by mean
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

 #ifndef DMX_INDX
 #define DMX_INDX
 
typedef enum 
{
  ADM_STREAM_UNKNOWN=0,
  ADM_STREAM_MPEG_VIDEO, 
  ADM_STREAM_MPEG_AUDIO,
  ADM_STREAM_AC3,
  ADM_STREAM_DTS,
  ADM_STREAM_H264,
  ADM_STREAM_MPEG4,
  ADM_STREAM_AAC
}ADM_STREAM_TYPE;

typedef struct MPEG_TRACK
{
        uint16_t pid;
        uint16_t pes;
        // Only for audio...
        uint32_t channels;
        uint32_t bitrate;
        // for Both
        ADM_STREAM_TYPE streamType;
}MPEG_TRACK;

uint8_t dmx_indexer(const char *mpeg,const char *file,uint32_t preferedAudio,uint8_t autosync,uint32_t nbTracks,MPEG_TRACK *tracks);
        

#endif

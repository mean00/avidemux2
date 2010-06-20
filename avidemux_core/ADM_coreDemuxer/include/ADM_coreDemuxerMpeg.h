/***************************************************************************
    \file   ADM_coreDemuxerMpeg.h
    \brief  Common part for mpeg demuxer
    \author (C) 2010 by mean  : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CORE_DEMUXER_MPEG_H 
#define ADM_CORE_DEMUXER_MPEG_H
/**
    \struct dmxFrame
*/
typedef struct 
{
    uint64_t  startAt;
    uint32_t  index;
    uint8_t   type; // 1=I 2=P 3=B
    uint32_t  pictureType; // 3=Frame, 1=Top, 2=Bottom
    uint64_t  pts;
    uint64_t  dts;
    uint32_t  len;
    
}dmxFrame;

/**
    \struct ADM_mpgAudioSeekPoint
    \brief The dts are stored in rescaled us. Warning the low level getpacket is getting absolute 90 khzTick
*/
typedef struct
{            
      uint64_t position;
      uint64_t dts;
      uint32_t size;

}ADM_mpgAudioSeekPoint;
#endif

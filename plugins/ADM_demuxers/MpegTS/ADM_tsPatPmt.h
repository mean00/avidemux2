/** *************************************************************************
    \file ADM_tsPatPmt.cpp
    \brief Analyze pat & pmt
    copyright            : (C) 2007 by mean
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
#ifndef ADM_TS_PAT_PMT_H
#define ADM_TS_PAT_PMT_H

#define ADM_TS_MAX_EXTRADATA 256
/**
    \typedef ADM_TS_VIDEO_TYPE
*/
typedef enum
{
    ADM_TS_UNKNOWN=0,
    ADM_TS_MPEG2,
    ADM_TS_H264,
    ADM_TS_MPEG_AUDIO=10,
    ADM_TS_AC3,
    ADM_TS_AAC
}ADM_TS_TRACK_TYPE;
/**
    \typedef ADM_TS_TRACK
*/
typedef struct
{
    uint32_t          trackPid;
    ADM_TS_TRACK_TYPE trackType;
    uint32_t          extraDataLen;
    uint8_t           extraData[ADM_TS_MAX_EXTRADATA];
}ADM_TS_TRACK;


/**
    \fn TS_scanForPrograms
    \brief Analyze a stream and returns tracks within
    @param in:file File to open
    @param out:nbTracks Number of tracks found
    @param out:tracks, to be freed by delete [] by the caller
*/

bool TS_scanForPrograms(const char *file,uint32_t *nbTracks, ADM_TS_TRACK **tracks);

#endif
//EOF

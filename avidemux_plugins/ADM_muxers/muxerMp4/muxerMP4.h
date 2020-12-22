/***************************************************************************
                          oplug_vcdff.h  -  description
                             -------------------
    begin                : Sun Nov 10 2002
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
#ifndef ADM_MUXER_MP4
#define ADM_MUXER_MP4

#include "ADM_muxer.h"
#include "ADM_coreMuxerFfmpeg.h"
typedef enum
{
    MP4_MUXER_MP4,
    MP4_MUXER_PSP
}MP4_MUXER_TYPE;

typedef enum
{
    MP4_MUXER_OPT_NONE,
    MP4_MUXER_OPT_FASTSTART,
#ifndef MUXER_IS_MOV
    MP4_MUXER_OPT_FRAGMENT
#endif
}MP4_MUXER_OPTIMIZE;

typedef enum
{
    STANDARD=0,
    WIDE=1,
    UNI=2,
    CINEMA=3,
    CUSTOM=4
}MP4_MUXER_DAR;

typedef enum
{
    MP4_MUXER_ROTATE_0,
    MP4_MUXER_ROTATE_90,
    MP4_MUXER_ROTATE_180,
    MP4_MUXER_ROTATE_270
}MP4_MUXER_ROTATION;

typedef enum
{
    MP4_MUXER_CLOCK_FREQ_AUTO,
    MP4_MUXER_CLOCK_FREQ_24KHZ,
    MP4_MUXER_CLOCK_FREQ_25KHZ,
    MP4_MUXER_CLOCK_FREQ_30KHZ,
    MP4_MUXER_CLOCK_FREQ_50KHZ,
    MP4_MUXER_CLOCK_FREQ_60KHZ,
    MP4_MUXER_CLOCK_FREQ_90KHZ,
    MP4_MUXER_CLOCK_FREQ_180KHZ
}MP4_MUXER_CLOCK_FREQUENCIES;

#include "mp4_muxer.h"
extern mp4_muxer muxerConfig;

#ifdef MUXER_IS_MOV
#   define MOVCLASS muxerMov
#else
#   define MOVCLASS muxerMP4
#endif
class MOVCLASS : public muxerFFmpeg
{
protected:
                const char *getContainerName(void)
#ifdef MUXER_IS_MOV
                {return "MOV";}
#else
                {return "MP4";}
#endif
public:
                MOVCLASS();
        virtual ~MOVCLASS();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;
        virtual bool useGlobalHeader(void) {return true;}

};

#endif

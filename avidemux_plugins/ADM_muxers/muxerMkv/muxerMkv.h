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
#ifndef ADM_MUXER_MKV
#define ADM_MUXER_MKV

#include "ADM_muxer.h"
#include "ADM_coreMuxerFfmpeg.h"
#include "mkv_muxer.h"
extern mkv_muxer muxerConfig;

typedef enum
{
    OTHER=0,
    STANDARD=1,
    WIDE=2,
    UNI=3,
    CINEMA=4
}MKV_MUXER_DAR;

#ifdef MUXER_IS_WEBM
#   define MKVCLASS muxerWebm
#else
#   define MKVCLASS muxerMkv
#endif
class MKVCLASS : public muxerFFmpeg
{
protected:
        bool muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts);
        const char *getContainerName(void)
#ifdef MUXER_IS_WEBM
        {return "WebM";}
#else
        {return "Matroska";}
#endif
public:
                MKVCLASS();
        virtual ~MKVCLASS();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;
        virtual bool useGlobalHeader(void) {return true;}
};

#endif

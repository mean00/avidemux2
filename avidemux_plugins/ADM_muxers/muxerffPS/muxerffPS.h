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
#ifndef ADM_MUXER_ffPS
#define ADM_MUXER_ffPS

#include "ADM_muxer.h"
#include "ADM_coreMuxerFfmpeg.h"

typedef enum
{
    MUXER_VCD,
    MUXER_SVCD,
    MUXER_DVD
}psMuxingType;

typedef struct
{
    psMuxingType muxingType;
    bool         acceptNonCompliant;
}psMuxerConfig_s;

extern psMuxerConfig_s psMuxerConfig;

/**
    \fn class muxerffPS
*/
class muxerffPS : public muxerFFmpeg
{
protected:
        bool muxerRescaleVideoTime(uint64_t *time);
        bool muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts);
        
        bool muxerRescaleAudioTime(uint64_t *time,uint32_t fq);

public:
                muxerffPS();
        virtual ~muxerffPS();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

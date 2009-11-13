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
#ifndef ADM_MUXER_ffTS
#define ADM_MUXER_ffTS

#include "ADM_muxer.h"
#include "ADM_coreMuxerFfmpeg.h"
#include "ts_muxer.h"

extern ts_muxer tsMuxerConfig;

/**
    \fn class muxerffTS
*/
class muxerffTS : public muxerFFmpeg
{
protected:
        
        bool muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts);

public:
                muxerffTS();
        virtual ~muxerffTS();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

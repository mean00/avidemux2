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

class muxerMkv : public muxerFFmpeg
{
protected:
        bool muxerRescaleVideoTimeDts(uint64_t *time,uint64_t computedDts);
        const char *getContainerName(void) {return "Matroska";};
public:
                muxerMkv();
        virtual ~muxerMkv();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

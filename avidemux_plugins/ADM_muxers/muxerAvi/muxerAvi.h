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
#ifndef ADM_MUXER_AVI
#define ADM_MUXER_AVI

#include "ADM_muxer.h"
#include "op_aviwrite.hxx"
#include "ADM_audioClock.h"
#include "avi_muxer.h"


extern avi_muxer muxerConfig;

/**
    \class muxerAvi
*/
class muxerAvi : public ADM_muxer
{
protected:
        bool    setupAudio(int trackNumber, ADM_audioStream *audio);
        bool    setupVideo(ADM_videoStream *video);
        bool    fillAudio(uint64_t targetDts);
        aviWrite  writter;
        uint8_t   *audioBuffer;
        uint8_t   *videoBuffer;
        audioClock **clocks;
public:
                muxerAvi();
        virtual ~muxerAvi();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

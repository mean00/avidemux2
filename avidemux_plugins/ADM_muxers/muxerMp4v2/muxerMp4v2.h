/***************************************************************************
    \file muxerMp4v2.h
    \brief muxer using libmp4v2
    \author mean fixounet@free.fr 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_MUXER_MP4V2
#define ADM_MUXER_MP4V2

#include "ADM_muxer.h"
#include "ADM_audioClock.h"
#include "mp4v2/mp4v2.h"
/**
    \class muxerMp4v2
*/
class muxerMp4v2 : public ADM_muxer
{
protected:
        MP4FileHandle   handle;
        MP4TrackId      videoTrackId;
        MP4TrackId      *audioTrackIds;
        uint32_t        videoBufferSize;
        uint8_t         *videoBuffer[2];
        ADMBitstream    in[2];
        int             nextWrite;
protected:
        bool            setMpeg4Esds(void);
        bool            initVideo(void);
        bool            initAudio(void);
        bool            fillAudio(uint64_t targetDts);
static  uint64_t        timeScale(uint64_t timeUs);

public:
                muxerMp4v2();
        virtual ~muxerMp4v2();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

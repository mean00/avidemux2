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
    \class mp4v2AudioPacket
*/
#define AUDIO_BUFFER_SIZE 16*1024*2
#define MP4V2_MAX_JITTER (40*1000) // 40 ms
class mp4v2AudioPacket
{
    public:
         class mp4v2AudioBlock
         {
            public:
                    uint8_t     *buffer;
                    uint64_t    dts;
                    uint32_t    nbSamples;
                    uint32_t    sizeInBytes;
                    bool        present;
            public:
                  mp4v2AudioBlock() {buffer=new uint8_t[AUDIO_BUFFER_SIZE];present=false;}
                  ~mp4v2AudioBlock() {delete [] buffer;buffer=NULL;}
         };
            bool                eos;
            mp4v2AudioBlock     blocks[2];
            int                 nextWrite;
            audioClock          *clock;
            mp4v2AudioPacket() {eos=false;nextWrite=0;clock=NULL;}
            ~mp4v2AudioPacket() {if(clock) delete clock;clock=NULL;}

};
/**
    \class muxerMp4v2
*/
class muxerMp4v2 : public ADM_muxer
{
protected:
        MP4FileHandle   handle;
        MP4TrackId      videoTrackId;
        MP4TrackId      *audioTrackIds;
        mp4v2AudioPacket *audioPackets;
        uint32_t        videoBufferSize;
        uint8_t         *videoBuffer[2];
        ADMBitstream    in[2];
        int             nextWrite;
        uint64_t        audioDelay; // In fact videoDelay, but must be added to all audioTrack
protected:
        bool            setMpeg4Esds(void);
        bool            initVideo(void);
        bool            initAudio(void);
        bool            fillAudio(uint64_t targetDts);
static  uint64_t        timeScale(uint64_t timeUs);
        bool            loadAndToggleAudioSlot(int index);
        bool            writeAudioBlock(int index,mp4v2AudioPacket::mp4v2AudioBlock *block,uint64_t duration90);
protected:
        bool            addAc3(int index, WAVHeader *header);
public:
                muxerMp4v2();
        virtual ~muxerMp4v2();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;

};

#endif

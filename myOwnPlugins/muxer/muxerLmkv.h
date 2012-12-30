/***************************************************************************
    \file  muxerLmkv.h
    \brief muxer using libmkv
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
#ifndef ADM_MUXER_LMKV
#define ADM_MUXER_LMKV
#include "ADM_cpp.h"
#include "ADM_muxer.h"
#include "ADM_audioClock.h"
#include "ADM_paramList.h"

/**
    \class mp4v2AudioPacket
*/
#define AUDIO_BUFFER_SIZE 16*1024*2
#define MP4V2_MAX_JITTER (40*1000) // 40 ms
#if 0
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
#endif
/**
    \class muxerMp4v2
*/
class muxerLmkv : public ADM_muxer
{
protected:

protected: // video
protected: // audio
protected:
public:
                muxerLmkv();
        virtual ~muxerLmkv();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;
        virtual bool useGlobalHeader(void) {return true;}
                void setPercent(int percent);

};

#endif

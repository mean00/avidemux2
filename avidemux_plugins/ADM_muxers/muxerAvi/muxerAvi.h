/***************************************************************************
    \file   muxerAvi
    \brief  avi muxer class
    \author mean fixounet@free.Fr (c) 2002/2012
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
/**
    \enum set muxer type, i.e. generate openDml or plain avi
*/
enum
{
    AVI_MUXER_TYPE1=0,AVI_MUXER_AUTO=1,AVI_MUXER_TYPE2=2
};

#define AUDIO_BUFFER_SIZE 48000*6*sizeof(float)
extern avi_muxer muxerConfig;

class aviAudioPacket
{
    public:
            uint8_t     *buffer;
            uint64_t    dts;
            uint32_t    nbSamples;
            uint32_t    sizeInBytes;
            bool        present;
            bool        eos;

            aviAudioPacket() {buffer=new uint8_t[AUDIO_BUFFER_SIZE];eos=false;present=false;}
            ~aviAudioPacket() {delete [] buffer;buffer=NULL;}

};
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
        aviAudioPacket  *audioPackets;
        uint8_t         *videoBuffer;
        audioClock      **clocks;
        uint64_t        audioDelay;
public:
                muxerAvi();
        virtual ~muxerAvi();
        virtual bool open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a);
        virtual bool save(void) ;
        virtual bool close(void) ;
        virtual bool preferH264AnnexB(void) {return true;};
        virtual bool canDealWithTimeStamps(void) {return false;}; // need perfect audio

};

#endif

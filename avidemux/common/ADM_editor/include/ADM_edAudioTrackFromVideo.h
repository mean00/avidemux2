
/***************************************************************************
    \file  ADM_edAudioTrackFromVideo
    \brief Manage audio track(s) coming from a video

    (c) 2012 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_EDAUDIOTRACKFROMVIDEO
#define ADM_EDAUDIOTRACKFROMVIDEO

#include "ADM_edAudioTrack.h"

/**
    \class ADM_edAudioTrackFromVideo
*/
class ADM_edAudioTrackFromVideo : public ADM_edAudioTrack
{
protected:
                    uint32_t   _audioSeg;        // current segment we are in
                    int         myTrackNumber; // Track number in video
                    bool        switchToNextAudioSegment(void);
                    bool        refillPacketBuffer(void);
public:
                                 ADM_edAudioTrackFromVideo(ADM_audioStreamTrack *track,int trackNumber, ADM_Composer *parent);
            virtual            ~ADM_edAudioTrackFromVideo();
                    bool         destroyable() {return false;};
                    int          getMyTrackIndex() {return myTrackNumber; }


            virtual uint8_t     getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
            virtual bool         getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts);
            virtual bool         goToTime(uint64_t nbUs);
                    bool          getExtraData(uint32_t *l, uint8_t **d);
                    uint64_t      getDurationInUs(void);
                    uint8_t	   getAudioStream(ADM_audioStream **audio);
            virtual WAVHeader    *getInfo(void);
                    uint32_t      getOutputFrequency(void); // sbr
            virtual CHANNEL_TYPE * getChannelMapping(void );
                    bool           hasVBRAudio(void) {return true;};

              ADM_audioStreamTrack *getTrackAtVideoNumber(uint32_t refVideo);
              ADM_audioStreamTrack *getCurrentTrack(void);

            virtual ADM_edAudioTrackFromVideo *castToTrackFromVideo(void) {return this;}


};
#endif
//EOF

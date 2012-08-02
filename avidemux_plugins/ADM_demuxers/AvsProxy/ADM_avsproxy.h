/***************************************************************************
    \file ADM_avsproxy.h
    \author (C) 2007-2010 by mean  fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AVS_PROXY_H
#define AVS_PROXY_H
#include "ADM_Video.h"
#include "ADM_audioStream.h"
#include "ADM_avsproxy_net.h"
#define AVS_PROXY_DUMMY_FILE "::ADM_AVS_PROXY::" // warning this is duplicated in main app
/**
    \fn ADM_avsAccess
*/
class ADM_avsAccess : public ADM_audioAccess
{
protected:
                      
                avsNet           *network;
                WAVHeader        *wavHeader;
                uint64_t         duration;
                uint64_t         nextSample;
                uint8_t          *audioBuffer;
                uint64_t         sampleToTime(uint64_t sample);
                void             increment(uint64_t sample);
public:
                                  ADM_avsAccess(avsNet *net, WAVHeader *wav,uint64_t duration); 
                virtual           ~ADM_avsAccess();
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return true;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return true;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return true;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void);
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs);
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                bool              getExtraData(uint32_t *l, uint8_t **d){*l=0;*d=NULL;return true;};
};
/**
    \class avsHeader
*/
class avsHeader         :public vidHeader
{
    protected:
        uint64_t                    frameToTime(uint32_t frame);
        avsNet                      network;
        WAVHeader                   wavHeader;
        bool                        haveAudio;
        ADM_audioStream             *audioStream;
        ADM_avsAccess               *audioAccess;
    public:


        virtual   void 				Dump(void) {};

                                    avsHeader( void );
                                    ~avsHeader(  );
// AVI io
        virtual 	uint8_t			open(const char *name);
        virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________

virtual 	WAVHeader              *getAudioInfo(uint32_t i )  
                                    {
                                        if(true==haveAudio) 
                                            return &wavHeader;
                                        return NULL;
                                    };
virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio)
                                    {
                                            *audio=NULL;
                                            if(false==haveAudio) return 0;
                                            if(i) return 0;
                                            *audio= audioStream;
                                            return true;
                                    }
virtual     uint8_t                 getNbAudioStreams(void) 
                                    {
                                        if(true==haveAudio) 
                                                return 1;
                                        return 0;
                                    }

  //__________________________
  //				 video
  //__________________________
 virtual uint8_t                    setFlag(uint32_t frame,uint32_t flags);
 virtual uint32_t                   getFlags(uint32_t frame,uint32_t *flags);
 virtual uint8_t                    getFrame(uint32_t framenum,ADMCompressedImage *img);
 virtual uint64_t                   getTime(uint32_t frame);
         uint8_t                    getExtraHeaderData(uint32_t *len, uint8_t **data);
 virtual uint64_t                   getVideoDuration(void);

virtual   bool                      getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool                      setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);

          bool                      providePts(void) {return true;};
virtual   uint8_t                   getFrameSize(uint32_t frame,uint32_t *size);
};
#endif
//EOF

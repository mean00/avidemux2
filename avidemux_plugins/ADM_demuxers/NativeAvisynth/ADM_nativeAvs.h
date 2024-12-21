/***************************************************************************
                          ADM_pics.h  -  description
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002/2009 by mean
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
#pragma once
#include <vector>
using std::vector;
#include "ADM_Video.h"
#include "ADM_audioClock.h"
#include "avisynth.h"

class nativeAvsHeader;
/**
\fn nativeAvsAudio
*/
class nativeAvsAudio : public ADM_audioAccess
{
protected:
    audioClock       clock;
	WAVHeader        *wavHeader;
	uint64_t         duration;
	uint64_t         nextSample;
	nativeAvsHeader  *avs;
    int              sampleType;
public:
	                  nativeAvsAudio(nativeAvsHeader *net, WAVHeader *wav, int sampleType, uint64_t duration);
	virtual           ~nativeAvsAudio();
	/// Hint, the stream is pure CBR (AC3,MP2,MP3)
	virtual bool      isCBR(void) { return true; }
	/// Return true if the demuxer can seek in time
	virtual bool      canSeekTime(void) { return true; };
	/// Return true if the demuxer can seek by offser
	virtual bool      canSeekOffset(void) { return false; };
	/// Return true if we can have the audio duration
	virtual bool      canGetDuration(void) { return true; };
	/// Returns audio duration in us
	virtual uint64_t  getDurationInUs(void);
	/// Go to a given time
	virtual bool      goToTime(uint64_t timeUs);
	virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize, uint64_t *dts);
	bool              getExtraData(uint32_t *l, uint8_t **d) { *l = 0; *d = NULL; return true; };
};

/**
    \class nativeAvsHeader
    \brief Asf Demuxer
*/

class nativeAvsHeader         :public vidHeader
{
  protected:

    uint8_t                 close(void);    
    PClip                   *clip;


  public: // Shared with audio track
  public:
                                        nativeAvsHeader(void);
   virtual                              ~nativeAvsHeader();
                uint8_t                 open(const char *name);
      //__________________________
      //				 Audio
      //__________________________

    virtual 	WAVHeader              *getAudioInfo(uint32_t i )  ;
    virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio);
    virtual     uint8_t                 getNbAudioStreams(void);
    // Frames
      //__________________________
      //				 video
      //__________________________

    virtual 	uint8_t                 setFlag(uint32_t frame,uint32_t flags);
    virtual 	uint32_t                getFlags(uint32_t frame,uint32_t *flags);
    virtual 	uint8_t                 getFrameSize(uint32_t frame,uint32_t *size);
    virtual 	uint8_t                 getFrame(uint32_t framenum,ADMCompressedImage *img);

    virtual   void                       Dump(void);
    virtual   uint64_t                   getTime(uint32_t frameNum);
    virtual   uint64_t                   getVideoDuration(void);

    // Return true if the container provides pts informations
    virtual   bool                       providePts(void) {return false;};
    //
    virtual   bool                       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
    virtual   bool                       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);

			PClip						*getClip() { return clip; }
            bool                        getAudioPacket(uint64_t sample, uint8_t *buffer, uint32_t size);

protected:
              uint64_t                   frameNum2PTS(int frameNumber);
              ADM_pixelFormat             pixfrmt;
			  WAVHeader					 audioInfo;
			  nativeAvsAudio			 *audioAccess;
			  ADM_audioStream            *audioStream;
};



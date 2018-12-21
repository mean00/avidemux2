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
#include "avisynth.h"

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

protected:
              uint64_t                   frameNum2PTS(int frameNumber);

};



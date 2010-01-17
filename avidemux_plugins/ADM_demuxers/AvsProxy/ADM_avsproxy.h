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
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_Video.h"
#include "ADM_audioStream.h"
#include "ADM_avsproxy_net.h"
#define AVS_PROXY_DUMMY_FILE "::ADM_AVS_PROXY::" // warning this is duplicated in main app

/**
    \class avsHeader
*/
class avsHeader         :public vidHeader
{
    protected:
        uint64_t                    frameToTime(uint32_t frame);
        avsNet                      network;
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

virtual 	WAVHeader              *getAudioInfo(uint32_t i )  ;
virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio);
virtual     uint8_t                 getNbAudioStreams(void);

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

/***************************************************************************
     \file                     ADM_mxf.cpp
     \brief MXF demuxer
     \author mean, fixounet@free.fr (C) 2010
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef ADM_MXF_H
#define ADM_MXF_H

#include "ADM_assert.h"
#include "ADM_Video.h"
#include "ADM_audioStream.h"

/**
    \class mxfHeader
    \brief Demuxers for mxf format

*/
class mxfHeader         :public vidHeader
{
protected:


public:
//  static int checkFourCC(uint8_t *in, uint8_t *fourcc);

virtual   void 				Dump(void) {};

                            mxfHeader( void );
                            ~mxfHeader(  ) { };
// AVI io
virtual 	uint8_t			open(const char *name);
virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________


virtual 	WAVHeader              *getAudioInfo(uint32_t i ) {return NULL;} ;
virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio) {*audio=NULL;return 1;}
virtual     uint8_t                 getNbAudioStreams(void) {return 0;}

    
// Frames
  //__________________________
  //				 video
  //__________________________

virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags);
virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags);
virtual 	uint8_t  getFrame(uint32_t framenum,ADMCompressedImage *);

virtual   uint64_t                   getTime(uint32_t frameNum);
virtual   uint64_t                   getVideoDuration(void);
virtual 	uint8_t                 getFrameSize(uint32_t frame,uint32_t *size);

virtual   bool       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);


};


#endif



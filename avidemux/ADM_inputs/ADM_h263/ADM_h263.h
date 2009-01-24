/***************************************************************************
                          ADM_pics.h  -  description
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
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
 


#ifndef __H263HEADER__
#define __H263HEADER__
#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"


typedef struct h263Entry
{
	uint64_t offset;
	uint64_t size;
	uint8_t  intra;

}h263Entry;
class h263Header         :public vidHeader
{
protected:
       				
	  FILE 				*_fd;
	  h263Entry 			*_idx;
                  	
		
public:
//  static int checkFourCC(uint8_t *in, uint8_t *fourcc);

virtual   void 				Dump(void) {};

			h263Header( void ) {_fd=NULL;};
       		    	~h263Header(  ) { };
// AVI io
virtual 	uint8_t			open(const char *name);
virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________

virtual 	WAVHeader *getAudioInfo(void ) { return NULL ;} ;
virtual 	uint8_t			getAudioStream(AVDMGenericAudioStream **audio)
										{  *audio=NULL;return 0;};

// Frames
  //__________________________
  //				 video
  //__________________________

virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags);
virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags);
virtual 	uint8_t  getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img);
	     	 		
};
class mp4Header         :public h263Header
{
protected:
       				
	
                  	
		
public:
//  static int checkFourCC(uint8_t *in, uint8_t *fourcc);

// AVI io
virtual 	uint8_t			open(char *name);
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________


// Frames
  //__________________________
  //				 video
  //__________________________

};


#endif



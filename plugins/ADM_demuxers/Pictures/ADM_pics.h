/***************************************************************************
                          ADM_pics.h  -  description
                             -------------------
    
    copyright            : (C) 2002-2008 by mean/gruntster
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



#ifndef __PICHEADER__
#define __PICHEADER__
#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_audioStream.h"

enum
{
	PIC_BMP=1,
	PIC_JPEG=2,
	PIC_BMP2=3,
	PIC_PNG=4
};
/**
    \class picHeader
    \brief Demuxers for images (PNG/BMP/...)

*/
class picHeader         :public vidHeader
{
protected:
	char *_fileMask;

					uint32_t 		_nb_file;
                    uint32_t 		_first,_offset,_w,_h;

					uint32_t		*_imgSize;
					uint32_t 		_type;

					uint32_t    read32(FILE *fd);
					uint16_t    read16(FILE *fd);
					uint8_t     read8(FILE *fd);
					FILE* openFrameFile(uint32_t frameNum);

public:
//  static int checkFourCC(uint8_t *in, uint8_t *fourcc);

virtual   void 				Dump(void) {};

                            picHeader( void );
                            ~picHeader(  ) { };
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



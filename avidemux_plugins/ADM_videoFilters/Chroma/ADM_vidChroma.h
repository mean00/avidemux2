/***************************************************************************
                          ADM_vidChroma.h  -  description
                             -------------------
    begin                : Wed Aug 28 2002
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

#ifndef CHROMA_
#define CHROMA_

 class  ADMVideoChromaU:public AVDMGenericVideoStream
 {

 protected:


           virtual char 									*printConf(void);
			
			

 public:
 		
  					ADMVideoChromaU(  AVDMGenericVideoStream *in,CONFcouple *setup);  	          							
  					virtual ~ADMVideoChromaU();
		          virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
			  		ADMImage *data,uint32_t *flags);
				uint8_t configure( AVDMGenericVideoStream *instream) { UNUSED_ARG(instream); return 1;}
							
 }     ;

 class  ADMVideoChromaV:public AVDMGenericVideoStream
 {

 protected:


           virtual char 									*printConf(void);
			uint8_t											_reverse;

 public:
 		
  					ADMVideoChromaV(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					virtual ~ADMVideoChromaV();
		          virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,											ADMImage *data,uint32_t *flags);
					uint8_t configure( AVDMGenericVideoStream *instream) { UNUSED_ARG(instream); return 1;}
							
 }     ;

#endif

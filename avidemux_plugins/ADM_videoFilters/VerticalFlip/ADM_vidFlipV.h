/***************************************************************************
                          ADM_vidFlipV.h  -  description
                             -------------------
    begin                : Wed Nov 6 2002
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
#ifndef FLIP__
#define FLIP__

 class  ADMVideoFlipV:public AVDMGenericVideoStream
 {

 protected:
    		AVDMGenericVideoStream 	*_in;    	
           virtual char 									*printConf(void);
          
 public:
 		
  					ADMVideoFlipV(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					virtual ~ADMVideoFlipV();
		          virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          																	ADMImage *data,uint32_t *flags);
					virtual uint8_t configure( AVDMGenericVideoStream *instream) { UNUSED_ARG(instream); return 1;};          																	


 }     ;
#endif

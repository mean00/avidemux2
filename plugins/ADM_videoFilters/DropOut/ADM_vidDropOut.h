/***************************************************************************
                          ADM_vidDropOut.h  -  description
                             -------------------
    begin                : Mon Oct 7 2002
    copyright            : (C) 2002 by mean
    email                : RON
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __DROPOUT__
#define __DROPOUT__   
 
class  ADMVideoDropOut:public AVDMGenericVideoStream
 {

 protected:

        virtual char 				*printConf(void) ;
						VideoCache	*vidCache;
	 uint32_t				*_param;
 public:
 					
  						ADMVideoDropOut(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~ADMVideoDropOut();
		      virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
			virtual uint8_t getCoupledConf( CONFcouple **couples);

 }     ;
 
#endif

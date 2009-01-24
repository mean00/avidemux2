/***************************************************************************
                          ADM_vidPalShift.h  -  description
                             -------------------
    begin                : Sat Aug 24 2002
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
#ifndef PALSHIFT__
#define PALSHIFT__
 class  ADMVideoPalShift:public AVDMGenericVideoStream
 {

 protected:
    		
		VideoCache			*vidCache;
     virtual 	char 				*printConf(void);

		uint32_t			*_reverse;


 public:


  				ADMVideoPalShift(  AVDMGenericVideoStream *in,CONFcouple *setup);

  	virtual 		~ADMVideoPalShift();
	virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          								ADMImage *data,uint32_t *flags);
	virtual uint8_t configure( AVDMGenericVideoStream *instream) ;

 }     ;
#endif

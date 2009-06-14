/***************************************************************************
                          ADM_vidLargeMedian.h  -  description
                             -------------------
    begin                : Wed Jan 1 2003
    copyright            : (C) 2003 by mean
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
 #ifndef  __CONVPARAM__
 #define __CONVPARAM__
 typedef struct CONV_PARAM
 {
	 	uint32_t luma;
	  	uint32_t chroma;
	}CONV_PARAM;
 #endif
class  ADMVideoLargeMedian:public AVDMGenericVideoStream
 {

 protected:
    		AVDMGenericVideoStream 	*_in;
    		//uint8_t					*_uncompressed;
	virtual uint8_t 					doLine(uint8_t  *pred2,
										uint8_t *pred1,
   										uint8_t *cur,
                                          					uint8_t *next1,
                                           					uint8_t *next2,
   										uint8_t *out,
                       								uint32_t w)    ;
        	CONV_PARAM				*_param;
 public:
 		        virtual char 	*printConf(void) ;
  						
  						ADMVideoLargeMedian(  AVDMGenericVideoStream *in,CONFcouple *setup);

  			virtual 		~ADMVideoLargeMedian();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
									ADMImage *data,uint32_t *flags);
			virtual uint8_t 	configure( AVDMGenericVideoStream *instream) ;
			virtual uint8_t 	getCoupledConf( CONFcouple **couples);

 }     ;




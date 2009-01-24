/***************************************************************************
                          ADM_vidPartial.h  -  description
                             -------------------
    begin                : Mon Dec 30 2002
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
#ifndef PARTIAL__
#define PARTIAL__

#define MAXSONCONF 100

#include "ADM_vidPartial_param.h"
 class  ADMVideoPartial:public AVDMGenericVideoStream
 {

 protected:

      			AVDMGenericVideoStream 	*_son;
			PARTIAL_CONFIG			*_param;
 public:
 	virtual 	char 		*printConf(void);
  					ADMVideoPartial(  AVDMGenericVideoStream *in,CONFcouple *setup);
					ADMVideoPartial(  AVDMGenericVideoStream *in,VF_FILTERS tag,CONFcouple *setup);
  			virtual ~ADMVideoPartial();
		    	virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          																	ADMImage *data,uint32_t *flags);
			virtual uint8_t configure( AVDMGenericVideoStream *instream) ;
			virtual uint8_t	getCoupledConf( CONFcouple **couples);
 }     ;
 

#endif

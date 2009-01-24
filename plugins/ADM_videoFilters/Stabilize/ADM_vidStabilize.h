/***************************************************************************
                          ADM_vidStabilize.h  -  description
                             -------------------
    begin                : Mon Oct 7 2002
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
#ifndef __STABL__
#define __STABL__   
 class  ADMVideoStabilize:public AVDMGenericVideoStream
 {

 protected:
    		
        		uint32_t		*_param;
        		virtual char 	*printConf(void);
				VideoCache		*vidCache;


 public:

						ADMVideoStabilize(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					 	~ADMVideoStabilize();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          										ADMImage *data,uint32_t *flags);
			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
			virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
 }     ;
#endif

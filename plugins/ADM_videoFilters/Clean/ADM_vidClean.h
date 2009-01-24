/***************************************************************************
                          ADM_vidClean.h  -  description
                             -------------------
    begin                : Sun Apr 14 2002
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
 typedef struct
 {
	   	uint32_t radius,blend;
   }SMOOTH_PARAMS;


    class  AVDMVideoSmooth:public AVDMGenericVideoStream
 {

 protected:

    			uint8_t						*_unpack;
           virtual 	char 							*printConf(void);
    			SMOOTH_PARAMS				*_param;
 public:

  					AVDMVideoSmooth(  AVDMGenericVideoStream *in,CONFcouple *setup);
  			virtual 	~AVDMVideoSmooth();
		        virtual 	uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);
					uint8_t 	configure( AVDMGenericVideoStream *instream) ;
			 virtual 	uint8_t	getCoupledConf( CONFcouple **couples)		;

 }     ;



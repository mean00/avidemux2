/***************************************************************************
                          ADM_vidContrast.h  -  description
                             -------------------
    begin                : Sun Sep 22 2002
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
#ifndef _CONTRAST_
#define _CONTRAST_

typedef struct 
{
		float 	coef;
		int32_t  offset;
		uint32_t	doLuma;
		uint32_t	doChromaU;
		uint32_t	doChromaV;
		
	
	}CONTRAST_PARAM;
	
  uint8_t buildContrastTable( float coef,int8_t off,	
  								uint8_t *tableFlat,uint8_t *tableNZ);
  uint8_t doContrast(uint8_t *in,uint8_t *out,uint8_t *table,
												uint32_t w,uint32_t h);


 class  ADMVideoContrast:public AVDMGenericVideoStream
 {

 protected:
				virtual char 					*printConf(void);
				CONTRAST_PARAM					*_param;
				uint8_t						_tableFlat[256];
				uint8_t						_tableNZ[256];

 public:

					ADMVideoContrast(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					virtual ~ADMVideoContrast();
		          virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          																	ADMImage *data,uint32_t *flags);
					virtual uint8_t 	configure( AVDMGenericVideoStream *instream) ;

 					virtual uint8_t	getCoupledConf( CONFcouple **couples);
 				
 }     ;
#endif

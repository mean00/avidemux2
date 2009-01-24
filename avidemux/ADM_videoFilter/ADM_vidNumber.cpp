/***************************************************************************
                          ADM_vidNumber.cpp  -  Add frame number 
                             -------------------

    copyright            : (C) 2008 by mean
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

#include "config.h"
#include "ADM_default.h"
#include "ADM_videoFilter.h"
#include "ADM_videoFilterDynamic.h"

static FILTER_PARAM flipParam={0,{""}};

 class  ADMVideoNumber:public AVDMGenericVideoStream
 {

 protected:
    		AVDMGenericVideoStream 	*_in;    	
           virtual char 									*printConf(void);
          
 public:
 		
  					ADMVideoNumber(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					virtual ~ADMVideoNumber();
		          virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          																	ADMImage *data,uint32_t *flags);
					virtual uint8_t configure( AVDMGenericVideoStream *instream) { UNUSED_ARG(instream); return 1;};          																	


 }     ;
SCRIPT_CREATE(number_script,ADMVideoNumber,flipParam);
BUILD_CREATE(number_create,ADMVideoNumber);
 

char *ADMVideoNumber::printConf( void )
{
    ADM_FILTER_DECLARE_CONF(" Add Frame Number");
        
}

ADMVideoNumber::ADMVideoNumber(  AVDMGenericVideoStream *in,CONFcouple *setup)
{
    UNUSED_ARG(setup);
 	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info)); 	
  	_info.encoding=1;	
}
ADMVideoNumber::~ADMVideoNumber()
{
  
}
uint8_t ADMVideoNumber::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{

	if(frame>= _info.nb_frames) return 0;
	// read uncompressed frame
	if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;
    char f[50];
    snprintf(f,49,"%08u",frame);
    drawString(data, 2, 2, f); 
        
	return 1;
}

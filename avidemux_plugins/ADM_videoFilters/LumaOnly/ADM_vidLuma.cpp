/***************************************************************************
                          ADM_vidLuma.cpp  -  description
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

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidLuma.h"


static FILTER_PARAM nullParam={0,{""}};

//********** Register chunk ************
VF_DEFINE_FILTER(ADMVideoLuma,nullParam,
                lumaonly,
                QT_TR_NOOP("Luma only"),
                1,
                VF_COLORS,
                QT_TR_NOOP("Convert picture to greyscale (black and white)."));
//****************************************
char *ADMVideoLuma::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Luma only");
        
}

ADMVideoLuma::ADMVideoLuma(  AVDMGenericVideoStream *in,CONFcouple *setup)
{

 	_in=in;		
   	memcpy(&_info,_in->getInfo(),sizeof(_info));  		
	
		if(setup)
		{
			 _param=(void *)setup;
		}	
			else 			
		{	// default parameter	
				_param= NULL;
		}				
					 	
  _info.encoding=1;

  	  	
}
ADMVideoLuma::~ADMVideoLuma()
{
 	
}
uint8_t ADMVideoLuma::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{

	if(frame>= _info.nb_frames) return 0;
	// read uncompressed frame
	if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;

	uint32_t sz;
			
	sz=_info.width*_info.height;
	memset(UPLANE(data),128,sz>>2);
	memset(VPLANE(data),128,sz>>2);

      return 1;
}





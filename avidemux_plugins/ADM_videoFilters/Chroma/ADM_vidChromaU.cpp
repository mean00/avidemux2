/***************************************************************************
                          ADM_vidChroma.cpp  -  description
                             -------------------
    begin                : Wed Aug 28 2002
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

#include "ADM_vidChroma.h"


static FILTER_PARAM nullParam={0,{""}};



VF_DEFINE_FILTER(ADMVideoChromaU,nullParam,
				chromau,
				QT_TR_NOOP("Chroma U"),
				1,
				VF_COLORS,
				QT_TR_NOOP("Keep chroma U only."));

char *ADMVideoChromaU::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" chroma u only");
        
}

//_______________________________________________________________

ADMVideoChromaU::ADMVideoChromaU(
									AVDMGenericVideoStream *in,CONFcouple *setup)
{
    UNUSED_ARG(setup);

	_in=in;		
	memcpy(&_info,_in->getInfo(),sizeof(_info));  			 	
	_info.encoding=1;
	
  	  	
}
ADMVideoChromaU::~ADMVideoChromaU()
{
 	
	
 	
}

//
//	Remove y and v just keep U and expand it
//
   uint8_t ADMVideoChromaU::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{				
uint32_t w,x;
uint32_t page;
		if(frame>= _info.nb_frames) return 0;
       		if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;
		
		page= _info.width*_info.height;
		*len=(page*3)>>1;


		// now expand  u
		uint8_t *y,*v,*y2;

		y=YPLANE(data);
		y2=y+_info.width;
		v=UPLANE(data);
		for(w= _info.height>>1;w>0;w--)
		{
			for(x= _info.width>>1;x>0;x--)
			{
				*y=*v;
				*y2=*v;
				*(y+1)=*v;
				*(y2+1)=*v;
				v++;
				y+=2;
				y2+=2;
			}
                	y+=_info.width;
			y2+=_info.width;
       		 }

		 // Remove chroma u & v
		 memset(UPLANE(data),0x80,page>>2);
		 memset(VPLANE(data),0x80,page>>2);
}



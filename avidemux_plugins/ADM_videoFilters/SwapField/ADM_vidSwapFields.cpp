/***************************************************************************
                          Swap Fields.cpp  -  description
                             -------------------
Swap each line  (shift up for odd, down for even)


    begin                : Thu Mar 21 2002
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
#include "ADM_vidFieldUtil.h"
#include "ADM_vidSwapFields.h"


static FILTER_PARAM swapParam={0,{""}};

VF_DEFINE_FILTER(AVDMVideoSwapField,swapParam,
    swapfields,
                QT_TR_NOOP("Swap fields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Swap top and bottom fields."));
//********** Register chunk ************

char *AVDMVideoSwapField::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Swap fields");
 
}
//_______________________________________________________________
AVDMVideoSwapField::AVDMVideoSwapField(
									AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_uncompressed=new ADMImage(_info.width,_info.height);


}
// ___ destructor_____________
AVDMVideoSwapField::~AVDMVideoSwapField()
{
 	delete  _uncompressed;
	_uncompressed=NULL;

}
// ___ destructor_____________

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoSwapField::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//static Image in,out;
			if(frame>=_info.nb_frames) return 0;


			// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

		uint32_t w=_info.width;
		uint32_t h=_info.height;
		uint32_t page=w*h;
		uint32_t stride;

		// copy u & v
		memcpy(UPLANE(data),UPLANE(_uncompressed),page>>2);
		memcpy(VPLANE(data),VPLANE(_uncompressed),page>>2);
		
		uint8_t *odd,*even,*target,*target2;

		even=YPLANE(_uncompressed);
		odd=even+w;
		target=YPLANE(data);
		target2=YPLANE(data)+w;
		stride=2*w;

		h>>=1;
		for(;h--;h>0)
		{
			memcpy(target,odd,w);
			memcpy(target2,even,w);
			target+=stride;
			target2+=stride;
			odd+=stride;
			even+=stride;
		}
		data->copyInfo(_uncompressed);

      return 1;
}


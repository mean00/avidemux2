/***************************************************************************
                          ADM_vidDeinterlace.cpp  -  description
                             -------------------
	Strongly inspired by Donal Graft deinterlacer
 	Could be using some MMX
  	Should be faster than the original due to YV12 colorspace

20-Aug-2002 : Ported also the MMX part

    begin                : Sat Apr 20 2002
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

#include"ADM_vidDeinterlace.h"

#include "DIA_factory.h"

static FILTER_PARAM deintParam={2,{"motion_trigger","blend_trigger"}};

//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoDeinterlace,deintParam,
    deinterlace,
                QT_TR_NOOP("Deinterlace"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Mask interlacing artifacts. Port of Smart deinterlace."));
//********** Register chunk ************

//_______________________________________________________________


ADMVideoDeinterlace::~ADMVideoDeinterlace()
{
 	
	delete _uncompressed;
	_uncompressed=NULL;
}
ADMVideoDeinterlace::ADMVideoDeinterlace(  AVDMGenericVideoStream *in,CONFcouple *couples)
		:ADMVideoFields(in,couples)
{

	_uncompressed=new ADMImage(_info.width,_info.height);
}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t ADMVideoDeinterlace::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//uint8_t *dst,*dstu,*dstv,*srcu,*srcv;
uint32_t uvlen;
		
		if(frame>=_info.nb_frames) return 0;
		
								
		// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;         	
		
		uvlen=    _info.width*_info.height;
		*len= (uvlen*3)>>1;       			

		// No interleaving detected
		if(!hasMotion(_uncompressed))
           	{
			data->duplicate(_uncompressed);
			
		}
		else
		{
			//printf("Blending\n");
			doBlend(_uncompressed,data);
			memcpy(UPLANE(data),UPLANE(_uncompressed),uvlen>>2);
			memcpy(VPLANE(data),VPLANE(_uncompressed),uvlen>>2);
			data->copyInfo(_uncompressed);
		}
		return 1;
}



char *ADMVideoDeinterlace::printConf(void)
{
 		return (char *)"Deinterlace";;
}





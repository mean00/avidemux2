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
#include "ADM_vidSwapSmart.h"
#include "ADM_interlaced.h"

#define aprintf(...) {}

static FILTER_PARAM nullParam={0,{""}};
//********** Register chunk ************
//REGISTERX(VF_INTERLACING, "smartswapfield",QT_TR_NOOP("Smart swap fields"),QT_TR_NOOP(
    //"Smartly swap fields. Needed when field order changes."),VF_SMARTSWAPFIELDS,1,swapsmart_create,swapsmart_script);
VF_DEFINE_FILTER(AVDMVideoSwapSmart,nullParam,
    smartswapfield,
                QT_TR_NOOP("Smart swap fields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Smartly swap fields. Needed when field order changes."));
//********** Register chunk ************


char *AVDMVideoSwapSmart::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Smart Swap fields");
        
}

//_______________________________________________________________
AVDMVideoSwapSmart::AVDMVideoSwapSmart(
									AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));	
	_uncompressed=new ADMImage(_info.width,_info.height);



}
// ___ destructor_____________
AVDMVideoSwapSmart::~AVDMVideoSwapSmart()
{
 	delete  _uncompressed;
	_uncompressed=NULL;

}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoSwapSmart::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//static Image in,out;
			if(frame>=_info.nb_frames) return 0;


			// read uncompressed frame
       		if(!_in->getFrameNumberNoAlloc(frame, len,data,flags)) return 0;

		uint32_t w=_info.width;
		uint32_t h=_info.height;
		uint32_t page=w*h;
		uint32_t stride;



		uint8_t *odd,*even,*target,*target2;

		even=YPLANE(data);
		odd=even+w;
		target=YPLANE(_uncompressed);
		target2=_uncompressed->data+w;
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
		// now we have straight and swapped
		// which one is better ?
		uint32_t s,m;
		s=      ADMVideo_interlaceCount( YPLANE(data),_info.width, _info.height);
		m=      ADMVideo_interlaceCount( YPLANE(_uncompressed),_info.width, _info.height);

		aprintf(" %lu straight vs %lu swapped => %s\n",s,m,(s*2>m*3?"swapped":"straight"));

		// if swapped <= 66 % straight

		if(s*2>m*3)  // swapped is better
		{
			memcpy(YPLANE(data),YPLANE(_uncompressed),page);

		}

      return 1;
}



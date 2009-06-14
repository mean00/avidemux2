/***************************************************************************
                       Pulldown : Duplicate frame fields to convert
		       	24 fps to 30 fps movie

			1 2 3 4       1  2  3  4  4
			1 2 3 4 --> 1  2  2  3  4


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
#include "ADM_interlaced.h"
#include "ADM_vidPulldown.h"

#define aprintf(...) {}


static FILTER_PARAM swapParam={0,{""}};
//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoPullDown,swapParam,
                pulldown,
                QT_TR_NOOP("Pulldown"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Convert 24 fps to 30 fps by repeating fields."));
//********** Register chunk ************

char *ADMVideoPullDown::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Pulldown");
        
}
//_______________________________________________________________
ADMVideoPullDown::ADMVideoPullDown(
									AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_info.fps1000=(_info.fps1000*5)/4;
	_info.nb_frames=(_info.nb_frames*5)/4;
	for(uint32_t i=0;i<5;i++)
	{
		_uncompressed[i]=new ADMImage(_info.width,_info.height);
	}
	_cacheStart=0xfffffff;
}
// ___ destructor_____________
ADMVideoPullDown::~ADMVideoPullDown()
{
	for(uint32_t i=0;i<5;i++)
	{
 		delete  _uncompressed[i];
	}
}


uint8_t ADMVideoPullDown::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//static Image in,out;
			if(frame>=_info.nb_frames)
			{
				printf("out of bound frame (%lu / %lu)\n",frame,_info.nb_frames);
				return 0;
			}

		uint32_t w=_info.width;
		uint32_t h=_info.height;
		uint32_t page=w*h;
		uint32_t i;

		uint32_t target;
		uint32_t loop=0;

		*len=(page*3)>>1;

cont:

		target=frame-((frame)%5);
		// got it ?
		if(_cacheStart==target)
		{
			uint32_t index;

			aprintf("Filter: It is in cache...(cachestart=%lu)\n",_cacheStart);
			index=frame%5;
			aprintf("getting %lu)\n",index);
			memcpy(YPLANE(data),YPLANE(_uncompressed[index]),page);
			memcpy(UPLANE(data),UPLANE(_uncompressed[index]),page>>2);
			memcpy(VPLANE(data),VPLANE(_uncompressed[index]),page>>2);
			*flags=0;			
			return 1;
		}
		else
		{
			aprintf("Not in cache...\n");
		}
		// Else ask the 5 corresponding frame
		_cacheStart=target;
		target=(target*4)/5;

		uint32_t dflags,dlen;
#define GET_FRAME(x,y) if(!_in->getFrameNumberNoAlloc(x, &dlen,_uncompressed[y],&dflags)) \
 {\
				 	 	printf("Cannot get frame %lu\n",x);\
					 	return 0;     \
				 }

		GET_FRAME(target+0,0);
		GET_FRAME(target+1,1);
		GET_FRAME(target+2,3);
		GET_FRAME(target+3,4);
		// copy chroma 1->2
		memcpy(UPLANE(_uncompressed[2]),UPLANE(_uncompressed[1]),page>>2);
		memcpy(VPLANE(_uncompressed[2]),VPLANE(_uncompressed[1]),page>>2);
#define COPY_FIELD \
		for(uint32_t y=0;y<_info.height>>1;y++) \
		{ \
			memcpy(out,in,_info.width); \
			in+=_info.width<<1; \
			out+=_info.width<<1; \
		}

		// now we merge 1 & 3 into 2

		uint8_t *in,*out;
		in=YPLANE(_uncompressed[1]);
		out=YPLANE(_uncompressed[2]);
		COPY_FIELD;


		// merge 3->2
		//
		//	0 1 x 2 3
		//      0 1 x 2 3
		//
		//  0 1 1 2 3
		//  0 1 X 2 3
		in=YPLANE(_uncompressed[3])+w;
		out=YPLANE(_uncompressed[2])+w;
		COPY_FIELD;
		//  0 1 1 2 3
		//  0 1 2 2 3
		in=YPLANE(_uncompressed[4])+w;
		out=YPLANE(_uncompressed[3])+w;
		//  0 1 1 2 3
		//  0 1 2 3 3
		COPY_FIELD;
		goto cont;
		return 1;
}




/***************************************************************************
                          ADM_vidPalShift.cpp  -  description
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
#include "ADM_vidPalShift.h"
#include "DIA_factory.h"

static FILTER_PARAM nullParam={0,{""}};


//********** Register chunk ************
//REGISTERX(VF_INTERLACING, "palfieldshift",QT_TR_NOOP("PAL field shift"),QT_TR_NOOP(
    //"Shift fields by one. Useful for some PAL movies."),VF_PALSHIFT,1,addPALShift_create,addPALShift_script);
VF_DEFINE_FILTER(ADMVideoPalShift,nullParam,
    palfieldshift,
                QT_TR_NOOP("PAL field shift"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Shift fields by one. Useful for some PAL movies."));
//********** Register chunk ************


char *ADMVideoPalShift::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" PAL field shift");
        
}

ADMVideoPalShift::ADMVideoPalShift(  AVDMGenericVideoStream *in,CONFcouple *setup)
{
	UNUSED_ARG(setup);
	

	_reverse=NULL;
 	_in=in;		
	memcpy(&_info,_in->getInfo(),sizeof(_info));  		
	

	_reverse=new uint32_t;;
	*_reverse=1;
	
  	_info.encoding=1;

	vidCache=new VideoCache(5,in);


  	  	
}
 uint8_t ADMVideoPalShift::configure( AVDMGenericVideoStream *instream) 
{
  
  diaElemToggle chroma(_reverse,QT_TR_NOOP("_Try reverse"));
    
    diaElem *elems[]={&chroma};
  
    return diaFactoryRun(QT_TR_NOOP("Pal Field Shift"),sizeof(elems)/sizeof(diaElem *),elems);

} 
ADMVideoPalShift::~ADMVideoPalShift()
{
	delete vidCache;
	delete _reverse;
}
uint8_t ADMVideoPalShift::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{

uint32_t full,half;
ADMImage *cur,*next;

		full=_info.width*_info.height;
		half=full>>2;

		if(frame>=_info.nb_frames) return 0;
		
		cur=vidCache->getImage(frame);
		if(!cur) return 0;
			
		// for first or last frame do nothing
		if(!frame || frame==_info.nb_frames-1)
		{

			data->duplicate(cur);
			vidCache->unlockAll();
			return 1;	
		}				
								
		// copy u & v as they are			

		memcpy(UPLANE(data),UPLANE(cur),half);
		memcpy(VPLANE(data),VPLANE(cur),half);

		// now copy odd field from framei and even frame from frame i-1		
		// OR the other way around
		uint32_t dline=_info.width;
		
		next=vidCache->getImage(frame+1);
		if(!next) return 0;

		//
		uint8_t *src,*dst,*src2;
		if(!*_reverse)
		{
			src2=YPLANE(cur)+dline;
			src=YPLANE(next);
			dst=YPLANE(data);
		}
		else
		{
			src2=YPLANE(next)+dline;
			src=YPLANE(cur);
			dst=YPLANE(data);
		
		}
		for(uint32_t y=(_info.height>>1);y>0;y--)
		{
                	memcpy(dst,src, dline);
			dst+=dline;
                	memcpy(dst,src2, dline);
			dst+=dline;
			src+=dline*2;
			src2+=dline*2;
		}
		
	vidCache->unlockAll();
			data->copyInfo(cur);
      return 1;
}


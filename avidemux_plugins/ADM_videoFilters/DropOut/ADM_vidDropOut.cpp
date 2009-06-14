/***************************************************************************
                          ADM_vidDropOut.cpp  -  description
                             -------------------
    begin                : Mon Oct 7 2002
    copyright            : (C) 2002 by Ron Reithoffer 
    email                : <ron.reithoffer@chello.at>
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
#include "ADM_vidDropOut.h"
#include "DIA_factory.h"
#define aprintf(...) {}
static FILTER_PARAM dropParam={1,{"threshold"}};

//********** Register chunk ************

VF_DEFINE_FILTER(ADMVideoDropOut,dropParam,
                drop,
                QT_TR_NOOP("Drop"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Drop damaged fields (e.g. from VHS capture)."));
//********** Register chunk ************

//extern uint8_t distMatrix[256][256];
//extern uint32_t fixMul[16];

//_______________________________________________________________
 

char  *ADMVideoDropOut::printConf(void)
{
	ADM_FILTER_DECLARE_CONF(" DropOut :%ld",*_param);
        
}
uint8_t  GUI_getIntegerValue(int *valye, int min, int max, char *title);	
uint8_t ADMVideoDropOut::configure(AVDMGenericVideoStream *instream)
{
	_in=instream;
        
         diaElemUInteger chroma(_param,QT_TR_NOOP("DropOut Threshold"),1,255);    
         diaElem *elems[]={&chroma};
  
    return diaFactoryRun(QT_TR_NOOP("Drop Out"),sizeof(elems)/sizeof(diaElem *),elems);
}
ADMVideoDropOut::~ADMVideoDropOut()
{
	DELETE(_param);
	delete vidCache;
	vidCache=NULL;
}

//--------------------------------------------------------	
ADMVideoDropOut::ADMVideoDropOut(AVDMGenericVideoStream *in,CONFcouple *couples)
{

  
	_in=in;
  	_info.encoding=1;
	if(couples)
	{
  		_param=NEW( uint32_t);
		couples->getCouple((char *)"threshold",(uint32_t *)_param);
	}
	else
	{
		_param=NEW( uint32_t);
		*_param=30;
	}
	vidCache=new VideoCache(4,_in);
	memcpy(&_info,_in->getInfo(),sizeof(_info));
	
 
}

uint8_t	ADMVideoDropOut::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(1);
			(*couples)->setCouple((char *)"threshold",(*_param));
			return 1;

}

//                     1
//		Get in range in 121 + coeff matrix
//                     1
//
// If the value is too far away we ignore it
// else we blend

uint8_t ADMVideoDropOut::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
UNUSED_ARG(flags);

uint32_t uvlen;
uint32_t dlen,dflags;

ADMImage	*_next;
ADMImage	*_previous;
ADMImage	*_current;

	//		printf("\n DropOut : %lu\n",frame);




			uvlen=    _info.width*_info.height;
			*len=uvlen+(uvlen>>1);
			  if(frame> _info.nb_frames-1) return 0;
			if(!frame || (frame==_info.nb_frames-1))
			{
				_current=vidCache->getImage(frame);
				if(!_current) return 0;
				memcpy(YPLANE(data),YPLANE(_current),uvlen);
				memcpy(UPLANE(data),UPLANE(_current),uvlen>>2);
				memcpy(VPLANE(data),VPLANE(_current),uvlen>>2);
				vidCache->unlockAll();
				return 1;
			}
			
		_current=vidCache->getImage(frame);
		_previous=vidCache->getImage(frame-1);
		_next=vidCache->getImage(frame+1);
		if(!_current || !_previous || !_next)
		{
			vidCache->unlockAll();
			 return 0;	
		}
           	// for u & v , no action -> copy it as is
           	memcpy(UPLANE(data),UPLANE(_current),uvlen>>2);
		memcpy(VPLANE(data),VPLANE(_current),uvlen>>2);

             uint8_t *inprev,*innext,*incur,*zout;

              inprev=YPLANE(_previous)+1+_info.width;
              innext=YPLANE(_next)+1+_info.width;
              incur =YPLANE(_current)+1+_info.width;

              zout=YPLANE(data)+_info.width+1;

              int32_t c0,c1,c2,c3; //,_nextPix,_currPix,_prevPix,cc;

              for(uint32_t y= _info.height-2;y>2;y--)
              	{
		  c0=0;
		  c1=0;
		  c2=0;
		  c3=0;

	  	inprev=YPLANE(_previous)	+1+y*_info.width;
              	innext=YPLANE(_next)		+1+y*_info.width;;
              	incur =YPLANE(_current)	+1+y*_info.width;;

			// look if the field is more different temporarily than spacially

			    for(uint32_t x= _info.width-1;x>1;x--)
        		      	{

						c0+=(abs(((*inprev))-( *incur    ))^2);
						c1+=(abs(((*inprev))-( *innext   ))^2)<<1;
						c0+=(abs(((*incur ))-( *innext   ))^2);


						c2+=(abs(((    *(incur-_info.width*2) ))-( *(incur            )   ))^2)   ;
						c3+=(abs(((    *(incur-_info.width*2) ))-( *(incur+_info.width*2)   ))^2)<<1;
						c2+=(abs(((    *(incur            ) ))-( *(incur+_info.width*2)   ))^2)   ;


						incur++;
						innext++;
						inprev++;
				}

		// If yes, replace the line by an average of next/previous image
		inprev=YPLANE(_previous)	+y*_info.width;
              	innext=YPLANE(_next)		+y*_info.width;;
              	incur =YPLANE(_current)		+y*_info.width;;
		zout=YPLANE(data)		+y*_info.width;

		if (c1<c0 &&c3<c2)
		{
		    for(uint32_t x= _info.width;x>0;x--)
       			      	{
					*zout= ((*(inprev))+(*(innext)))>>1 ;
					zout++;
					innext++;
					inprev++;
				}
		}
		else
			memcpy(zout,incur,_info.width);
	}
	data->copyInfo(_current);
	vidCache->unlockAll();
return 1;
}




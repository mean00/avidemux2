/***************************************************************************
                          ADM_vidStabilize.cpp  -  description
                             -------------------
    begin                : Mon Oct 7 2002
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
#include <math.h>

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidStabilize.h"

#include "DIA_factory.h"

static FILTER_PARAM stabParam={1,{"param"}};

//REGISTERX(VF_NOISE, "stabilize",QT_TR_NOOP("Stabilize"),
    //QT_TR_NOOP("Light denoiser."),VF_STABILIZE,1,stabilize_create,stabilize_script);
VF_DEFINE_FILTER(ADMVideoStabilize,stabParam,
    Stabilize,
                QT_TR_NOOP("Light denoiser."),
                1,
                VF_NOISE,
                QT_TR_NOOP("Light denoiser."));

static uint8_t distMatrix[256][256];
static uint32_t fixMul[16];
static bool distMatrixDone=false;

static void buildDistMatrix( void )
{
int d;  
    for(uint32_t y=255;y>0;y--)
    for(uint32_t x=255;x>0;x--)
    {
          d=x-y;
          if(d<0) d=-d;
          distMatrix[x][y]=d;
        
    }

     for(int i=1;i<16;i++)
                        {
                                        fixMul[i]=(1<<16)/i;
                        }

}

char 	*ADMVideoStabilize::printConf(void)
{
  ADM_FILTER_DECLARE_CONF(" Stabilize :%ld",*_param);
        
}
uint8_t  GUI_getIntegerValue(int *valye, int min, int max, char *title);
uint8_t ADMVideoStabilize::configure(AVDMGenericVideoStream *instream)
{
int i;
	_in=instream;
        
         diaElemUInteger chroma(_param,QT_TR_NOOP("Stabilize Threshold"),1,255);    
         diaElem *elems[]={&chroma};
  
    return diaFactoryRun(QT_TR_NOOP("Stabilize"),sizeof(elems)/sizeof(diaElem *),elems);
	
}
//--------------------------------------------------------	
ADMVideoStabilize::ADMVideoStabilize(AVDMGenericVideoStream *in,CONFcouple *couples)
{
if(distMatrixDone==false)
        {
            buildDistMatrix();
            distMatrixDone=true;
        }
  //uint32_t frame;
  _uncompressed=NULL;
  _in=in;
  memcpy(&_info,in->getInfo(),sizeof(_info)); 
 
  _info.encoding=1;
  if(couples)
  {
			_param=NEW(uint32_t);
			ADM_assert(couples->getCouple((char *)"param",_param));
	}
	else
	{
			_param=NEW( uint32_t);
			*_param=30;
	}
	vidCache=new VideoCache(5,_in);
 
}
ADMVideoStabilize::~ADMVideoStabilize()
{
 
 	DELETE(_param);
	if(vidCache) delete vidCache;
	vidCache=NULL;
}


uint8_t	ADMVideoStabilize::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(1);
 			(*couples)->setCouple((char *)"param",*_param);

			return 1;

}
//                     1
//		Get in range in 121 + coeff matrix
//                     1
//
// If the value is too far away we ignore it
// else we blend

uint8_t ADMVideoStabilize::getFrameNumberNoAlloc(uint32_t frame,
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
		
		uvlen=    _info.width*_info.height;
		*len=uvlen+(uvlen>>1);
		if(frame> _info.nb_frames-1) return 0;
		_current=vidCache->getImage(frame);
		if(!_current) return 0;
		data->copyInfo(_current);
		if(!frame || (frame==_info.nb_frames-1))
		{

			data->duplicate(_current);
			vidCache->unlockAll();
			return 1;
		}	 
   		_previous=vidCache->getImage(frame-1);		
		if(!_previous)
		{
			vidCache->unlockAll();
			return 0;
		}
   		_next=vidCache->getImage(frame+1);
		if(!_next)
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
              
             
              uint8_t *nl,*pl,*nc,*pc;
              
              uint16_t c,coeff;
              uint32_t x;
              for(uint32_t y= _info.height-1;y>1;y--)
              	{
			nl=incur+_info.width;
			pl=incur-_info.width;
			nc=incur+1;
			pc=incur-1;		   
								   
		        for(x= _info.width-1;x>1;x--)
        		{
	               		c=*incur*4;;
	                    	coeff=4;
#define PONDERATE(x,p) if(distMatrix[*incur][x]<*_param)	                      \
{ c+=x;coeff++;}
																PONDERATE(*innext,1);
				PONDERATE(*inprev,1);
				PONDERATE(*(pc),1);
				PONDERATE(*(nc),1);
				PONDERATE(*(nl),1);
				PONDERATE(*(pl),1);
													//*zout=(uint8_t)floor(0.49+(c/coeff));
																ADM_assert(coeff);
				ADM_assert(coeff<16);
				*zout=(c*fixMul[coeff])>>16;
				zout++;
				incur++;
				innext++;
				inprev++;	
				nl++;pl++;nc++;pc++;	  
			}
			zout+=2;
			incur+=2;
			innext+=2;
			inprev+=2;
		}
		vidCache->unlockAll();
		return 1;	
}



/***************************************************************************
                          ADM_vidFastConvolution.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
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
#include "DIA_factory.h"
#include "ADM_vidConvolution.hxx"




uint8_t	AVDMFastVideoConvolution::getCoupledConf( CONFcouple **couples)
{

	ADM_assert(_param);
	
	*couples=new CONFcouple(2);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
	CSET(chroma);
	CSET(luma);
	return 1;

}

//_______________________________________________________________

AVDMFastVideoConvolution::AVDMFastVideoConvolution(
			AVDMGenericVideoStream *in,CONFcouple *couples)
{


	_in=in;		
	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_uncompressed=new ADMImage(_in->getInfo()->width,_in->getInfo()->height);	
	ADM_assert(_uncompressed); 
	_info.encoding=1;
	if(couples==NULL)
	{
		_param=NEW( CONV_PARAM);
		_param->chroma=1;
		_param->luma=1;	
		printf("\n Creating from null\n");				
	}
	else
	{
		_param=NEW(CONV_PARAM);
		GET(luma);
		GET(chroma);
	}

  	  	
}
AVDMFastVideoConvolution::~AVDMFastVideoConvolution()
{
	if(_uncompressed)
 		delete _uncompressed;
	_uncompressed=NULL;
 	DELETE(_param);
}

//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMFastVideoConvolution::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//uint8_t *dst,*dstu,*dstv,*srcu,*srcv;
uint8_t *x1,*x2,*x3,*o1;
uint32_t stride,page;

	if(frame>= _info.nb_frames) return 0;
	ADM_assert(_uncompressed);					
	stride=_info.width;
	page=(stride*_info.height)>>2;
	

	
	// read uncompressed frame
	if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;               
         
	if(!_param->luma)
	{
		memcpy(YPLANE(data),YPLANE(_uncompressed),page*4);
	}
	else
	{
		o1=YPLANE(data)+stride;
		x1=YPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;

		// first and last line
		memcpy(YPLANE(data),YPLANE(_uncompressed),stride);
		memcpy(YPLANE(data)+page*4-stride,YPLANE(_uncompressed)+page*4-stride,stride);          
		// Luma
		for(int32_t y=1;y<(int32_t)_info.height-1;y++)
		{
			doLine(x1,x2,x3,o1,stride);
			x1=x2;
			x2=x3;
			x3+=stride; 
			o1+=stride;                 
		}
	}
      	// chroma u & v
	if(!_param->chroma)
	{
		memcpy(UPLANE(data),UPLANE(_uncompressed),page);
		memcpy(VPLANE(data),VPLANE(_uncompressed),page);
	}
	else
	{
		stride>>=1;
		// chroma u
		o1=UPLANE(data)+stride;
		x1=UPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;
		// first and last line
		memcpy(UPLANE(data),UPLANE(_uncompressed),stride);
		memcpy(UPLANE(data)+page-stride,UPLANE(_uncompressed)+page-stride,stride);          
		// Luma
		for(int32_t y=1;y<(int32_t)(_info.height>>1)-1;y++)
		{
			doLine(x1,x2,x3,o1,stride);
			x1=x2;
			x2=x3;
			x3+=stride; 
			o1+=stride;                 
		}
		
		// chroma V
		o1=VPLANE(data)+stride;
		x1=VPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;
		// first and last line
		memcpy(VPLANE(data),VPLANE(_uncompressed),stride);
		memcpy(VPLANE(data)+page-stride,VPLANE(_uncompressed)+page-stride,stride);          
		// Luma
		for(int32_t y=1;y<(int32_t)(_info.height>>1)-1;y++)
		{
			doLine(x1,x2,x3,o1,stride);
			x1=x2;
			x2=x3;
			x3+=stride; 
			o1+=stride;                 
		}
	}
	data->copyInfo(_uncompressed);
      return 1;
}
uint8_t AVDMFastVideoConvolution::configure(AVDMGenericVideoStream * instream)
{
  
  //return DIA_getLumaChroma(&(_param->luma),&(_param->chroma)) ; 
  diaElemToggle luma(&(_param->luma),QT_TR_NOOP("_Process luma"),QT_TR_NOOP("Process luma plane"));
  diaElemToggle chroma(&(_param->chroma),QT_TR_NOOP("P_rocess chroma"));
  
  diaElem *elems[2]={&luma,&chroma};
  
  return diaFactoryRun(QT_TR_NOOP("Fast Convolution"),2,elems);
}




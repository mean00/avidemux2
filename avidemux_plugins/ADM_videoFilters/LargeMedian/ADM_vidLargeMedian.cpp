/***************************************************************************
                          ADM_vidLargeMedian.cpp  -  description
                             -------------------
    begin                : Wed Jan 1 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
    
    Using http://ndevilla.free.fr/median/median/node20.html
    optimized median search
    
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
#include "ADM_vidLargeMedian.h"
#include "DIA_factory.h"

static FILTER_PARAM nullParam={2,{"chroma","luma"}};


//********** Register chunk ************
VF_DEFINE_FILTER(ADMVideoLargeMedian,nullParam,
                largemedian,
                QT_TR_NOOP("Median (5x5)"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Median kernel 5x5. Good for reducing chroma noise."));
//****************************************
//_______________________________________________________________

ADMVideoLargeMedian::ADMVideoLargeMedian(
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

uint8_t	ADMVideoLargeMedian::getCoupledConf( CONFcouple **couples)
{

			ADM_assert(_param);
			*couples=new CONFcouple(2);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
			CSET(luma);
			CSET(chroma);
			return 1;

}
ADMVideoLargeMedian::~ADMVideoLargeMedian()
{
	if(_uncompressed)
 		delete _uncompressed;
	_uncompressed=NULL;
	DELETE(_param);
}
char *ADMVideoLargeMedian::printConf(void)
{
 		return (char *)"Median (5x5)";; // this one is pure
}
//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t ADMVideoLargeMedian::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
//uint8_t *dst,*dstu,*dstv,*srcu,*srcv;
uint8_t *x1,*x2,*x3,*x4,*x5,*o1;
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
		memcpy(YPLANE(data),YPLANE(_uncompressed),stride*2);
		memcpy(YPLANE(data)+page*4-stride*2,YPLANE(_uncompressed)+page*4-2*stride,2*stride);          
	         
		o1=YPLANE(data)+stride*2;;
		x1=YPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;
		x4=x3+stride;
		x5=x4+stride;
		// Luma
		for(int32_t y=2;y<(int32_t)(_info.height)-2;y++)
		{
			doLine(x1,x2,x3,x4,x5,o1,stride);
			x1=x2;
			x2=x3;
			x3=x4;
			x4=x5;
			x5+=stride; 
			o1+=stride;                 
		}
	}
					
      	
	stride>>=1;
	if(!_param->chroma)
	{
	 	memcpy(UPLANE(data),UPLANE(_uncompressed),page*2);			
	}	
	else
	{
		// first and last line
		memcpy(UPLANE(data),UPLANE(_uncompressed),stride*2);
		memcpy(UPLANE(data)+page-stride*2,UPLANE(_uncompressed)+page-2*stride,2*stride);
		// chroma u	
		o1=UPLANE(data)+stride*2;
		x1=UPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;
		x4=x3+stride;
		x5=x4+stride;
	          
		for(int32_t y=2;y<(int32_t)(_info.height>>1)-2;y++)
		{
			doLine(x1,x2,x3,x4,x5,o1,stride);
			x1=x2;
			x2=x3;
			x3=x4;
			x4=x5;
			x5+=stride; 
			o1+=stride;                 
		}
		// chroma V
		// first and last line
		memcpy(VPLANE(data),VPLANE(_uncompressed),stride*2);
		memcpy(VPLANE(data)+page-2*stride,VPLANE(_uncompressed)+page-2*stride,2*stride);          
	          
		o1=VPLANE(data)+stride*2;
		x1=VPLANE(_uncompressed);
		x2=x1+stride;
		x3=x2+stride;
		x4=x3+stride;
		x5=x4+stride;

	         
		for(int32_t y=2;y<(int32_t)(_info.height>>1)-2;y++)
		{
			doLine(x1,x2,x3,x4,x5,o1,stride);
			x1=x2;
			x2=x3;
			x3=x4;
			x4=x5;
			x5+=stride; 
			o1+=stride;                      
	      	}
	}
	data->copyInfo(_uncompressed);
	return 1;
}
//________________________________________________________________________
uint8_t ADMVideoLargeMedian::doLine(uint8_t  *pred2,uint8_t  *pred1,
					uint8_t *cur,
   					uint8_t *next1,uint8_t *next2,
   					uint8_t *out,
                       			uint32_t w)
                                 
{
static uint8_t box[5][5];	
static uint8_t box2[5][5];	

uint32_t col;
uint8_t temp;
uint32_t inbox;
	
// prefill box
	for(uint32_t x=0;x<4;x++)
		{
			box[0][x+1]=*(pred2+x);
			box[1][x+1]=*(pred1+x);
			box[2][x+1]=*(cur+x);
			box[3][x+1]=*(next1+x);
			box[4][x+1]=*(next2+x);			
		}
		col=0;
		*out=*cur;
		*(out+1)=*(cur+1);
		*(out+w-1)=*(cur+w-1);		
		*(out+w-2)=*(cur+w-2);
		out+=2;
		next1+=4;
		next2+=4;
		pred1+=4;
		pred2+=4;
		cur+=4;	
	while(w>4)
	{
		// fill
			box[0][col]=*pred2++;
			box[1][col]=*pred1++;
			box[2][col]=*cur++;
			box[3][col]=*next1++;
			box[4][col]=*next2++;
			col++;
			col%=5;
			// copy & sort
			memcpy(box2,box,5*5);
			uint8_t *p=(uint8_t *)box2;	
			inbox=0;	
#define PIX_SORT(a,b) { if ((a)>(b)) PIX_SWAP((a),(b)); }
#define PIX_SWAP(a,b) { temp=(a);(a)=(b);(b)=temp; }

			
    PIX_SORT(p[0], p[1]) ;   PIX_SORT(p[3], p[4]) ;   PIX_SORT(p[2], p[4]) ;
    PIX_SORT(p[2], p[3]) ;   PIX_SORT(p[6], p[7]) ;   PIX_SORT(p[5], p[7]) ;
    PIX_SORT(p[5], p[6]) ;   PIX_SORT(p[9], p[10]) ;  PIX_SORT(p[8], p[10]) ;
    PIX_SORT(p[8], p[9]) ;   PIX_SORT(p[12], p[13]) ; PIX_SORT(p[11], p[13]) ;
    PIX_SORT(p[11], p[12]) ; PIX_SORT(p[15], p[16]) ; PIX_SORT(p[14], p[16]) ;
    PIX_SORT(p[14], p[15]) ; PIX_SORT(p[18], p[19]) ; PIX_SORT(p[17], p[19]) ;
    PIX_SORT(p[17], p[18]) ; PIX_SORT(p[21], p[22]) ; PIX_SORT(p[20], p[22]) ;
    PIX_SORT(p[20], p[21]) ; PIX_SORT(p[23], p[24]) ; PIX_SORT(p[2], p[5]) ;
    PIX_SORT(p[3], p[6]) ;   PIX_SORT(p[0], p[6]) ;   PIX_SORT(p[0], p[3]) ;
    PIX_SORT(p[4], p[7]) ;   PIX_SORT(p[1], p[7]) ;   PIX_SORT(p[1], p[4]) ;
    PIX_SORT(p[11], p[14]) ; PIX_SORT(p[8], p[14]) ;  PIX_SORT(p[8], p[11]) ;
    PIX_SORT(p[12], p[15]) ; PIX_SORT(p[9], p[15]) ;  PIX_SORT(p[9], p[12]) ;
    PIX_SORT(p[13], p[16]) ; PIX_SORT(p[10], p[16]) ; PIX_SORT(p[10], p[13]) ;
    PIX_SORT(p[20], p[23]) ; PIX_SORT(p[17], p[23]) ; PIX_SORT(p[17], p[20]) ;
    PIX_SORT(p[21], p[24]) ; PIX_SORT(p[18], p[24]) ; PIX_SORT(p[18], p[21]) ;
    PIX_SORT(p[19], p[22]) ; PIX_SORT(p[8], p[17]) ;  PIX_SORT(p[9], p[18]) ;
    PIX_SORT(p[0], p[18]) ;  PIX_SORT(p[0], p[9]) ;   PIX_SORT(p[10], p[19]) ;
    PIX_SORT(p[1], p[19]) ;  PIX_SORT(p[1], p[10]) ;  PIX_SORT(p[11], p[20]) ;
    PIX_SORT(p[2], p[20]) ;  PIX_SORT(p[2], p[11]) ;  PIX_SORT(p[12], p[21]) ;
    PIX_SORT(p[3], p[21]) ;  PIX_SORT(p[3], p[12]) ;  PIX_SORT(p[13], p[22]) ;
    PIX_SORT(p[4], p[22]) ;  PIX_SORT(p[4], p[13]) ;  PIX_SORT(p[14], p[23]) ;
    PIX_SORT(p[5], p[23]) ;  PIX_SORT(p[5], p[14]) ;  PIX_SORT(p[15], p[24]) ;
    PIX_SORT(p[6], p[24]) ;  PIX_SORT(p[6], p[15]) ;  PIX_SORT(p[7], p[16]) ;
    PIX_SORT(p[7], p[19]) ;  PIX_SORT(p[13], p[21]) ; PIX_SORT(p[15], p[23]) ;
    PIX_SORT(p[7], p[13]) ;  PIX_SORT(p[7], p[15]) ;  PIX_SORT(p[1], p[9]) ;
    PIX_SORT(p[3], p[11]) ;  PIX_SORT(p[5], p[17]) ;  PIX_SORT(p[11], p[17]) ;
    PIX_SORT(p[9], p[17]) ;  PIX_SORT(p[4], p[10]) ;  PIX_SORT(p[6], p[12]) ;
    PIX_SORT(p[7], p[14]) ;  PIX_SORT(p[4], p[6]) ;   PIX_SORT(p[4], p[7]) ;
    PIX_SORT(p[12], p[14]) ; PIX_SORT(p[10], p[14]) ; PIX_SORT(p[6], p[7]) ;
    PIX_SORT(p[10], p[12]) ; PIX_SORT(p[6], p[10]) ;  PIX_SORT(p[6], p[17]) ;
    PIX_SORT(p[12], p[17]) ; PIX_SORT(p[7], p[17]) ;  PIX_SORT(p[7], p[10]) ;
    PIX_SORT(p[12], p[18]) ; PIX_SORT(p[7], p[12]) ;  PIX_SORT(p[10], p[18]) ;
    PIX_SORT(p[12], p[20]) ; PIX_SORT(p[10], p[20]) ; PIX_SORT(p[10], p[12]) ;
			
		  
		  *out++=p[12];
		  w--;
	}	
	
	return 1;
}

uint8_t ADMVideoLargeMedian::configure(AVDMGenericVideoStream * instream)
{
  diaElemToggle luma(&(_param->luma),QT_TR_NOOP("_Process luma"),QT_TR_NOOP("Process luma plane"));
  diaElemToggle chroma(&(_param->chroma),QT_TR_NOOP("P_rocess chroma"));
  
  diaElem *elems[2]={&luma,&chroma};
  
  return diaFactoryRun(QT_TR_NOOP("Large Median 5x5"),2,elems);
  
}







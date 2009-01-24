
/***************************************************************************
                          ADM_vidWhirl.cpp  -  description
                             -------------------
    begin                : Fri Jan 3 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
    
    
   Whrilpool like effect
    
    
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
#include <math.h>
//#define LOOP


static void Process(uint8_t *src,uint8_t *data, uint32_t width, uint32_t height,double frac);


static FILTER_PARAM whirlParam={0,{"ythresholdMask","cthresholdMask"}};

#define STEP_SIZE 150
static int COS_CALCED[3600];
static int SIN_CALCED[3600];
class  AVDMVideoWhirl:public AVDMGenericVideoStream
 {

 protected:

        			
        			virtual 	char *printConf(void);
						
 public:


						AVDMVideoWhirl(  AVDMGenericVideoStream *in,CONFcouple *setup);
  					 	AVDMVideoWhirl();
		        virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          							ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
			
 }     ;
 //REGISTERX(VF_MISC, "whirl",QT_TR_NOOP("Whirl"),QT_TR_NOOP(
 //    "Useless whirlwind effect."),VF_WHIRL,1,whirl_create,whirl_script); 
VF_DEFINE_FILTER(AVDMVideoWhirl,whirlParam,
    whirl,
                QT_TR_NOOP("Whirl"),
                1,
                VF_MISC,
                QT_TR_NOOP("Useless whirlwind effect."));

char *AVDMVideoWhirl::printConf(void)
{
	ADM_FILTER_DECLARE_CONF(" Whirl");
        
}


uint8_t AVDMVideoWhirl::configure( AVDMGenericVideoStream *instream)
{
		
		return 1;
}     											
AVDMVideoWhirl::AVDMVideoWhirl(  AVDMGenericVideoStream *in,CONFcouple *couples)
		

{
	_in=in;
	memcpy(&_info,_in->getInfo(),sizeof(_info));
	_uncompressed= new ADMImage(_info.width,_info.height);
	
	#define TWOPI (2*3.1415)
	double angle;
	for(int i=0;i<3600;i++)
	{
		angle=(double)i;
		angle/=TWOPI;
		angle/=10.;
		COS_CALCED[i]=(int)floor(0.49+65536.*cos(angle));
		SIN_CALCED[i]=(int)floor(0.49+65536.*sin(angle));
		
	}
}


AVDMVideoWhirl::AVDMVideoWhirl()
{
	delete _uncompressed;
}  


uint8_t AVDMVideoWhirl::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
	
	uint32_t page=_info.width*_info.height;
	ADMImage *cur,*prev;
	double frac;
	        if(frame>= _info.nb_frames) return 0;
		if(frame>STEP_SIZE)
			frac=1;
		else
			frac=(double)frame/(double)STEP_SIZE;
		if(frame>(_info.nb_frames-1)) return 0;
		
		*len=(page*3)>>1;
		if(!_in->getFrameNumberNoAlloc(frame,len,_uncompressed,flags)) return 0;
		
		data->copyInfo(_uncompressed);
		Process(YPLANE(_uncompressed),YPLANE(data),_info.width,_info.height,frac);
		Process(UPLANE(_uncompressed),UPLANE(data),_info.width>>1,_info.height>>1,frac);
		Process(VPLANE(_uncompressed),VPLANE(data),_info.width>>1,_info.height>>1,frac);
		
		return 1;

}
#define invlen(x) x
void Process(uint8_t *src,uint8_t *data, uint32_t width, uint32_t height,double frac)
{
int	dprime,d;
int 	pointx,pointy;
int	fx,fy;
int	alpha,beta;

uint8_t *wr;
double angle;
int 	iang;

	wr=data;
	frac=10*frac*180./width;
	for(uint32_t y=0;y<height;y++)
	{
	for(uint32_t x=0;x<width;x++)
	{
		pointx=x-(width>>1);
		pointy=y-(height>>1);
		if(!pointx && !pointy)
		{
			// center continue
			*wr++=*(src+((width*height+width)>>1));
			continue;
		}
		
		dprime=pointx*pointx+pointy*pointy;
		dprime=(int)floor(0.49+sqrt(dprime));

//		dprime=abs(pointx)+abs(pointy);		
		d=invlen(dprime);
		// rotate 
		
		iang=(int)floor((double)d*frac);
		iang%=3600;
		
		alpha=COS_CALCED[iang];
		beta=SIN_CALCED[iang];
		
		fx=alpha*pointx+beta*pointy;
		fy=-beta*pointx+alpha*pointy;
		// expand
		fx=fx>>16;
		fy=fy>>16;
		// put back to original
		pointx=fx+(width>>1);
		pointy=fy+(height>>1);
		
		// Clamp
		if(pointx<0) pointx=0;
		if(pointx>width-1) pointx=width-1;
		if(pointy<0) pointy=0;
		if(pointy>height-1) pointy=height-1;
		
		*(wr++)=*(src+pointx+width*pointy);
		
	
	}
	}
}

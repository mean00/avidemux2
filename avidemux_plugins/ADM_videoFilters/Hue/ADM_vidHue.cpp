/***************************************************************************
                          Hue/Saturation filter ported from mplayer 
 (c) Michael Niedermayer
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
#include "ADM_vidHue.h"
#include "math.h"
static FILTER_PARAM HueParam={2,{"hue","saturation"}};

extern uint8_t DIA_getHue(Hue_Param *param, AVDMGenericVideoStream *in);

class  ADMVideoHue:public AVDMGenericVideoStream
{

  protected:
    AVDMGenericVideoStream  *_in;           
    virtual char            *printConf(void);
            void            update(void);
            Hue_Param       *_param;    
            VideoCache      *vidCache; 
            float           _hue;
            float           _saturation;
  public:
                
                        ADMVideoHue(  AVDMGenericVideoStream *in,CONFcouple *setup);
    virtual             ~ADMVideoHue();
    virtual uint8_t     configure(AVDMGenericVideoStream *in);
    virtual uint8_t     getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);

             uint8_t     getCoupledConf( CONFcouple **couples);
}     ;


VF_DEFINE_FILTER_UI(ADMVideoHue,
          HueParam,
          hue,
          QT_TR_NOOP("MPlayer hue"),
          1,
          VF_COLORS,
          QT_TR_NOOP("Adjust hue and saturation."));

void HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc, int dststride, int srcstride,
		    int w, int h, float hue, float sat)
{
	int i;
	const int s= (int)rint(sin(hue) * (1<<16) * sat);
	const int c= (int)rint(cos(hue) * (1<<16) * sat);

	while (h--) {
		for (i = 0; i<w; i++)
		{
			const int u= usrc[i] - 128;
			const int v= vsrc[i] - 128;
			int new_u= (c*u - s*v + (1<<15) + (128<<16))>>16;
			int new_v= (s*u + c*v + (1<<15) + (128<<16))>>16;
			if(new_u & 768) new_u= (-new_u)>>31;
			if(new_v & 768) new_v= (-new_v)>>31;
			udst[i]= new_u;
			vdst[i]= new_v;
		}
		usrc += srcstride;
		vsrc += srcstride;
		udst += dststride;
		vdst += dststride;
	}
}


uint8_t ADMVideoHue::configure(AVDMGenericVideoStream *in)
{
uint8_t r=0;
  _in=in;   
  r=  DIA_getHue(_param, in);
  if(r) update();
  return r;  
}
char *ADMVideoHue::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Hue :%2.2f %2.2f",_param->hue,_param->saturation);
  
}

ADMVideoHue::ADMVideoHue(  AVDMGenericVideoStream *in,CONFcouple *couples)
{
  
  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1; 
  _param=new  Hue_Param;
  if(couples)
  {                 
    GET(hue);    
    GET(saturation); 
  }
  else
  {
    _param->hue =0.0;                
    _param->saturation=1.0;
  }      
  vidCache=new VideoCache(1,_in);
  update();
}
void ADMVideoHue::update(void)
{
    _hue=_param->hue*M_PI/180.;
    _saturation=(100+_param->saturation)/100;
}
ADMVideoHue::~ADMVideoHue()
{
  delete _param;
  delete vidCache;
  
}
uint8_t ADMVideoHue::getCoupledConf( CONFcouple **couples)
{
  ADM_assert(_param);
  *couples=new CONFcouple(2);


                CSET(hue);
                CSET(saturation);
                return 1;
}


uint8_t ADMVideoHue::getFrameNumberNoAlloc(uint32_t frame,
                                             uint32_t *len,
                                             ADMImage *data,
                                             uint32_t *flags)
{
  ADMImage *mysrc=NULL;
  

  if(frame>=_info.nb_frames) return 0;
  
  mysrc=vidCache->getImage(frame);
  if(!mysrc) return 0;
  memcpy(YPLANE(data),YPLANE(mysrc),_info.width*_info.height);
  HueProcess_C(VPLANE(data), UPLANE(data),
        VPLANE(mysrc), UPLANE(mysrc),
        _info.width>>1,_info.width>>1,
        _info.width>>1,_info.height>>1, 
        _hue, _saturation);
 
  vidCache->unlockAll();
  
  
  return 1;
}

  


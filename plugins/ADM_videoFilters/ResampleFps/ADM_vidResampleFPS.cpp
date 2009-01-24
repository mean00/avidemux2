/***************************************************************************
                          Resample fps
                             -------------------
    begin                : Wed Nov 6 2002
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
#include "DIA_enter.h"
#include "DIA_factory.h"

static FILTER_PARAM ResampParam={2,{"newfps","use_linear"}};
typedef struct FPS_Param
{
  uint32_t  newfps; 
  uint32_t  use_linear;
}FPS_Param;
class  ADMVideoResampleFPS:public AVDMGenericVideoStream
{

  protected:
    AVDMGenericVideoStream  *_in;           
    virtual char            *printConf(void);
            FPS_Param       *_param;    
            VideoCache      *vidCache; 
  public:
                
                        ADMVideoResampleFPS(  AVDMGenericVideoStream *in,CONFcouple *setup);
                        ADMVideoResampleFPS(  AVDMGenericVideoStream *in,uint32_t target1000);
    virtual             ~ADMVideoResampleFPS();
    virtual uint8_t     configure(AVDMGenericVideoStream *in);
    virtual uint8_t     getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);

             uint8_t     getCoupledConf( CONFcouple **couples);
}     ;
//***********************************
VF_DEFINE_FILTER(ADMVideoResampleFPS,ResampParam,
                resamplefps,
                QT_TR_NOOP("Resample fps"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Change framerate while keeping duration."));
//***********************************

AVDMGenericVideoStream *createResampleFps(AVDMGenericVideoStream *in,uint32_t targetfps1000)
{
  return new ADMVideoResampleFPS(in,targetfps1000);
}

uint8_t ADMVideoResampleFPS::configure(AVDMGenericVideoStream *in)
{
  float f=_param->newfps; 
  f/=1000;
  
  _in=in;
  
    diaElemFloat fps(&f,QT_TR_NOOP("_New frame rate:"),1,200.);
    diaElemToggle blend(&(_param->use_linear),QT_TR_NOOP("_Blend"));
    
    diaElem *elems[2]={&fps,&blend};
  
    if( diaFactoryRun(QT_TR_NOOP("Resample fps"),2,elems))
    {
        f*=1000;
      _param->newfps=(uint32_t)floor(f+0.4); 
      _info.fps1000=_param->newfps;
      return 1;
    }
    return 0;
}
char *ADMVideoResampleFPS::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Resample to %2.2f fps (blend:%d)",(double)_param->newfps/1000.,
                _param->use_linear);
  
}

ADMVideoResampleFPS::ADMVideoResampleFPS(  AVDMGenericVideoStream *in,uint32_t target)
{

  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1; 
  _param=new  FPS_Param;
   
    _param->newfps =target;                
    _param->use_linear=1;
 
  double newlength;
  
  newlength=_info.nb_frames;
  newlength/=_info.fps1000;
  newlength*=_param->newfps;
  _info.nb_frames=(uint32_t)floor(newlength);
  _info.fps1000=_param->newfps;
  printf("[Resample FPS] %u -> %u\n",_in->getInfo()->nb_frames,_info.nb_frames);
  vidCache=new VideoCache(3,_in);

}

ADMVideoResampleFPS::ADMVideoResampleFPS(  AVDMGenericVideoStream *in,CONFcouple *couples)
{
  
  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1; 
  _param=new  FPS_Param;
   
  if(couples)
  {                 
    GET(newfps);    
    GET(use_linear); 
  }
  else
  {
    _param->newfps =_info.fps1000;                
    _param->use_linear=0;
  }      
 
  double newlength;
  
  newlength=_info.nb_frames;
  newlength/=_info.fps1000;
  newlength*=_param->newfps;
  _info.nb_frames=(uint32_t)floor(newlength);
  _info.fps1000=_param->newfps;
  vidCache=new VideoCache(3,_in);
  
}
ADMVideoResampleFPS::~ADMVideoResampleFPS()
{
  delete _param;
  delete vidCache;
  
}
uint8_t ADMVideoResampleFPS::getCoupledConf( CONFcouple **couples)
{
  ADM_assert(_param);
  *couples=new CONFcouple(2);


                CSET(newfps);
                CSET(use_linear);
                return 1;
}
#ifdef ADM_CPU_X86
static uint64_t FUNNY_MANGLE(low) , FUNNY_MANGLE(high);
static void blendMMX(uint8_t *src, uint8_t *src2, uint8_t *dst, uint8_t alpha, uint8_t beta,uint32_t count)
{
uint32_t left=count&3;
#define EXPAND(x) (x)+((x)<<16)+((x)<<32) +((x)<<48)
        high=alpha;
        low=beta;
        high=EXPAND(high);
        low=EXPAND(low);
        count>>=2;
#ifdef GCC_2_95_X
         __asm__ __volatile__ (
                                "movq "Mangle(high)", %mm0\n"
                                "movq "Mangle(low)",  %mm1\n"                                
                                "pxor %mm7        ,  %mm7\n"
                                :: );
#else
         __asm__ __volatile__ (
                                "movq "Mangle(high)", %%mm0\n"
                                "movq "Mangle(low)",  %%mm1\n"                                
                                "pxor %%mm7        ,  %%mm7\n"
                                :: );
#endif

        while(count>0)
        {
                __asm__ __volatile__ (
                               
                                "movd      (%0),  %%mm2\n"
                                "movd      (%1),  %%mm3\n"                               

                                "punpcklbw %%mm7, %%mm2\n"
                                "punpcklbw %%mm7, %%mm3\n"

                                "pmullw   %%mm0, %%mm2\n"
                                "pmullw   %%mm1, %%mm3\n"
                
                                "paddw    %%mm3, %%mm2\n"

                                "psrlw    $8,    %%mm2 \n"

                                "packuswb %%mm2,%%mm2\n"
                                "movd     %%mm2, (%2)\n"

                                :: "r" (src), "r" (src2), "r" (dst) );
                                
                src+=4;
                src2+=4;
                dst+=4;
                count--;
        }
        __asm__ __volatile__ (
                                "emms\n"
                                :: );
        for(uint32_t i=0;i<left;i++)
        {
                dst[i] = ((src[i]*alpha) + (src2[i]*beta))>>8;
        }
}

#endif

uint8_t ADMVideoResampleFPS::getFrameNumberNoAlloc(uint32_t frame,
                                             uint32_t *len,
                                             ADMImage *data,
                                             uint32_t *flags)
{
  ADMImage *mysrc1=NULL;
  ADMImage *mysrc2=NULL;

  if(frame>=_info.nb_frames) return 0;
  // read uncompressed frame
  
  // What frame are we seeking ?
  double f;
  uint32_t page=_info.width*_info.height;
  
  f=frame;
  f*=_in->getInfo()->fps1000;
  f/=_param->newfps;
  
  if(!_param->use_linear)
  {
      uint32_t nw;
      
      nw=(uint32_t)floor(f+0.4);
      if(nw>_in->getInfo()->nb_frames-1)
        nw=_in->getInfo()->nb_frames-1;
    
      mysrc1=vidCache->getImage(nw);
      if(!mysrc1) return 0;
      
      memcpy(YPLANE(data),YPLANE(mysrc1),page);
      memcpy(UPLANE(data),UPLANE(mysrc1),page>>2);
      memcpy(VPLANE(data),VPLANE(mysrc1),page>>2);
    
      vidCache->unlockAll();
      
      return 1;
  }
  /* With linear blending */
  uint32_t nw;
  uint8_t lowweight;
  uint8_t highweight;
  
  double diff;
  
  nw=(uint32_t)floor(f);
  diff=f-floor(f);
  highweight = (uint8_t)floor(diff*256);
  lowweight = 256 - highweight;

  if(nw>=_in->getInfo()->nb_frames-1)
    {
      printf("[ResampleFps] In %u Out %u\n",frame,nw);
      nw=_in->getInfo()->nb_frames-1;
      highweight=0;
    }
  //printf("New:%lu old:%lu\n",frame,nw);

  if(highweight == 0)
    {
      mysrc1=vidCache->getImage(nw);  
      if(!mysrc1) return 0;
      
      memcpy(YPLANE(data),YPLANE(mysrc1),page);
      memcpy(UPLANE(data),UPLANE(mysrc1),page>>2);
      memcpy(VPLANE(data),VPLANE(mysrc1),page>>2);
      
      vidCache->unlockAll();
    }
  else
    {
      mysrc1=vidCache->getImage(nw);
      mysrc2=vidCache->getImage(nw+1);
      if(!mysrc1 || !mysrc2) return 0;
      
      uint8_t *out, *in1, *in2;
      uint32_t count;
      uint32_t idx;
      
      out = YPLANE(data);
      in1 = YPLANE(mysrc1);
      in2 = YPLANE(mysrc2);
        
      count = page;

#ifdef ADM_CPU_X86
        if(CpuCaps::hasMMX())
                blendMMX(in1,in2,out,lowweight,highweight,(count*3)>>1);
        else
#endif
      {
      for(idx = 0; idx < count; ++idx)
	out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;

      out = UPLANE(data);
      in1 = UPLANE(mysrc1);
      in2 = UPLANE(mysrc2);
      count = page>>2;

      for(idx = 0; idx < count; ++idx)
        out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;      


      out = VPLANE(data);
      in1 = VPLANE(mysrc1);
      in2 = VPLANE(mysrc2);
      count = page>>2;

      for(idx = 0; idx < count; ++idx)
	out[idx] = ((in1[idx]*lowweight) + (in2[idx]*highweight))>>8;
      }

      vidCache->unlockAll();
    }
  return 1;
 
}





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
#include "DIA_coreToolkit.h"
#include "ADM_vidFade_param.h"
class AVDM_Fade : public AVDMGenericVideoStream
{
  VideoCache      *vidCache;
  VIDFADE_PARAM   *_param;
  uint16_t         lookupLuma[256][256];
  uint16_t         lookupChroma[256][256];
  uint8_t         buildLut(void);
  public:
                                
                    AVDM_Fade(AVDMGenericVideoStream *in,CONFcouple *couples);    
                    ~AVDM_Fade(void);
    uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);
        
    char            *printConf( void );
    uint8_t         configure(AVDMGenericVideoStream *in);
    uint8_t         getCoupledConf( CONFcouple **couples);
};

static FILTER_PARAM fadeParam={4,{"startFade","endFade","inOut","toBlack"}};

VF_DEFINE_FILTER(AVDM_Fade,fadeParam,
                fade,
                QT_TR_NOOP("Fade"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Fade in/out."));
/*************************************/
uint8_t AVDM_Fade::configure(AVDMGenericVideoStream *in)
{
  uint32_t mx=_info.nb_frames;
  _in=in;
  
  diaMenuEntry menuE[2]={{0,QT_TR_NOOP("Out"),QT_TR_NOOP("Fade out")},{1,QT_TR_NOOP("In"),QT_TR_NOOP("Fade in")}};
  uint32_t start,end;
  VIDFADE_PARAM param=*_param;
  
while(1)
{
    diaElemMenu     menu(&(param.inOut),QT_TR_NOOP("_Fade type:"), 2,menuE);
    diaElemUInteger start(&(param.startFade),QT_TR_NOOP("_Start frame:"),0,mx);
    diaElemUInteger end(&(param.endFade),QT_TR_NOOP("_End frame:"),0,mx);
    diaElemToggle   black(&(param.toBlack),QT_TR_NOOP("Fade to _black"));
    
    diaElem *elems[4]={&menu,&start,&end,&black};
  
    if( diaFactoryRun(QT_TR_NOOP("Fade"),4,elems))
    {
      // Check it is consistent
      if(param.startFade>=param.endFade || (param.startFade>=mx) || (param.endFade>=mx))
      {
        GUI_Error_HIG(QT_TR_NOOP("Parameter Error"),QT_TR_NOOP("Start must be before end, and both within video # of frames."));
        continue; 
      }
      //
      *_param=param;
      return 1;
    }else
        return 0;
} 
  return 1;
}

char *AVDM_Fade::printConf( void )
{
	ADM_FILTER_DECLARE_CONF(" Fade : Start %u End %u",_param->startFade,_param->endFade);
}


AVDM_Fade::AVDM_Fade(AVDMGenericVideoStream *in,CONFcouple *couples)

{
                
  int count = 0;
  char buf[80];
  unsigned int *p;

  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1;
  vidCache=new VideoCache(3,in);
  
  _param=new VIDFADE_PARAM;
  if(couples)
  {
    GET(startFade);
    GET(endFade);
    GET(inOut);
    GET(toBlack);
    
  }else
  {
    _param->startFade=0; 
    _param->endFade=_info.nb_frames-1;
    _param->inOut=0;
    _param->toBlack=0;
  }
  buildLut();
}
//________________________________________________________
uint8_t AVDM_Fade::getCoupledConf( CONFcouple **couples)
{
  *couples=new CONFcouple(4);
  CSET(startFade);
  CSET(endFade);
  CSET(inOut);
  CSET(toBlack);
  return 1;
}
//________________________________________________________
AVDM_Fade::~AVDM_Fade(void)
{
                
  if(vidCache) delete vidCache;                
  vidCache=NULL;   
  if(_param) delete _param;
  _param=NULL;
}
uint8_t AVDM_Fade::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                            ADMImage *data,uint32_t *flags)
{

  uint32_t num_frames,tgt;
  
  ADMImage *src;

  num_frames=_info.nb_frames;   // ??

  tgt=frame+_info.orgFrame;
  if(frame>=num_frames)
  {
    printf("[Fade] out of bound\n");
    return 0;
  }

  src=vidCache->getImage(frame);
  if(!src) return 0;
  if(tgt>_param->endFade || tgt <_param->startFade ||_param->endFade==_param->startFade )
  {
    //printf("Cur %u start %u end %u\n",tgt,_param->startFade,_param->endFade);
    data->duplicate(src);
    vidCache->unlockAll();
    return 1;
  }
  uint8_t *s,*d,*s2;
  uint16_t *index,*invertedIndex;
  uint32_t count=_info.width*_info.height,w;
  float num,den;
  
  den=_param->endFade-_param->startFade;
  
  num=tgt-_param->startFade;
  
  num=num/den;
  num*=255.;
  w=(uint32_t)floor(num+0.4);
  
//printf("w :%u\n",w);

  s=src->data;
  d=data->data;
  if(_param->toBlack)
  {
        index=lookupLuma[w];
        for(int i=0;i<count;i++)
        {
          *d++=(index[*s++]>>8);
        }
        // Now do chroma
        count>>=2;
        s=UPLANE(src);
        d=UPLANE(data);
        index=lookupChroma[w];
        for(int i=0;i<count;i++)
        {
          *d++=(index[*s++]>>8);
        }
        s=VPLANE(src);
        d=VPLANE(data);
        for(int i=0;i<count;i++)
        {
          *d++=(index[*s++]>>8);
        }
  }
  else
  {
        uint32_t x,alpha;
        ADMImage *final;

        final=vidCache->getImage(_param->endFade-_info.orgFrame);
        if(!final)
        {
              data->duplicate(src);
              vidCache->unlockAll();
              return 1;
        }

        s2=final->data;

        index=lookupLuma[w];
        
        invertedIndex=lookupLuma[255-w];
        for(int i=0;i<count;i++)
        {
          *d++=(index[*s++]+invertedIndex[*s2++])>>8;
        }
        // Now do chroma
        count>>=2;
        s=UPLANE(src);
        d=UPLANE(data);
        s2=UPLANE(final);
        index=lookupChroma[w];
        invertedIndex=lookupChroma[255-w];
        for(int i=0;i<count;i++)
        {
            *d++=(index[*s++]+invertedIndex[*s2++]-(128<<8))>>8;
        }
        s=VPLANE(src);
        d=VPLANE(data);
        s2=VPLANE(final);
        for(int i=0;i<count;i++)
        {
            *d++=(index[*s++]+invertedIndex[*s2++]-(128<<8))>>8;
            
        }
  }
  vidCache->unlockAll();
  return 1;
}

uint8_t AVDM_Fade::buildLut(void)
{
  float f,ration;
  for(int i=0;i<256;i++)
  {
    if(!_param->inOut) ration=255-i;
    else ration=i;
    for(int r=0;r<256;r++)
    {
      f=r;
      f=f*ration;
      lookupLuma[i][r]=(uint16_t)(f+0.4);

      f=r-128;
      f=f*ration;
      lookupChroma[i][r]=(128<<8)+(uint16_t)(f+0.4);

    }
    
  }
  return 1;
}
//EOF




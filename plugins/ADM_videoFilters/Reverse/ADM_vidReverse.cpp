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


class AVDM_Reverse : public AVDMGenericVideoStream
{
  VideoCache      *vidCache;
  public:
                                
    AVDM_Reverse(AVDMGenericVideoStream *in,CONFcouple *couples);    
    ~AVDM_Reverse(void);
    uint8_t         getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);
        
    char            *printConf( void );
    uint8_t         configure(AVDMGenericVideoStream *in);
    uint8_t         getCoupledConf( CONFcouple **couples);
};

static FILTER_PARAM reverseParam={0,{""}};

VF_DEFINE_FILTER(AVDM_Reverse,reverseParam,
                reverse,
                QT_TR_NOOP("Reverse"),
                1,
                VF_TRANSFORM,
                QT_TR_NOOP("Play video backward."));
/*************************************/

uint8_t AVDM_Reverse::configure(AVDMGenericVideoStream *in)
{
  _in=in;
  return 1;
        
}

char *AVDM_Reverse::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Reverse");
  
}


AVDM_Reverse::AVDM_Reverse(AVDMGenericVideoStream *in,CONFcouple *couples)

{
                
  int count = 0;
  char buf[80];
  unsigned int *p;

  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1;
  vidCache=new VideoCache(18,in);
}
//________________________________________________________
uint8_t AVDM_Reverse::getCoupledConf( CONFcouple **couples)
{
  *couples=NULL;
  return 1;
}
//________________________________________________________
AVDM_Reverse::~AVDM_Reverse(void)
{
                
  if(vidCache) delete vidCache;                
  vidCache=NULL;   
}
uint8_t AVDM_Reverse::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                     ADMImage *data,uint32_t *flags)
{

  uint32_t num_frames,tgt;
  ADMImage *src;

  num_frames=_in->getInfo()->nb_frames;   // ??
  if(frame>=num_frames) return 0;
  
  tgt=num_frames-frame-1;

  src=vidCache->getImage(tgt);
  if(!src) return 0;
  data->duplicate(src);

  vidCache->unlockAll();
  return 1;
}
//EOF




//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//  Small wrapper for lavcodec deinterlacer postprocessing


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

#include "DIA_fileSel.h"

#include "ADM_colorspace.h"

#include "ADM_lavpp_deintparam.h"
extern "C"
{
#include "ADM_libraries/ADM_ffmpeg/ADM_libpostproc/postprocess.h"
};
#include "DIA_factory.h"
/*
{"al", "autolevels",            0, 1, 2, LEVEL_FIX},
{"lb", "linblenddeint",         1, 1, 4, LINEAR_BLEND_DEINT_FILTER},
{"li", "linipoldeint",          1, 1, 4, LINEAR_IPOL_DEINT_FILTER},
{"ci", "cubicipoldeint",        1, 1, 4, CUBIC_IPOL_DEINT_FILTER},
{"md", "mediandeint",           1, 1, 4, MEDIAN_DEINT_FILTER},
{"fd", "ffmpegdeint",           1, 1, 4, FFMPEG_DEINT_FILTER},
{"tn", "tmpnoise",              1, 7, 8, TEMP_NOISE_FILTER},

{"fq", "forcequant",            1, 0, 0, FORCE_QUANT}, *******************
{"l5", "lowpass5",              1, 1, 4, LOWPASS5_DEINT_FILTER}, *********
*/

#define AVI_KEY_FRAME 0x10  // FIXME
#define AVI_B_FRAME 0x4000


class  ADMVideoLavPPDeint:public AVDMGenericVideoStream
{

  protected:
    virtual char                    *printConf(void);
    void                            *ppcontext;
    void                            *ppmode;
    ADMImage                        *uncompressed;
    lavc_pp_param                   *_param;
    
    void                            setup( void );
    void                            cleanup( void );
  public:
    
    
    ADMVideoLavPPDeint(  AVDMGenericVideoStream *in,CONFcouple *setup);
    ~ADMVideoLavPPDeint();
    virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                          ADMImage *data,uint32_t *flags);
    virtual uint8_t getCoupledConf( CONFcouple **couples)           ;
    virtual uint8_t configure( AVDMGenericVideoStream *instream);
                                                        
};


static FILTER_PARAM lavppdeint_param={2,{"deintType","autolevel"}};
//*************************************************************
//

//*************************************************************
//********** Register chunk ************
//REGISTERX(VF_INTERLACING, "lavcppdeint",QT_TR_NOOP("libavcodec deinterlacer"),
//    QT_TR_NOOP("All FFmpeg deinterlace filters (bicubic, median, ...)."),VF_LAVPP_DEINT,1,lavppdeint_create,lavppdeint_script);

VF_DEFINE_FILTER(ADMVideoLavPPDeint,lavppdeint_param,
    lavcppdeint,
                QT_TR_NOOP("libavcodec deinterlacer"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("All FFmpeg deinterlace filters (bicubic, median, ...)."));
//********** Register chunk ************



//*************************************************************
uint8_t ADMVideoLavPPDeint::configure(AVDMGenericVideoStream *in)
{

  #define PX(x) &(_param->x)
  _in=in;
  
  
   diaMenuEntry menuField[6]={{PP_BM_NONE,        QT_TR_NOOP("None"),NULL},
                             {PP_BM_LINEAR_BLEND, QT_TR_NOOP("Linear blend"),NULL},
                             {PP_BM_LINEAR_INTER, QT_TR_NOOP("Linear interpolate"),NULL},
                             {PP_BM_CUBIC_INTER, QT_TR_NOOP("Cubic interpolate"),NULL},
                             {PP_BM_MEDIAN_INTER, QT_TR_NOOP("Median interpolate"),NULL},
                             {PP_BM_FFMPEG_DEINT, QT_TR_NOOP("FFmpeg deint"),NULL},
                          };
  
    
    diaElemMenu     menu1(PX(deintType),QT_TR_NOOP("_Deinterlacing:"), 6,menuField);
    diaElemToggle   autolevel(PX(autolevel),QT_TR_NOOP("_Autolevel"));
    
    diaElem *elems[2]={&menu1,&autolevel};
  
   if(diaFactoryRun(QT_TR_NOOP("libavcodec deinterlacer"),2,elems))
  {
    setup();
    return 1; 
  }
  return 0;        
}
 
//*************************************************************
char *ADMVideoLavPPDeint::printConf( void )
{
 ADM_FILTER_DECLARE_CONF(" Lavcodec PP deinterlacer autolev:%d deint:%d",_param->autolevel,_param->deintType);
  
}
//*************************************************************
ADMVideoLavPPDeint::ADMVideoLavPPDeint(  AVDMGenericVideoStream *in,CONFcouple *couples)
{

  _in=in;         
  memcpy(&_info,_in->getInfo(),sizeof(_info));    
  _info.encoding=1;  
  
  ppcontext=NULL;  
  ppmode=NULL;   
  
  _uncompressed=new ADMImage(_info.width,_info.height);
  if(couples)
  {   
    _param=NEW(lavc_pp_param);
    GET(deintType);    
    GET(autolevel);
  }      
  else
  {
    _param=NEW(lavc_pp_param);
    _param->deintType=0;
    _param->autolevel=0;
    
  } 
                 
  setup();
 
}
//*************************************************************
void ADMVideoLavPPDeint::cleanup(void)
{
  if(ppcontext)
  {
    pp_free_context(ppcontext);
    ppcontext=NULL;
  }
  if(ppmode)
  {
    pp_free_mode(ppmode);
    ppmode=NULL;
  }
  
} 
//*************************************************************
void ADMVideoLavPPDeint::setup(void)
{
  char string[1024];
  uint32_t ppCaps=0;
                
  string[0]=0;
  
        cleanup();
#ifdef ADM_CPU_X86
#define ADD(x,y) if( CpuCaps::has##x()) ppCaps|=PP_CPU_CAPS_##y;                
                ADD(MMX,MMX);           
                ADD(3DNOW,3DNOW);
                ADD(MMXEXT,MMX2);
#endif             
        cleanup();
#undef ADD       
#define ADD(z)  { if(string[0]) strcat(string,","#z); else strcpy(string,#z);}        
               
        if(_param->autolevel) ADD(al);
        switch(_param->deintType)
        {
          case PP_BM_NONE:break;
          case PP_BM_LINEAR_BLEND: ADD(lb);break;
          case PP_BM_LINEAR_INTER: ADD(li);break;
          case PP_BM_CUBIC_INTER: ADD(ci);break;
          case PP_BM_MEDIAN_INTER: ADD(md);break;
          case PP_BM_FFMPEG_DEINT: ADD(fd);break;                             
        }        


        ppcontext=pp_get_context(_info.width, _info.height, ppCaps);           
        ppmode=pp_get_mode_by_name_and_quality(string,1);;
        
        ADM_assert(ppcontext);
        ADM_assert(ppmode);
  
} 
//*************************************************************
ADMVideoLavPPDeint::~ADMVideoLavPPDeint()
{
  cleanup();
  if(_uncompressed)
        delete _uncompressed;
  DELETE(_param);
  _uncompressed=NULL;
}

//*************************************************************
uint8_t ADMVideoLavPPDeint::getCoupledConf( CONFcouple **couples)
{
  
  *couples=new CONFcouple(2);
  CSET(deintType);    
  CSET(autolevel);
  
  return 1;
}
//*************************************************************
uint8_t ADMVideoLavPPDeint::getFrameNumberNoAlloc(uint32_t frame,
                                              uint32_t *len,
                                              ADMImage *data,
                                              uint32_t *flags)
{

 
  if(frame>= _info.nb_frames) return 0;
        // read uncompressed frame
  if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

  
  //
  const uint8_t *iBuff[3];
   uint8_t *oBuff[3];
  int strideTab[3],strideTab2[3];                 
                                                                
  oBuff[0]=YPLANE(data);
  oBuff[1]=UPLANE(data);
  oBuff[2]=VPLANE(data);
                                
  iBuff[0]=YPLANE(_uncompressed);
  iBuff[1]=UPLANE(_uncompressed);
  iBuff[2]=VPLANE(_uncompressed);
                                
                                
  strideTab[0]=strideTab2[0]=_info.width;
  strideTab[1]=strideTab2[1]=_info.width>>1;
  strideTab[2]=strideTab2[2]=_info.width>>1;
        
  int type;
  if(_uncompressed->flags&AVI_KEY_FRAME)
    type=1;
  else if(_uncompressed->flags & AVI_B_FRAME)
    type=3;
  else
    type=2;
  pp_postprocess(
        iBuff,
        strideTab,
        oBuff,
        strideTab2,
        _info.width,
        _info.height,
        NULL,
        0,
        ppmode,
        ppcontext,
        type); // I ?
                                
  
  data->copyInfo(_uncompressed);
  return 1;
}

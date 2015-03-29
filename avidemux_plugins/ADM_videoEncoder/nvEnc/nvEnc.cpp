/***************************************************************************
                          \fn ADM_ffNvEnc
                          \brief Front end for libavcodec Mpeg4 asp encoder
                             -------------------

    copyright            : (C) 2002/2009 by mean
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
#include "nvEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#define USE_NV12 
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif
extern bool  loadNvEnc();

nvencconf NvEncSettings = NVENC_CONF_DEFAULT;

/**
        \fn ADM_nvEncEncoder
*/
ADM_nvEncEncoder::ADM_nvEncEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{    
    ADM_info("[ffNvEncEncoder] Creating.\n");
    nv12=NULL;

}

/**
    \fn pre-open
*/
bool ADM_nvEncEncoder::configureContext(void)
{
    switch(NvEncSettings.preset)
    {
#define MIAOU(x,y) case NV_PRESET_##x: ;break;  
        
     MIAOU(HP,"hp")   
     MIAOU(BD,"bd")   
     MIAOU(LL,"ll")   
     MIAOU(LLHP,"llhp")   
     MIAOU(LLHQ,"llhq")   
     MIAOU(HQ,"hq")                
default:break;
    }
    return true;             
}

/**
    \fn setup
*/
bool ADM_nvEncEncoder::setup(void)
{
    
    if(false==loadNvEnc())
    {
        ADM_warning("Cuda not available \n");
        return false;
    }
    ADM_warning("Cuda available \n");
    ADM_info("[ffMpeg] Setup ok\n");
    
    int w= getWidth();
    int h= getHeight();
    
    w=(w+31)&~31; // Try to be aligned
    
    nv12=new uint8_t[(w*h)/2]; 
    nv12Stride=w;
    
    return true;
}


/**
    \fn ~ADM_nvEncEncoder
*/
ADM_nvEncEncoder::~ADM_nvEncEncoder()
{
    ADM_info("[ffNvEncEncoder] Destroying.\n");
    if(nv12)
    {
        delete [] nv12;
        nv12=NULL;
    }

}

/**
    \fn encode
*/
bool         ADM_nvEncEncoder::encode (ADMBitstream * out)
{
    return false;
}
/**
 * 
 * @param l
 * @param d
 * @return 
 */
bool        ADM_nvEncEncoder::getExtraData(uint32_t *l,uint8_t **d)
{

}
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         nvEncConfigure(void)
{
diaMenuEntry mePreset[]={ 
  {NV_PRESET_HP,QT_TRANSLATE_NOOP("nvenc","Low Quality")},
  {NV_PRESET_HQ,QT_TRANSLATE_NOOP("nvenc","High Quality")},
  {NV_PRESET_BD,QT_TRANSLATE_NOOP("nvenc","BluRay")},
  {NV_PRESET_LL,QT_TRANSLATE_NOOP("nvenc","Low Latency")},
  {NV_PRESET_LLHP,QT_TRANSLATE_NOOP("nvenc","Low Latency (LQ)")},
  {NV_PRESET_LLHQ,QT_TRANSLATE_NOOP("nvenc","Low Latency (HQ)")}
};

        nvencconf *conf=&NvEncSettings;

#define PX(x) &(conf->x)

        diaElemMenu      qzPreset(PX(preset),QT_TRANSLATE_NOOP("ffnvenc","Preset:"),6,mePreset);        
        diaElemUInteger  bitrate(PX(bitrate),QT_TRANSLATE_NOOP("ffnvenc","Bitrate (kbps):"),1,50000);
        diaElemUInteger  maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("ffnvenc","Max Bitrate (kbps):"),1,50000);
          /* First Tab : encoding mode */
        diaElem *diamode[]={&qzPreset,&bitrate,&maxBitrate};

        if( diaFactoryRun(QT_TRANSLATE_NOOP("ffnvenc","libavcodec MPEG-4 configuration"),3,diamode))
        {
          
          return true;
        }
         return false;
}
// EOF

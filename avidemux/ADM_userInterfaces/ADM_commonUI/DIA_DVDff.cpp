/*
    Dialog for lavcodec based Mpeg1/mpeg2 codec


*/

#include "config.h"
#include "ADM_default.h"

#include "ADM_lavcodec.h"


#include "ADM_codecs/ADM_codec.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_codecs/ADM_ffmpegConfig.h"
#include "DIA_factory.h"

#warning FIXME Old : MaxBitrate in Byte/s, new entry in kb/s
#warning FIXME Old : MaxBitrate in Byte/s, new entry in kb/s
#warning FIXME Old : MaxBitrate in Byte/s, new entry in kb/s
#warning FIXME Old : MaxBitrate in Byte/s new entry in kb/s
#warning FIXME Old : MaxBitrate in Byte/s new entry in kb/s
#warning FIXME Old : MaxBitrate in Byte/s new entry in kb/s

#include "../../ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"

/**
      \fn DIA_DVDffParam
      \brief Dialog to set encoding options for SVCD/DVD lavcodec based
*/
//____________________________________________

uint8_t DIA_DVDffParam(COMPRES_PARAMS *incoming)
{
	

	int ret;
	FFcodecSetting *conf=(FFcodecSetting *)incoming->extraSettings;
	ADM_assert(incoming->extraSettingsLen==sizeof(FFcodecSetting));

diaMenuEntry wideM[]={
  {0,QT_TR_NOOP("4:3")},
  {1,QT_TR_NOOP("16:9")}};
diaMenuEntry matrixM[]={
  {0,QT_TR_NOOP("Default")},
  {1,QT_TR_NOOP("TMPGEnc")},
  {2,QT_TR_NOOP("Anime")},
  {3,QT_TR_NOOP("KVCD")}
};
diaMenuEntry interM[]={
  {0,QT_TR_NOOP("Progressive")},
  {1,QT_TR_NOOP("Interlaced TFF")},
  {2,QT_TR_NOOP("Interlaced BFF")}
};
diaMenuEntry vbvM[3]=
{
	{40,QT_TR_NOOP("VCD 40 kB")},
	{112,QT_TR_NOOP("SVCD 112 kB")},
	{224,QT_TR_NOOP("DVD 224 kB")}
};

                      
         diaElemBitrate bitrate(incoming,NULL);
         diaElemUInteger maxb(&(conf->maxBitrate),QT_TR_NOOP("Ma_x. bitrate:"),100,9000);
         diaElemUInteger minb(&(conf->minBitrate),QT_TR_NOOP("Mi_n. bitrate:"),0,9000);
         diaElemToggle    xvid(&(conf->use_xvid_ratecontrol),QT_TR_NOOP("_Use Xvid rate control"));
         
         diaElemMenu      vbv(&(conf->bufferSize),QT_TR_NOOP("_Buffer size:"),3,vbvM);
         
         diaElemMenu      widescreen(&(conf->widescreen),QT_TR_NOOP("Aspect _ratio:"),2,wideM);
         diaElemMenu      matrix(&(conf->user_matrix),QT_TR_NOOP("_Matrices:"),4,matrixM);
         diaElemUInteger  gop(&(conf->gop_size),QT_TR_NOOP("_GOP size:"),1,30);
         
uint32_t inter;
          if(!conf->interlaced) inter=0;
            else if(conf->bff) inter=2;
                else inter=1;
         diaElemMenu      interW(&inter,QT_TR_NOOP("_Interlacing:"),3,interM);
  
      diaElem *elems[9]={&bitrate,&maxb,&minb,&xvid,&vbv,&widescreen,&interW,&matrix,&gop};
    
  if( diaFactoryRun(QT_TR_NOOP("libavcodec MPEG-2 Configuration"),9,elems))
  {
    switch(inter)
    {
      case 0: conf->interlaced=0;conf->bff=0;break;
      case 1: conf->interlaced=1;conf->bff=0;break;
      case 2: conf->interlaced=1;conf->bff=1;break;
      default: ADM_assert(0);
    }
    return 1;
  }
  return 0;
}	
// EOF


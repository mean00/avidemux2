/*
    Simplified dialog for FLV1 codec
    Only bitrate and gop size


*/
#include "config.h"

#include "ADM_lavcodec.h"

#include "ADM_default.h"

#include "ADM_codecs/ADM_codec.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_codecs/ADM_ffmpegConfig.h"
#include "DIA_factory.h"
 

#include "../../ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"

/**
      \fn DIA_DVDffParam
      \brief Dialog to set encoding options for SVCD/DVD lavcodec based
*/
//____________________________________________

uint8_t DIA_flv1Param(COMPRES_PARAMS *incoming)
{
	

	int ret;
	FFcodecSetting *conf=(FFcodecSetting *)incoming->extraSettings;
	ADM_assert(incoming->extraSettingsLen==sizeof(FFcodecSetting));

                      
         diaElemUInteger  bitrate(&(incoming->bitrate),QT_TR_NOOP("_Bitrate (kb/s):"),100,9000);
         diaElemUInteger  gop(&(conf->gop_size),QT_TR_NOOP("_GOP size:"),1,250);

         diaElem *elems[2]={&bitrate,&gop};
    
  if( diaFactoryRun(QT_TR_NOOP("FLV1 Configuration"),2,elems))
  {
    return 1;
  }
  return 0;
}	
// EOF


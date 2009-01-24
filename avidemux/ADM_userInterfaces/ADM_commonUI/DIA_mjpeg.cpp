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

#include "../../ADM_encoder/adm_encmjpeg_param.h"

/**
      \fn DIA_mjpegCodecSetting
      \brief Dialog to set encoding options for Mjpeg lavcodec based
*/
//____________________________________________
uint8_t DIA_mjpegCodecSetting(COMPRES_PARAMS *param)
{
        MJPEGConfig *config=(MJPEGConfig *)param->extraSettings;
        ADM_assert(sizeof(MJPEGConfig)==param->extraSettingsLen);
        uint8_t ret=0;
        diaElemUInteger  qual(&(config->qual),QT_TR_NOOP("_Quality:"),1,100);
        diaElemToggle    swap(&(config->swapped),QT_TR_NOOP("_Swap U&V"));
        diaElem *elems[2]={&qual,&swap};
        if( diaFactoryRun(QT_TR_NOOP("MJPEG Configuration"),2,elems))
        {
            ret=1;
        }
        return ret;
        
}   
// EOF


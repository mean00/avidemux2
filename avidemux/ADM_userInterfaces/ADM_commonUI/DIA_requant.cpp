/*
    Dialog for lavcodec based Mpeg1/mpeg2 codec


*/
#include "config.h"

#if 0
#include "ADM_lavcodec.h"

#include "ADM_default.h"

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
uint8_t DIA_requant(COMPRES_PARAMS *param)
{
  uint32_t *pp;
  ELEM_TYPE_FLOAT fp;
        ADM_assert(param->extraSettingsLen==sizeof(uint32_t));
        pp=(uint32_t *)param->extraSettings;
        
       
        fp=(ELEM_TYPE_FLOAT)(*pp);
        fp/=1000;
        
       
        uint8_t ret=0;
        diaElemFloat  shrink(&fp,QT_TR_NOOP("_Shrink Factor:"),1,4.0);
        
        diaElem *elems[1]={&shrink};
        if( diaFactoryRun(QT_TR_NOOP("Requant Configuration"),1,elems))
        {
            ret=1;
            *pp=(uint32_t)(fp*1000);
        }
        return ret;
        
}   
#endif
// EOF


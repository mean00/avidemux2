/***************************************************************************
                          \fn     libvaEnc_plugin
                          \brief  Plugin to use libva hw encoder (intel mostly)
                             -------------------

    copyright            : (C) 2018 by mean
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
#include "ADM_libvaEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "DIA_factory.h"
#include "ADM_coreLibVA_h264Encoding.h"

#include "vaenc_settings.h"
extern "C"
{
#include "vaenc_settings_desc.cpp"
}
extern bool     vaEncConfigure(void);
 static const  vaconf_settings defaultConf = {10*1000,100};
vaconf_settings vaH264Settings=defaultConf;
/**
 * 
 */
void resetConfigurationData()
{  
    memcpy(&vaH264Settings, &defaultConf, sizeof(vaH264Settings));
}

/**
 * 
 * @return 
 */
static bool         ADM_libvaEncoder_probe()
{
    VAProfile profile=vaGetH264EncoderProfile()->profile;
    if(profile==VAProfileNone)
    {
        ADM_error("No H264 encoding support\n");
        return false;
    }    
    return true;
} 
ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(ADM_libvaEncoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN_EX("LibVaEncoder (HW)",
                               "Intel AVC HW (VA)",
                               "Simple Libva Encoder (c) 2018 Mean",
                                vaEncConfigure, // No configuration
                                ADM_UI_ALL,
                                1,0,0,
                                vaconf_settings_param, // conf template
                                &vaH264Settings, // conf var
                                NULL,NULL ,       // setProfile,getProfile)
                                ADM_libvaEncoder_probe                                  
);
/**
 * 
 * @return 
 */
bool vaEncConfigure(void)
{

    diaElemUInteger  period(&(vaH264Settings.IntraPeriod),QT_TRANSLATE_NOOP("vaH264","_IDR Period:"),1,1000);
    diaElemUInteger  bitrate(&(vaH264Settings.BitrateKbps),QT_TRANSLATE_NOOP("vaH264","_Bitrate(kbps)"),1,100*1000);

    diaElem *elems[2]={&bitrate,&period};    
    return diaFactoryRun(QT_TRANSLATE_NOOP("vaEncH264","vaH264 Configuration"),2 ,elems);
}
// EOF
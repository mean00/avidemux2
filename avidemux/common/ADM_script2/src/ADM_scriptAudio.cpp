/**
    \file ADM_jsAudio.cpp
    \brief Audio oriented functions
    \author mean (c) 2009 fixounet@free.fr


    jsapigen does not like much variable number of arguments
    In that case, we patch the generated file to go back to native spidermonkey api


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_scriptIf.h"
#include "ADM_editor/ADM_edit.hxx"
#include "A_functions.h"
#include "GUI_ui.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptAudio.h"
/**
    \fn int jsAudioReset(void);
*/
int scriptAudioReset (void)
{
    audioFilterReset();
    return 1;
}
/**
    \fn jsAudioMixer
*/
int scriptAudioMixer(const char *s)
{
    CHANNEL_CONF c=AudioMuxerStringToId(s);
    return audioFilterSetMixer(c);
}
/**
    \fn jsGetResample
*/
int32_t scriptGetResample(void)
{
    return 0;
}
/**
    \fn jsSetResample
*/
void    scriptSetResample(int32_t fq)
{

}
/**

*/
/**
    \fn scriptSetAudioCodec
*/
int     scriptSetAudioCodec(const char *codec,int bitrate,CONFcouple *c)
{ 
        int r=true;        
        // First search the codec by its name
        if(!audioCodecSetByName(codec))
        {
                r=false;
                jsLogError("Cannot set audio codec %s\n",codec);
        }
        else
        {
            r=setAudioExtraConf(bitrate,c);
            
        }
        if(c) delete c;
        return r;
}
//EOF
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
extern ADM_Composer *video_body;
/**
    \fn int jsAudioReset(void);
*/
int jsAudioReset (void)
{
    audioFilterReset();
    return 1;
}
/**
    \fn jsAudioMixer
*/
int jsAudioMixer(const char *s)
{
    CHANNEL_CONF c=AudioMuxerStringToId(s);
    return audioFilterSetMixer(c);
}
/**
    \fn jsGetResample
*/
int32_t jsGetResample(void)
{
    return 0;
}
/**
    \fn jsSetResample
*/
void    jsSetResample(int32_t fq)
{

}
//EOF
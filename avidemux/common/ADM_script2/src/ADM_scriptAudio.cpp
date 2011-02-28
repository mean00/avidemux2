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
#include "ADM_cpp.h"
#include "ADM_scriptIf.h"
#include "ADM_editor/ADM_edit.hxx"
#include "A_functions.h"
#include "GUI_ui.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptAudio.h"

extern ADM_Composer *video_body;

int A_setAudioTrack(int track);
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
    \fn scriptGetNormalizeMode
*/
int scriptGetNormalizeMode(void)
{
    ADM_GAINMode m;
    uint32_t gain;
    audioFilterGetNormalize( &m,&gain);
    return m;
}
/**
    \fn scriptGetNormalizeValue
*/
int scriptGetNormalizeValue(void)
{
    ADM_GAINMode m;
    uint32_t gain;
    audioFilterGetNormalize( &m,&gain);
    return (int)gain;
}

/**
    \fn scriptSetNormalizeMode
*/
void scriptSetNormalizeMode(int mode)
{
    ADM_GAINMode m;
    uint32_t gain;
    audioFilterGetNormalize( &m,&gain);
    m=(ADM_GAINMode)mode;
    audioFilterSetNormalize( m,gain);
}
/**
    \fn scriptSetNormalizeValue
*/
void scriptSetNormalizeValue(int value)
{
    ADM_GAINMode m;
    uint32_t gain;
    audioFilterGetNormalize( &m,&gain);
    gain=(uint32_t)value;
    audioFilterSetNormalize( m,gain);

}
/**
    \fn jsGetResample
*/
int32_t scriptGetResample(void)
{
    return audioFilterGetResample();;
}
/**
    \fn jsSetResample
*/
void    scriptSetResample(int32_t fq)
{
    audioFilterSetResample(fq);
}
/**

*/
int32_t scriptGetPal2film(void)
{
    if(audioFilterGetFrameRate()==FILMCONV_PAL2FILM) return 1;
    return 0;
}
/**

*/
int32_t scriptGetFilm2pal(void)
{
    if(audioFilterGetFrameRate()==FILMCONV_FILM2PAL) return 1;
    return 0;
}
/**

*/
void    scriptSetPal2film(int32_t rate)
{
    if(rate)
        audioFilterSetFrameRate(FILMCONV_PAL2FILM);
    else
    {
        if(scriptGetPal2film())
            audioFilterSetFrameRate(FILMCONV_NONE);
    }
}
/**

*/
void    scriptSetFilm2pal(int32_t rate)
{
    if(rate)
        audioFilterSetFrameRate(FILMCONV_FILM2PAL);
    else
    {
        if(scriptGetFilm2pal())
            audioFilterSetFrameRate(FILMCONV_NONE);
    }
}

/**
    \fn  scriptAudioSetTrack
*/
int scriptAudioSetTrack(int track)
{
        uint32_t nbAudioTracks,currentAudioTrack;
        audioInfo *infos=NULL;

        if(!video_body->getAudioStreamsInfo(0,&nbAudioTracks,&infos)) 
            return false;
        if(nbAudioTracks<=track) return false;
        A_setAudioTrack(track);
        return true;
}
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

/**
    \fn script setAudioFrequency
    \brief stubs, read only attributes
*/
void    scriptSetAudioFrequency(int fq)
{
    ADM_error("Cannot write audio frequency\n");
}

void    scriptSetAudioEncoding(int)
{
    ADM_error("Cannot write audio encoding\n");
}

void    scriptSetAudioChannels(int dq)
{
    ADM_error("Cannot write audio channel\n");
}
static bool audioProlog(audioInfo **info)
{
        uint32_t  nbAudioTracks;
        audioInfo *infos=NULL;
    
        if(!video_body->getAudioStreamsInfo(0,&nbAudioTracks,&infos)) 
        {
            ADM_warning("There is no audio track\n");
            return false;
        }
        int track=video_body->getCurrentAudioStreamNumber(0);
        *info=infos+track;
        return true;
}
/**
    \fn scriptGetAudioChannels
*/
int     scriptGetAudioChannels(void)
{
        audioInfo *info=NULL;
        if(false==audioProlog(&info)) return 0;
        return info->channels;
}
/**
    \fn scriptGetAudioFrequency
*/
int     scriptGetAudioFrequency(void)
{
        audioInfo *info=NULL;
        if(false==audioProlog(&info)) return 0;
        return info->frequency;

}
/**
    \fn scriptGetAudioEncoding
*/
int     scriptGetAudioEncoding(void)
{
        audioInfo *info=NULL;
        if(false==audioProlog(&info)) return 0;
        return info->encoding;
}

//EOF

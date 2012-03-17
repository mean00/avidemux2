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
#include "ADM_edit.hxx"
#include "A_functions.h"
#include "GUI_ui.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptAudio.h"

extern ADM_Composer *video_body;

int A_setAudioTrack(int track);
#define AUDIO_ACTION(r,i,x) \
    { EditableAudioTrack *ed=video_body->getEditableAudioTrackAt(i);\
    if(ed) \
    {\
        r=ed->audioEncodingConfig.x;\
    }\
    else \
    {\
        ADM_warning("No audio track at index %d\n",i);\
        return 0;\
    }\
    }
/**
    \fn int jsAudioReset(void);
*/
int scriptAudioReset (int i)
{
    int r;
    AUDIO_ACTION(r,i,reset());
    return 1;
}
/**
    \fn jsAudioMixer
*/
int scriptAudioMixer(int i,const char *s)
{
    int r;
    CHANNEL_CONF c=AudioMuxerStringToId(s);
    AUDIO_ACTION(r,i,reset());
    return r;
}
/**
    \fn scriptGetNormalizeMode
*/
int scriptGetNormalizeMode(int i)
{
    ADM_GAINMode m;
    uint32_t gain;
    int r;
    AUDIO_ACTION(r,i,audioFilterGetNormalize( &m,&gain));
    return m;
}
/**
    \fn scriptGetNormalizeValue
*/
int scriptGetNormalizeValue(int i)
{
    ADM_GAINMode m;
    uint32_t gain;
    int r;
    AUDIO_ACTION(r,i,audioFilterGetNormalize( &m,&gain));
    return (int)gain;
}

/**
    \fn scriptSetNormalizeMode
*/
int scriptSetNormalizeMode(int i,int mode)
{
    int r;
    ADM_GAINMode m;
    uint32_t gain;
    AUDIO_ACTION(r,i,audioFilterGetNormalize( &m,&gain));
    m=(ADM_GAINMode)mode;
    AUDIO_ACTION(r,i,audioFilterSetNormalize( m,gain));
}
/**
    \fn scriptSetNormalizeValue
*/
int scriptSetNormalizeValue(int i,int value)
{
    int r;
    ADM_GAINMode m;
    uint32_t gain;
    AUDIO_ACTION(r,i,audioFilterGetNormalize( &m,&gain));
    gain=(uint32_t)value;
    AUDIO_ACTION(r,i,audioFilterSetNormalize( m,gain));

}
/**
    \fn jsGetResample
*/
int32_t scriptGetResample(int i)
{
    int r;
    AUDIO_ACTION(r,i,audioFilterGetResample());
    return r;
}
/**
    \fn jsSetResample
*/
int    scriptSetResample(int i,int32_t fq)
{
    int r;
    AUDIO_ACTION(r,i, audioFilterSetResample(fq));
    return r;
}
/**
    \fn scriptGetPal2film
*/
int32_t scriptGetPal2film(int i)
{
    FILMCONV r;
    AUDIO_ACTION(r,i,audioFilterGetFrameRate());
    if(r==FILMCONV_PAL2FILM) return 1;
    return 0;
}
/**
    \fn scriptGetPal2film
*/
int32_t scriptGetFilm2Pal(int i)
{
    FILMCONV r;
    AUDIO_ACTION(r,i,audioFilterGetFrameRate());
    if(r==FILMCONV_FILM2PAL) return 1;
    return 0;
}

/**
    \fn scriptSetPal2film
*/
int    scriptSetPal2film(int i,int32_t rate)
{
    int r;
    if(rate)
    {
            AUDIO_ACTION(r,i,audioFilterSetFrameRate(FILMCONV_PAL2FILM));
    }
    else
    {
        if(scriptGetPal2film(i))
                AUDIO_ACTION(r,i,audioFilterSetFrameRate(FILMCONV_NONE));
    }
}
/**
        \fn scriptSetFilm2pal
*/
int    scriptSetFilm2pal(int i,int32_t rate)
{
    int r;
    if(rate)
    {
        AUDIO_ACTION(r,i,audioFilterSetFrameRate(FILMCONV_FILM2PAL));
    }
    else
    {
        if(scriptGetFilm2Pal(i))
        {
            AUDIO_ACTION(r,i,audioFilterSetFrameRate(FILMCONV_NONE));
        }
    }
}
#if 0
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
#endif
/**
    \fn scriptSetAudioCodec
*/
int     scriptSetAudioCodec(int dex,const char *codec,int bitrate,CONFcouple *c)
{ 
        int r=true;        
        // First search the codec by its name
        if(!audioCodecSetByName(dex,codec))
        {
                r=false;
                jsLogError("Cannot set audio codec %s\n",codec);
        }
        else
        {
            r=setAudioExtraConf(dex,bitrate,c);
            
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

/***************************************************************************
                          audiodeng_buildfilters.cpp  -  description
                             -------------------
    begin                : Mon Dec 2 2002
    copyright            : (C) 2002 by mean
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
#include "config.h"
#include <math.h>

#include "ADM_default.h"

#include "ADM_audiofilter/audiofilter_limiter_param.h"
#include "ADM_audiofilter/audiofilter_normalize_param.h"

#include "ADM_audiofilter/audioeng_buildfilters.h"


#include "audioencoder.h"

#include "ADM_audiocodec/ADM_audiocodeclist.h"


#include "audiofilter_bridge.h"
#include "audiofilter_mixer.h"
#include "audiofilter_normalize.h"
#include "audiofilter_limiter.h"

#include "prefs.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_FILTER
#include "ADM_osSupport/ADM_debug.h"

extern void UI_setAProcessToggleStatus( uint8_t status );
extern uint8_t DIA_audioCodec( int *codec );
extern void UI_setAudioCodec( int i);
uint32_t audioFilterGetNbEncoder(void);
const char* audioFilterGetIndexedName(uint32_t i);


typedef struct externalSource
{
 AudioSource type;
 const char *name;
}externalSource;

static const externalSource Sources[]=
{
        {AudioAvi,"VIDEO"},
        {AudioMP3,"MP3"},
        {AudioWav,"WAV"},
        {AudioAC3,"AC3"},
        {AudioNone,"NONE"}
};
typedef struct
{
  const char         *name;
  CHANNEL_CONF conf;
}Mixer_String;

#define DOME(x) {#x,CHANNEL_##x}
static Mixer_String Mixer_strings[]=
{
  {"NONE",CHANNEL_INVALID},
  DOME(MONO),
  DOME(STEREO),
  DOME(2F_1R),
  DOME(3F),
  DOME(3F_1R),
  DOME(2F_2R),
  DOME(3F_2R),
  DOME(3F_2R_LFE),
  DOME(DOLBY_PROLOGIC),
  DOME(DOLBY_PROLOGIC2)

};



extern void UI_PrintCurrentACodec( const char *s);


/*----------------------------------*/
GAINparam audioGain;
int  audioFreq=48000;
int  audioDRC = 0;
FILMCONV audioFilmConv=FILMCONV_NONE;
RESAMPLING  audioResampleMode = RESAMPLING_NONE;
CHANNEL_CONF audioMixing=CHANNEL_INVALID;
// These are globals for the moment
//************************************
int 	   audioShift = 0;
int	   audioDelay=0;
//**********

const char              *getCurrentMixerString(void)
{
        uint32_t nb=sizeof(Mixer_strings)/sizeof(Mixer_String);
        for(uint32_t i=0;i<nb;i++)
                if(audioMixing==Mixer_strings[i].conf) return Mixer_strings[i].name;
        ADM_assert(0);

}
uint8_t                    setCurrentMixerFromString(const char *name)
{
        uint32_t nb=sizeof(Mixer_strings)/sizeof(Mixer_String);
        for(uint32_t i=0;i<nb;i++)
                if(!strcasecmp(name,Mixer_strings[i].name))
                {
                  audioMixing= Mixer_strings[i].conf;
                  return 1;
                }
        return 0;

}
//**********

const char              *audioSourceFromEnum(AudioSource src)
{
        uint32_t nb=sizeof(Sources)/sizeof(externalSource);
        for(uint32_t i=0;i<nb;i++)
                if(src==Sources[i].type) return Sources[i].name;
        ADM_assert(0);

}
AudioSource             audioSourceFromString(const char *name)
{
        uint32_t nb=sizeof(Sources)/sizeof(externalSource);
        for(uint32_t i=0;i<nb;i++)
                if(!strcasecmp(name,Sources[i].name)) return Sources[i].type;
        return (AudioSource)0;

}

//**********
uint8_t audioReset(void )
{
  audioGain.mode=ADM_NO_GAIN;
  audioResampleMode = RESAMPLING_NONE;
  audioFilmConv=FILMCONV_NONE;
  audioMixing=CHANNEL_INVALID;
  return 1;
}
//************
uint8_t audioGetNormalizeMode(void)
{
  return audioGain.mode;

}
int32_t  audioGetNormalizeValue(void)
{
  return audioGain.gain10;

}

uint32_t audioGetResample(void)
{
      return audioFreq;

}
uint32_t audioGetDrc(void)
{
  return audioDRC;
}
uint32_t audioGetDelay(void)
{
        if(audioShift && audioDelay)
        {
                return audioDelay;

        }
        return 0;
}
FILMCONV audioGetFpsConv(void)
{
        return audioFilmConv;
}

/*----------------------------------*/
//




void audioFilterNormalizeMode(uint8_t onoff)
{
  audioGain.mode=(ADM_GAINMode)onoff;
}
void audioFilterNormalizeValue(int v)
{
  audioGain.gain10=v;
}

uint8_t audioFilterDelay(int32_t delay)
{
	if(delay)
	{
		audioShift=1;
		audioDelay=delay;
	}
	else
	{
		audioShift=audioDelay=0;
	}
	return 1;

}
RESAMPLING  audioGetResampling(void)
{
        return audioResampleMode;
}
uint8_t audioFilterFilm2Pal(uint8_t onoff)
{
	if(onoff) audioFilmConv=FILMCONV_FILM2PAL;
	else audioFilmConv=FILMCONV_NONE;
	return 1;
}
uint8_t audioFilterDrc(uint8_t onoff)
{
  audioDRC=onoff;
  return 1;
}
uint8_t audioFilterPal2Film(uint8_t onoff)
{
        if(onoff) audioFilmConv=FILMCONV_PAL2FILM;
        else audioFilmConv=FILMCONV_NONE;
        return 1;
}

void audioFilterResample(uint32_t onoff)
{
	if(onoff)
	{
		audioResampleMode=RESAMPLING_CUSTOM;
		audioFreq=onoff;
	}
	else
		audioResampleMode=RESAMPLING_NONE;

}
//______________________________
//#include "ADM_gui2/GUI_ui.h"
uint8_t UI_setTimeShift(int onoff,int value);
extern  int DIA_getAudioFilter(GAINparam *normalized, RESAMPLING *downsamplingmethod, int *tshifted,
  			 int *shiftvalue, int *drc,int *freqvalue,FILMCONV *filmconv,CHANNEL_CONF *channel);

void audioFilter_configureFilters( void )
{
    int olddelay=audioDelay;
    int oldshift=audioShift;
	 DIA_getAudioFilter(&audioGain,&audioResampleMode,&audioShift,&audioDelay,&audioDRC,&audioFreq,
	 		&audioFilmConv,&audioMixing );
         if(audioDelay!=olddelay ||oldshift!= audioShift)
         {  // Refresh



         }

}

/*

*/

#define Read(x) { \
		tmp=name; \
		if((tmp=strstr(name,#x))) \
			{ \
				tmp+=strlen(#x); \
				aprintf("-- %s\n",tmp); \
				sscanf(tmp,"=%d ",(int *)&x); \
			} \
			 else \
			{ printf("*** %s not found !***\n",#x);} \
		}
#define Add(x) {sprintf(tmp,"%s=%d ",#x,x);strcat(conf,tmp);}

uint8_t audioFilterSetByName( const char *name)
{
	const char *tmp;
	aprintf("-Audio filter by name : %s\n",name);

	Read(audioGain.mode);
        Read(audioGain.gain10);
	Read(audioResampleMode);
	Read(audioDRC);
	Read(audioShift);
	Read(audioDelay);
	Read(audioFreq);
	Read(audioMixing);
	return 1;
}

const char *audioFilterGetName( void )
{
	static char conf[400];
	static char tmp[200];
	conf[0]=0;
	#undef Add
	#define Add(x) {sprintf(tmp,"%s=%d ",#x,x);strcat(conf,tmp);}
	Add(audioGain.mode);
        Add(audioGain.gain10);
	Add(audioResampleMode);
	Add(audioDRC);
	Add(audioShift);
	Add(audioDelay);
	Add(audioFreq);
	Add(audioMixing);
	return conf;

}


/*
	Refresh   activeAudioEncoder value
	depending on what's selected

*/



void audioSetResample(uint32_t fq)
{

	audioResampleMode=RESAMPLING_CUSTOM;
	audioFreq=fq;
}
/**
 * 	\fn getAudioOuputFrequency
 *  \brief Return the encoding of the currently selected codec
 */
uint32_t audioProcessMode(void);
uint32_t audioFilter_getOuputFrequency(uint32_t inputFrequency)
{
	if(!audioProcessMode()) return inputFrequency;
	if(audioResampleMode == RESAMPLING_NONE) return inputFrequency;
	return audioFreq;
}
/**
    \fn audioCodecGetName
    \brief Returns the current codec tagname
*/

AudioSource audioCodecGetFromName( const char *name)
{
                for(uint32_t i=0;i<sizeof(Sources)/sizeof(externalSource);i++)
                {
                        if(!strcasecmp(name,Sources[i].name))
                        {

                                return Sources[i].type;
                        }

                }
                printf("\n Mmmm Select audio codec by name failed...(%s).\n",name);
                return AudioNone;
}

//EOF


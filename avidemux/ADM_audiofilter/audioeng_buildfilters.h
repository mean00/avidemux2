/**************************************************************************
                          audioeng_buildfilters.h  -  description
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
#ifndef ADM_AUDIO_BUILDFILTER_H
#define  ADM_AUDIO_BUILDFILTER_H

#include "audioencoder.h"
#include "ADM_audio/aviaudio.hxx"

#include "audioEncoderApi.h"
#include "ADM_audioStream.h"

 ADM_audioStream *buildAudioFilter(ADM_audioStream *stream, uint32_t startTime);
 AUDMAudioFilter *buildPlaybackFilter(ADM_audioStream *currentaudiostream,
				uint32_t starttime, uint32_t duration);

 void deleteAudioFilter(ADM_audioStream *in);
void audioFilter_configureFilters( void );



void audioCodecConfigure( void );
void audioCodecSelect( void );
void audioFilter_MP3DisableReservoir(int onoff);
uint32_t audioFilter_getOuputCodec(void);
uint32_t audioFilter_getOuputFrequency(uint32_t inputFrequency);
uint32_t audioFilter_getMaxChannels(void);

//#include "ADM_audioEncoder/include/audioencoder_enum.h"

typedef enum 
{
	RESAMPLING_NONE=0,
	RESAMPLING_CUSTOM=1,
	RESAMPLING_LAST
}RESAMPLING;

typedef enum 
{
	FILMCONV_NONE=0,
	FILMCONV_FILM2PAL=1,
	FILMCONV_PAL2FILM=2,
	FILMCONV_LAST
}FILMCONV;

typedef enum
{
        AudioInvalid,
        AudioAvi=1,
        AudioMP3,
        AudioWav,
        AudioAC3,
        AudioNone

}AudioSource;

//void audioCodecSetcodec(AUDIOENCODER codec);
uint8_t audioReset(void);
/*  for job/workspace stuff  */
const char *audioFilterGetName( void );
uint8_t audioFilterSetByName( const char *name);

//AVDMGenericAudioStream *mpt_getAudioStream(void);

/* -- Set filter --*/
void audioFilterNormalizeMode(uint8_t onoff);
void audioFilterNormalizeValue(int value);
void audioFilterResample(uint32_t onoff);
uint8_t audioFilterDelay(int32_t delay);
uint8_t audioFilterFilm2Pal(uint8_t onoff);
uint8_t audioFilterPal2Film(uint8_t onoff);
uint8_t audioFilterDrc(uint8_t onoff);
void audioFilter_SetBitrate( int i);
/* -- Get filter -- */
uint32_t audioGetBitrate(void);
uint8_t audioGetNormalizeMode(void);
int32_t audioGetNormalizeValue(void);
uint32_t audioGetResample(void);
uint32_t audioGetDelay(void);
FILMCONV audioGetFpsConv(void);
uint32_t audioGetDrc(void);
RESAMPLING  audioGetResampling(void);
/*-----*/
uint8_t                 A_changeAudioStream(ADM_audioStream *newaudio,AudioSource so,char *name);
AudioSource             getCurrentAudioSource(char **name);
const char              *audioSourceFromEnum(AudioSource src);
AudioSource             audioSourceFromString(const char *name);
const char              *getCurrentMixerString(void);
uint8_t                 setCurrentMixerFromString(const char *string);

//*****
//
#endif


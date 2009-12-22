/***************************************************************************
                        audioouput.h  -  description
                             -------------------
    begin                : Thu Dec 27 2001
    copyright            : (C) 2001 by mean
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

#ifndef AUDIOOUPUT_H
#define AUDIOOUPUT_H
#include "ADM_audiodef.h"
typedef int AUDIO_DEVICE;

void 		AVDM_audioSave( void ); /// Save in Prefs the current audio Device
void 		AVDM_audioInit( void );
void		AVDM_cleanup(void);
void 		AVDM_switch( AUDIO_DEVICE action );

uint8_t 	AVDM_AudioPlay(float *ptr, uint32_t nb);
uint32_t 	AVDM_AudioSetup(uint32_t fq, uint8_t channel, CHANNEL_TYPE    *channelMapping);
void 		AVDM_AudioClose(void);
uint32_t    AVDM_GetLayencyMs(void);
AUDIO_DEVICE 	AVDM_getCurrentDevice( void);
uint8_t         AVDM_setVolume(int volume);
// Get infos
uint32_t    ADM_av_getNbDevices(void);
bool        ADM_av_getDeviceInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t    AVDM_getMsFullness(void); /// Fullness in ms
bool        AVDM_getStats(uint32_t *vol); // Retrieve volume info
#endif


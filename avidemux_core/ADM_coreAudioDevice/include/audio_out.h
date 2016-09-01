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

#include "ADM_coreAudioDevice6_export.h"
#include "ADM_audiodef.h"

typedef int AUDIO_DEVICE;

ADM_COREAUDIODEVICE6_EXPORT void 		AVDM_audioSave( void ); /// Save in Prefs the current audio Device
ADM_COREAUDIODEVICE6_EXPORT void 		AVDM_audioInit( void );
ADM_COREAUDIODEVICE6_EXPORT void		AVDM_cleanup(void);
ADM_COREAUDIODEVICE6_EXPORT void 		AVDM_switch( AUDIO_DEVICE action );

ADM_COREAUDIODEVICE6_EXPORT uint8_t 	AVDM_AudioPlay(float *ptr, uint32_t nb);
ADM_COREAUDIODEVICE6_EXPORT uint32_t 	AVDM_AudioSetup(uint32_t fq, uint8_t channel, CHANNEL_TYPE    *channelMapping);
ADM_COREAUDIODEVICE6_EXPORT void 		AVDM_AudioClose(void);
ADM_COREAUDIODEVICE6_EXPORT uint32_t    AVDM_GetLayencyMs(void);
ADM_COREAUDIODEVICE6_EXPORT AUDIO_DEVICE 	AVDM_getCurrentDevice( void);
ADM_COREAUDIODEVICE6_EXPORT uint8_t         AVDM_setVolume(int volume);
// Get infos
ADM_COREAUDIODEVICE6_EXPORT uint32_t    ADM_av_getNbDevices(void);
ADM_COREAUDIODEVICE6_EXPORT bool        ADM_av_getDeviceInfo(int filter, std::string &name, uint32_t *major,uint32_t *minor,uint32_t *patch);
ADM_COREAUDIODEVICE6_EXPORT uint32_t    AVDM_getMsFullness(void); /// Fullness in ms
ADM_COREAUDIODEVICE6_EXPORT bool        AVDM_getStats(uint32_t *vol); // Retrieve volume info
#endif


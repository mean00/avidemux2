
/**
    \file  ADM_scriptAudio
    \brief Audio codec etc..
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_SCRIPT_AUDIO_H
#define ADM_SCRIPT_AUDIO_H

#include "ADM_confCouple.h"
#ifdef __cplusplus
extern "C" {
#endif
int     scriptSetAudioCodec(const char *codec,int bitrate,CONFcouple *c);
int     scriptAudioSetTrack(int trackNo);


void    scriptSetAudioFrequency(int fq);
int     scriptGetAudioFrequency(void);

void    scriptSetAudioChannels(int dq);
int     scriptGetAudioChannels(void);

int     scriptGetAudioEncoding(void);
void    scriptSetAudioEncoding(int);


#ifdef __cplusplus
};
#endif

#endif

/**
    \file ADM_psAudioProbe.h
    \brief Probe audio tracks by brute force analysus of the file
    copyright            : (C) 2009 by mean
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
#ifndef ADM_PS_AUDIO_PROBE_H
#define ADM_PS_AUDIO_PROBE_H
#include <BVector.h>

typedef struct
{
    WAVHeader header;
    uint8_t   esID;
}psAudioTrackInfo;

typedef BVector <psAudioTrackInfo*> listOfPsAudioTracks;

/// Returns a list of audio tracks found in the file.
listOfPsAudioTracks *psProbeAudio(const char *fileName);
bool DestroyListOfPsAudioTracks(listOfPsAudioTracks *list);


#endif
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
#ifndef ADM_TS_AUDIO_PROBE_H
#define ADM_TS_AUDIO_PROBE_H
#include <vector>
using std::vector;
typedef struct
{
    WAVHeader header;
    uint8_t   esID;
}tsAudioTrackInfo;

typedef vector <tsAudioTrackInfo*> listOfTsAudioTracks;

/// Returns a list of audio tracks found in the file.
listOfTsAudioTracks *tsProbeAudio(const char *fileName);
bool DestroyListOfTsAudioTracks(listOfTsAudioTracks *list);


#endif
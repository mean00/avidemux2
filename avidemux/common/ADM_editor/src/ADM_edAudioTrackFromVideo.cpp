/***************************************************************************
    \file  ADM_edAudioTrackFromVideo
    \brief Manage audio track(s) coming from a video
(c) 2012 Mean, fixounet@free.Fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>


#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
/**
    \fn ctor
*/
ADM_edAudioTrackFromVideo::ADM_edAudioTrackFromVideo(int trackNumber, ADM_Composer *parent)
:  ADM_edAudioTrack(ADM_EDAUDIO_FROM_VIDEO,parent)
{
    
    myTrackNumber=trackNumber;
    _audioSeg=0;
}
/**
    \fn dtor
*/
ADM_edAudioTrackFromVideo::~ADM_edAudioTrackFromVideo()
{
    // No need to destroy, we are just a wrapper
}
/**
    \fn getDurationInUs
*/
uint64_t ADM_edAudioTrackFromVideo::getDurationInUs()
{
    // get duratio...
}

/**
    \fn getDefaultAudioTrack
*/
bool        ADM_Composer::getDefaultAudioTrack(ADM_audioStream **stream)
{
        *stream=NULL;
        if(audioTrackPool.size()) 
        {
            *stream=audioTrackPool.at(0);
            return true;
        }else return false;
}
/**
    \fn getDefaultAudioTrackFromVideo
*/
ADM_edAudioTrackFromVideo     *ADM_Composer::getDefaultAudioTrackFromVideo(void)
{
        if(audioTrackPool.size()) 
        {
            return audioTrackPool.at(0);
        
        }
        return NULL;
}
// EOF

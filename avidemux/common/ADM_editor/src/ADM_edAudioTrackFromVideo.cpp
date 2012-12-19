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
ADM_edAudioTrackFromVideo::ADM_edAudioTrackFromVideo(ADM_audioStreamTrack *track,int trackNumber, ADM_Composer *parent)
:  ADM_edAudioTrack(ADM_EDAUDIO_FROM_VIDEO,NULL)
{
    ADM_info("Creating edAudio from video track %d at %"PRIx32"\n",trackNumber,this);
    myTrackNumber=trackNumber;
    this->parent=parent;
    _audioSeg=0;
    // Fill in wavHeader and access
    ADM_assert(track);
    wavHeader=track->wavheader;
    durationInUs = track->stream->getDurationInUs();    
}
/**
    \fn isCBR
*/
bool ADM_edAudioTrackFromVideo::isCBR()
{
    _SEGMENT *seg=parent->_segments.getSegment(_audioSeg);
    if(!seg) return false;
    ADM_audioStreamTrack *trk=getTrackAtVideoNumber(seg->_reference);
    if(!trk) return false;
    return trk->stream->isCBR();
}
/**
    \fn dtor
*/
ADM_edAudioTrackFromVideo::~ADM_edAudioTrackFromVideo()
{
    ADM_info("Destroying edAudio from video track %d at %"PRIx32"\n",myTrackNumber,this);
    // No need to destroy, we are just a wrapper
}
/**
    \fn getDurationInUs
*/
uint64_t ADM_edAudioTrackFromVideo::getDurationInUs()
{
    // get duration...
    return durationInUs;
}
// EOF

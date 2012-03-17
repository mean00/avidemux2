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
    \fn getDefaultAudioTrack
    \brief return the main audio track
*/
bool        ADM_Composer::getDefaultAudioTrack(ADM_audioStream **stream)
{
    *stream=NULL;
    if(!activeAudioTracks.size()) return true;
    *stream=activeAudioTracks.at(0);
    return true;
}
/**
    \fn getDefaultAudioTrack
    \brief same as above but returns as ADM_edAudioTrack
*/
ADM_edAudioTrack *ADM_Composer::getDefaultEdAudioTrack(void)
{

    if(!activeAudioTracks.size()) return NULL;
    return activeAudioTracks.at(0);
}
// EOF

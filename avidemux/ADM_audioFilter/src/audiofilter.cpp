/***************************************************************************
            \file audiofilter.h
            \brief Creates destroy audio filters
              (c) 2006 Mean , fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include <math.h>

#include "audiofilter_bridge.h"
#include "audiofilter_access.h"
#include "audiofilter_internal.h"

VectorOfAudioFilter PlaybackVector;
extern ADM_Composer *video_body;
/**
        \fn createPlaybackFilter
        \brief Create a float output filter for playback
        @param StartTime in us
        @param shift in ms
*/
AUDMAudioFilter *createPlaybackFilter(uint64_t startTime,int32_t shift)
{
AUDMAudioFilter *nw;
    ADM_assert(0==PlaybackVector.size());
    // The First one is always the bridge
    nw=new AUDMAudioFilter_Bridge(video_body,(uint32_t)( startTime/1000),shift);
    PlaybackVector.push_back(nw);

    //
int last=PlaybackVector.size();
    ADM_assert(last);
    return PlaybackVector[last-1];
}
/**
        \fn destroyPlaybackFilter
        \brief Destroy a float output filter for playback
*/

bool            destroyPlaybackFilter(void)
{

    while(PlaybackVector.size())
    {
        delete PlaybackVector[0];
        PlaybackVector.erase(PlaybackVector.begin());

    }
    return true;

}

// EOF
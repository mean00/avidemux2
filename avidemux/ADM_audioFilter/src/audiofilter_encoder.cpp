/***************************************************************************
            \file audiofilter_encoder.cpp
            \brief Generate a access class = to the output of encoder + filterchain
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
#include "audiofilter_conf.h"
#include "audioencoder.h"
#include "audioEncoderApi.h"

VectorOfAudioFilter EncodingVector;
extern ADM_Composer *video_body;

//
 extern bool ADM_buildFilterChain(VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config);
 extern bool ADM_emptyFilterChain(VectorOfAudioFilter *vec);
/**
        \fn createPlaybackFilter
        \brief Create a float output filter for playback
        @param StartTime in us
        @param shift in ms
*/
AUDMAudioFilter *createEncodingFilter(uint64_t startTime,int32_t shift)
{
    //
    ADM_AUDIOFILTER_CONFIG playback;
    playback.startTimeInUs=startTime;
    playback.shiftInMs=shift;
    playback.mixerEnabled=true;
    playback.mixerConf=CHANNEL_STEREO;
    //
    ADM_buildFilterChain(&EncodingVector,&playback);
    //
    int last=EncodingVector.size();
    ADM_assert(last);
    return EncodingVector[last-1];
}
/**
        \fn destroyPlaybackFilter
        \brief Destroy a float output filter for playback
*/

bool            destroyEncodingFilter(void)
{

    ADM_emptyFilterChain(&EncodingVector);
    return true;

}
/**
    \fn createEncodingAccess
*/
ADM_audioStream *createEncodingStream(uint64_t startTime,int32_t shift)
{
    printf("[AccessFilter] Creating access filter\n");
    // 1-Create access filter
    AUDMAudioFilter *filter=createEncodingFilter(startTime,shift);
    if(!filter)
    {
        printf("[Access] Cannot create audio filter\n");
        return NULL;
    }

    // 2- spawn encoder
    ADM_AudioEncoder *encoder=audioEncoderCreate(filter);
    if(!encoder) 
    {
        printf("[Access] Cannot create audio encoder\n");
        return NULL;
    }
    // 3- Create access
    ADMAudioFilter_Access *access=new ADMAudioFilter_Access(filter,encoder,0);
    if(!access)
    {
        printf("[Access] Cannot create access\n");
    }
    // 4- Create Stream // MEMLEAK!!!!
    ADM_audioStream *stream=ADM_audioCreateStream(access->getWavHeader(), access);
    if(!access)
    {
        printf("[Access] Cannot create access\n");
    }
    return (ADM_audioStream *)access;
}
// EOF
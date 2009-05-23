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
#include "audiofilter_conf.h"

VectorOfAudioFilter PlaybackVector;
extern ADM_Composer *video_body;

//
 bool ADM_buildFilterChain(VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config);
 bool ADM_emptyFilterChain(VectorOfAudioFilter *vec);
/**
        \fn createPlaybackFilter
        \brief Create a float output filter for playback
        @param StartTime in us
        @param shift in ms
*/
AUDMAudioFilter *createPlaybackFilter(uint64_t startTime,int32_t shift)
{
    //
    ADM_AUDIOFILTER_CONFIG playback;
    playback.startTimeInUs=startTime;
    playback.shiftInMs=shift;
    playback.mixerEnabled=true;
    playback.mixerConf=CHANNEL_STEREO;
    // If we have no audio, dont even try...
    if(!video_body->getInfo()) return NULL;
    //
    ADM_buildFilterChain(&PlaybackVector,&playback);
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

    ADM_emptyFilterChain(&PlaybackVector);
    return true;

}
/***********************************************************************/
#define ADD_FILTER(x) { vec->push_back(x);last=x;}
/**
    \fn ADM_buildFilterChain
    \brief Create a filterchain
    @param vec : VectorFilter to build filters into
    @param config: Filters configuration
*/
bool ADM_buildFilterChain(VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config)
{
    // make sure the chain is empty...
    AUDMAudioFilter *last=NULL;
    ADM_emptyFilterChain(vec);
    
    // Bridge
    AUDMAudioFilter_Bridge *nw=new AUDMAudioFilter_Bridge(video_body,(uint32_t)( config->startTimeInUs/1000),
                                                                                config->shiftInMs);
    ADD_FILTER(nw);

    // Mixer
    if(config->mixerEnabled)
    {
        AUDMAudioFilterMixer *mixer=new AUDMAudioFilterMixer(last,config->mixerConf);
        ADD_FILTER(mixer);
    }
    // Resample
    if(config->resamplerEnabled && config->resamplerFrequency!=last->getInfo()->frequency)
    {
        AUDMAudioFilterSrc *src=new AUDMAudioFilterSrc(last,config->resamplerFrequency);
        ADD_FILTER(src);
    }
    // Normalize

    return true;
}
/**
    \fn ADM_emptyFilterChain
    \brief Destroy a filter chain
*/
bool ADM_emptyFilterChain(VectorOfAudioFilter *vec)
{
   while(vec->size())
    {
        delete (*vec)[0];
        vec->erase(vec->begin());
    }
    return true;
}
/**
 * 	\fn ADM_audioCompareChannelMapping
 *  \brief return true if the two channel mapping are identical, false else.
 */
bool ADM_audioCompareChannelMapping(WAVHeader *wh1, WAVHeader *wh2,CHANNEL_TYPE *map1,CHANNEL_TYPE *map2)
{
	if(wh1->channels != wh2->channels) return false; // cannot be identical..
		
			for (int j = 0; j < wh1->channels; j++)
			{
				if (map1[j] != map2[j]) 
				{
					return false;
					
				}
			}
	return true;
}

// EOF
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


#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include <math.h>

#include "audiofilter_bridge.h"
#include "audiofilter_access.h"
#include "audiofilter_internal.h"
#include "audiofilter_conf.h"
//#include "audiofilter_film2pal.h"
#include "audiofilter_stretch.h"
#include "prefs.h"
VectorOfAudioFilter PlaybackVector;
extern ADM_Composer *video_body;

//
 bool ADM_buildFilterChain(ADM_edAudioTrack *source,VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config);
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
    ADM_edAudioTrack *trk=video_body->getDefaultEdAudioTrack();
    if(!trk) return NULL;
    trk->goToTime(startTime);
    //
    uint32_t downmix;
    ADM_AUDIOFILTER_CONFIG playback;
    playback.playBack = true;
    playback.startTimeInUs=startTime;
    playback.shiftInMs=shift;
    if(shift)
        playback.shiftEnabled=true;
    else
        playback.shiftEnabled=false;
    playback.mixerEnabled=true;
    if(prefs->get(DEFAULT_DOWNMIXING,&downmix)!=RC_OK)
    {
      downmix=0;
    }
      switch (downmix) {
            case 0:
                    playback.mixerEnabled=false;
                    break;
            case 1:
                    
                    playback.mixerConf=CHANNEL_STEREO;
                    break;  
            case 2:
                    
                    playback.mixerConf=CHANNEL_STEREO_HEADPHONES;
                    break;  
            case 3:
                    
                    playback.mixerConf=CHANNEL_DOLBY_PROLOGIC;
                    break;
            case 4:
                    
                    playback.mixerConf=CHANNEL_DOLBY_PROLOGIC2;
                    break;
            case 5:
                    
                    playback.mixerConf=CHANNEL_SURROUND_HEADPHONES;
                    break;
            default:
                    ADM_assert(0);break;
      }


    // Fetch mixer from prefs...

    // If we have no audio, dont even try...
    ADM_audioStream *s=NULL;
    if(!video_body->getDefaultAudioTrack(&s)) return NULL;
    if(!s) return NULL;
    //
    ADM_buildFilterChain(trk,&PlaybackVector,&playback);
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
bool ADM_buildFilterChain(ADM_edAudioTrack *source,VectorOfAudioFilter *vec,ADM_AUDIOFILTER_CONFIG *config)
{
    ADM_assert(source);
    // make sure the chain is empty...
    AUDMAudioFilter *last=NULL;
    ADM_emptyFilterChain(vec);
    // Bridge
    int32_t actualShift=config->shiftInMs;
    if(!config->shiftEnabled)
        actualShift=0;
    AUDMAudioFilter_Bridge *nw=new AUDMAudioFilter_Bridge(source,(uint32_t)( config->startTimeInUs/1000),actualShift);
    ADD_FILTER(nw);

    if (!config->playBack)
    {
        AUDMAudioFilterFade *fade=new AUDMAudioFilterFade(last,&config->fadeConf);
        ADD_FILTER(fade);
    }

    // Equalizer
    if(config->eqConf.enable)
    {
        AUDMAudioFilterEq *eq=new AUDMAudioFilterEq(last,&config->eqConf);
        ADD_FILTER(eq);
    }
    
    // Channel manipulations
    AUDMAudioFilterChannels * channelManipulation = new AUDMAudioFilterChannels(last, &config->chansConf);
    ADD_FILTER(channelManipulation);
    
    // Mixer
    if(config->mixerEnabled)
    {
        AUDMAudioFilterMixer *mixer=new AUDMAudioFilterMixer(last,config->mixerConf);
        ADD_FILTER(mixer);
    }
    if (config->drcEnabled)
    {
            AUDMAudioFilterLimiter *pdrc = NULL;
            pdrc = new AUDMAudioFilterLimiter(last,&config->drcConf);
            ADD_FILTER(pdrc);
     }
    // Pal 2 film & friends
    switch(config->filmConv)
    {
        case FILMCONV_NONE:
            break;
        case FILMCONV_FILM2PAL:
            {
            AUDMAudioFilterFilm2PalV2 *f2p=new AUDMAudioFilterFilm2PalV2(last);
            ADD_FILTER(f2p);
            }
            break;
        case FILMCONV_PAL2FILM:
            {
            AUDMAudioFilterPal2FilmV2 *f2p=new AUDMAudioFilterPal2FilmV2(last);
            ADD_FILTER(f2p);
            }
            break;
        case FILMCONV_CUSTOM:
            {
            AUDMAudioFilterStretch * stretch = new AUDMAudioFilterStretch(last,config->filmConvTempo, config->filmConvPitch);
            ADD_FILTER(stretch);
            }
            break;
        default:
            ADM_assert(0);
            break;

    }
    // Resample
    if(config->resamplerEnabled && config->resamplerFrequency!=last->getInfo()->frequency)
    {
        AUDMAudioFilterSrc *src=new AUDMAudioFilterSrc(last,config->resamplerFrequency);
        ADD_FILTER(src);
    }
    // Normalize
    if(config->gainParam.mode!=ADM_NO_GAIN)
    {
        AUDMAudioFilterNormalize *norm=new AUDMAudioFilterNormalize(last,&(config->gainParam));
        ADD_FILTER(norm);
    }
    return true;
}
/**
    \fn ADM_emptyFilterChain
    \brief Destroy a filter chain
*/
bool ADM_emptyFilterChain(VectorOfAudioFilter *vec)
{
    int nb=vec->size();
    for(int i=0;i<nb;i++) 
    {
        delete (*vec)[i];
        (*vec)[i]=NULL;
    }
    vec->clear();
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

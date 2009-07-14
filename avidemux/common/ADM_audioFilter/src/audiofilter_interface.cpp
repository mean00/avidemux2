/***************************************************************************
            \file audiofilter_interface.cpp
            \brief Offer simple C api to enable/disable filter
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
#include "audiofilter_conf.h"

// This structure defines the current filter chain used for **ENCODING**
// Warning, we have another one for playback

ADM_AUDIOFILTER_CONFIG audioEncodingConfig;
/**
    \fn audioFilterReset
    \brief Put back default value on audio filter chain

*/
bool audioFilterReset(void)
{
       return audioEncodingConfig.reset();
}

/**
    \fn audioFilterSetResample
    \brief
*/
bool    audioFilterSetResample(uint32_t newfq)  // Set 0 to disable frequency
{
    if(!newfq) audioEncodingConfig.resamplerEnabled=false;
        else        
            {
                    audioEncodingConfig.resamplerEnabled=true;
                    audioEncodingConfig.resamplerFrequency=newfq;
            }
    return true;
}
/**
    \fn audioFilterGetResample
    \brief
*/

uint32_t        audioFilterGetResample(void)  // Set 0 to disable frequency
{
    if(audioEncodingConfig.resamplerEnabled==false) return 0;
    return audioEncodingConfig.resamplerFrequency;
}

/**
    \fn audioFilterSetFrameRate
    \brief
*/

bool    audioFilterSetFrameRate(FILMCONV conf)
{
    audioEncodingConfig.film2pal=conf;
    return true;
}

/**
    \fn audioFilterGetFrameRate
    \brief
*/

FILMCONV        audioFilterGetFrameRate(void)
{
    return audioEncodingConfig.film2pal;
}

/**
    \fn audioFilterSetMixer
    \brief
*/

bool    audioFilterSetMixer(CHANNEL_CONF conf) // Invalid to disable
{
    if(conf==CHANNEL_INVALID)
    {
        audioEncodingConfig.mixerEnabled=false;
    }else   
    {
        audioEncodingConfig.mixerEnabled=true;
        audioEncodingConfig.mixerConf=conf;
    }
    return true;
}

/**
    \fn audioFilterGetMixer
    \brief
*/

CHANNEL_CONF    audioFilterGetMixer(void) // Invalid to disable
{
    if( audioEncodingConfig.mixerEnabled==false) return CHANNEL_INVALID;
    return audioEncodingConfig.mixerConf;
}

// EOF



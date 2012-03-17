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


extern int DIA_getAudioFilter(ADM_AUDIOFILTER_CONFIG *config);

/**
    \fn audioFilterconfigure
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterConfigure(void)
{
    return DIA_getAudioFilter(this);
}

/**
    \fn audioFilterSetResample
    \brief
*/
bool    ADM_AUDIOFILTER_CONFIG::audioFilterSetResample(uint32_t newfq)  // Set 0 to disable frequency
{
    if(!newfq) resamplerEnabled=false;
        else        
            {
                    resamplerEnabled=true;
                    resamplerFrequency=newfq;
            }
    return true;
}
/**
    \fn audioFilterGetResample
    \brief
*/

uint32_t        ADM_AUDIOFILTER_CONFIG::audioFilterGetResample(void)  // Set 0 to disable frequency
{
    if(resamplerEnabled==false) return 0;
    return resamplerFrequency;
}

/**
    \fn audioFilterSetFrameRate
    \brief
*/

bool    ADM_AUDIOFILTER_CONFIG::audioFilterSetFrameRate(FILMCONV conf)
{
    film2pal=conf;
    return true;
}

/**
    \fn audioFilterGetFrameRate
    \brief
*/

FILMCONV        ADM_AUDIOFILTER_CONFIG::audioFilterGetFrameRate(void)
{
    return film2pal;
}
/**
    \fn audioFilterSetNormalize
    \brief 
*/
bool            ADM_AUDIOFILTER_CONFIG::audioFilterSetNormalize( ADM_GAINMode mode,uint32_t gain)
{
    if(mode>=ADM_GAIN_MAX)
    {
        ADM_error("incorrect mode value for normalize");
        return false;
    }
    if(!gain)
    {
        ADM_error("gain cannot be null");
        return false;
    }

    gainParam.mode=mode;
    gainParam.gain10=gain;
    return true;
}
/**
    \fn audioFilterSetNormalize
    \brief 
*/
bool            ADM_AUDIOFILTER_CONFIG::audioFilterGetNormalize( ADM_GAINMode *mode,uint32_t *gain)
{

    *mode=gainParam.mode;
    *gain=gainParam.gain10;
    return true;
}

/**
    \fn audioFilterSetMixer
    \brief
*/

bool    ADM_AUDIOFILTER_CONFIG::audioFilterSetMixer(CHANNEL_CONF conf) // Invalid to disable
{
    if(conf==CHANNEL_INVALID)
    {
        mixerEnabled=false;
    }else   
    {
        mixerEnabled=true;
        mixerConf=conf;
    }
    return true;
}

/**
    \fn audioFilterGetMixer
    \brief
*/

CHANNEL_CONF    ADM_AUDIOFILTER_CONFIG::audioFilterGetMixer(void) // Invalid to disable
{
    if( mixerEnabled==false) return CHANNEL_INVALID;
    return mixerConf;
}

// EOF



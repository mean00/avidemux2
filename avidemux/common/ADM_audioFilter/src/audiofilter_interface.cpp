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
    \fn audioFilterCopyConfig
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterCopyConfig(ADM_AUDIOFILTER_CONFIG * other)
{
    startTimeInUs=other->startTimeInUs;
    shiftInMs=other->shiftInMs;
    mixerEnabled=other->mixerEnabled;
    mixerConf=other->mixerConf;
    resamplerEnabled=other->resamplerEnabled;
    resamplerFrequency=other->resamplerFrequency;
    film2pal=other->film2pal;
    gainParam=other->gainParam;
    drcEnabled=other->drcEnabled;
    drcConf=other->drcConf;
    shiftEnabled=other->shiftEnabled;
    shiftInMs=other->shiftInMs;    
    return true;
}

/**
    \fn audioFilterSetDrcConfig
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterSetDrcConfig(bool active, int normalize, float nFloor, float attTime, float decTime, float ratio, float thresDB)
{
    drcEnabled = active;
    drcConf.mUseGain = normalize;
    drcConf.mFloor = nFloor;
    drcConf.mAttackTime = attTime;
    drcConf.mDecayTime = decTime;
    drcConf.mRatio = ratio;
    drcConf.mThresholdDB = thresDB;
    return true;
}

/**
    \fn audioFilterGetDrcConfig
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterGetDrcConfig(bool * active, int * normalize, float * nFloor, float * attTime, float * decTime, float * ratio, float * thresDB)
{
    *active = drcEnabled;
    *normalize = drcConf.mUseGain;
    *nFloor = drcConf.mFloor;
    *attTime = drcConf.mAttackTime;
    *decTime = drcConf.mDecayTime;
    *ratio = drcConf.mRatio;
    *thresDB = drcConf.mThresholdDB;
    return true;
}

/**
    \fn audioFilterSetChannelGains
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterSetChannelGains(float fL, float fR, float fC, float sL, float sR, float rL, float rR, float rC, float LFE)
{
    chansConf.chGainDB[ADM_CH_FRONT_LEFT] = fL;
    chansConf.chGainDB[ADM_CH_FRONT_RIGHT] = fR;
    chansConf.chGainDB[ADM_CH_FRONT_CENTER] = fC;
    chansConf.chGainDB[ADM_CH_SIDE_LEFT] = sL;
    chansConf.chGainDB[ADM_CH_SIDE_RIGHT] = sR;
    chansConf.chGainDB[ADM_CH_REAR_LEFT] = rL;
    chansConf.chGainDB[ADM_CH_REAR_RIGHT] = rR;
    chansConf.chGainDB[ADM_CH_REAR_CENTER] = rC;
    chansConf.chGainDB[ADM_CH_LFE] = LFE;
    return true;
}

/**
    \fn audioFilterGetChannelGains
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterGetChannelGains(float * fL, float * fR, float * fC, float * sL, float * sR, float * rL, float * rR, float * rC, float * LFE)
{
    *fL = chansConf.chGainDB[ADM_CH_FRONT_LEFT];
    *fR = chansConf.chGainDB[ADM_CH_FRONT_RIGHT];
    *fC = chansConf.chGainDB[ADM_CH_FRONT_CENTER];
    *sL = chansConf.chGainDB[ADM_CH_SIDE_LEFT];
    *sR = chansConf.chGainDB[ADM_CH_SIDE_RIGHT];
    *rL = chansConf.chGainDB[ADM_CH_REAR_LEFT];
    *rR = chansConf.chGainDB[ADM_CH_REAR_RIGHT];
    *rC = chansConf.chGainDB[ADM_CH_REAR_CENTER];
    *LFE = chansConf.chGainDB[ADM_CH_LFE];
    return true;
}

/**
    \fn audioFilterSetChannelDelays
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterSetChannelDelays(int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE)
{
    chansConf.chDelayMS[ADM_CH_FRONT_LEFT] = fL;
    chansConf.chDelayMS[ADM_CH_FRONT_RIGHT] = fR;
    chansConf.chDelayMS[ADM_CH_FRONT_CENTER] = fC;
    chansConf.chDelayMS[ADM_CH_SIDE_LEFT] = sL;
    chansConf.chDelayMS[ADM_CH_SIDE_RIGHT] = sR;
    chansConf.chDelayMS[ADM_CH_REAR_LEFT] = rL;
    chansConf.chDelayMS[ADM_CH_REAR_RIGHT] = rR;
    chansConf.chDelayMS[ADM_CH_REAR_CENTER] = rC;
    chansConf.chDelayMS[ADM_CH_LFE] = LFE;
    return true;
}

/**
    \fn audioFilterGetChannelDelays
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterGetChannelDelays(int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE)
{
    *fL = chansConf.chDelayMS[ADM_CH_FRONT_LEFT];
    *fR = chansConf.chDelayMS[ADM_CH_FRONT_RIGHT];
    *fC = chansConf.chDelayMS[ADM_CH_FRONT_CENTER];
    *sL = chansConf.chDelayMS[ADM_CH_SIDE_LEFT];
    *sR = chansConf.chDelayMS[ADM_CH_SIDE_RIGHT];
    *rL = chansConf.chDelayMS[ADM_CH_REAR_LEFT];
    *rR = chansConf.chDelayMS[ADM_CH_REAR_RIGHT];
    *rC = chansConf.chDelayMS[ADM_CH_REAR_CENTER];
    *LFE = chansConf.chDelayMS[ADM_CH_LFE];
    return true;
}

/**
    \fn audioFilterSetChannelRemap
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterSetChannelRemap(bool active, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE)
{
    chansConf.enableRemap = active;
    chansConf.remap[0] = fL;
    chansConf.remap[1] = fR;
    chansConf.remap[2] = fC;
    chansConf.remap[3] = sL;
    chansConf.remap[4] = sR;
    chansConf.remap[5] = rL;
    chansConf.remap[6] = rR;
    chansConf.remap[7] = rC;
    chansConf.remap[8] = LFE;
    return true;
}

/**
    \fn audioFilterGetChannelRemap
    \brief
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterGetChannelRemap(bool * active, int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE)
{
    *active = chansConf.enableRemap;
    *fL = chansConf.remap[0];
    *fR = chansConf.remap[1];
    *fC = chansConf.remap[2];
    *sL = chansConf.remap[3];
    *sR = chansConf.remap[4];
    *rL = chansConf.remap[5];
    *rR = chansConf.remap[6];
    *rC = chansConf.remap[7];
    *LFE = chansConf.remap[8];
    return true;
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
bool ADM_AUDIOFILTER_CONFIG::audioFilterSetNormalize( ADM_GAINMode mode, int32_t gain, int32_t limit )
{
    if(mode>=ADM_GAIN_MAX)
    {
        ADM_error("incorrect mode value for normalize");
        return false;
    }

    gainParam.mode=mode;
    gainParam.gain10=gain;
    gainParam.maxlevel10=limit;
    return true;
}
/**
    \fn audioFilterSetNormalize
    \brief 
*/
bool ADM_AUDIOFILTER_CONFIG::audioFilterGetNormalize( ADM_GAINMode *mode, int32_t *gain, int32_t *limit )
{
    *mode=gainParam.mode;
    *gain=gainParam.gain10;
    *limit=gainParam.maxlevel10;
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
/**

*/
bool            ADM_AUDIOFILTER_CONFIG::audioFilterSetShift( bool enabled ,int32_t shift)
{
	shiftEnabled=enabled;
	shiftInMs=shift;
	return true;	
}
/**
*/
bool            ADM_AUDIOFILTER_CONFIG::audioFilterGetShift( bool *enabled,int32_t *shift)
{
	*enabled=shiftEnabled;
	*shift=shiftInMs;
	return true;
}
// EOF



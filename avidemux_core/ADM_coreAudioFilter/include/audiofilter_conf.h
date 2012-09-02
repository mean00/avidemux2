/***************************************************************************
            \file audiofilter_conf.h
            \brief Manage configuration
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

#ifndef  ADM_audiofilter_conf_h
#define  ADM_audiofilter_conf_h

#include "ADM_audioFilter.h"
#include "audiofilter.h"
#include "audiofilter_mixer.h"
#include "audiofilter_SRC.h"
#include "audiofilter_normalize.h"
#include "audiofilter_limiter.h"

/**
    \class ADM_AUDIOFILTER_CONFIG
*/
class ADM_AUDIOFILTER_CONFIG
{
public    :

    ADM_AUDIOFILTER_CONFIG(void)
                {
                    reset();
                }
    bool        reset()
                {
                        startTimeInUs=0;
                        shiftInMs=0;
                        mixerEnabled=false;
                        mixerConf=CHANNEL_STEREO;
                        resamplerEnabled=false;
                        resamplerFrequency=44100;
                        film2pal    =FILMCONV_NONE;
                        gainParam.mode=ADM_NO_GAIN;
                        gainParam.gain10=10;
                        drcEnabled=false;
                        drcConf=drcConfDefault;
			shiftEnabled=false;
    			shiftInMs=0;
                        return true;
                }

    uint64_t     startTimeInUs;
    //
    bool         shiftEnabled;
    int32_t      shiftInMs;
    // Mixer
    uint32_t     mixerEnabled;
    CHANNEL_CONF mixerConf;
    // Resampler
    uint32_t     resamplerEnabled;
    uint32_t     resamplerFrequency;
    // film2pal & pal2film
    FILMCONV     film2pal;
    // Gain filter
    GAINparam    gainParam;
    // DRC / limiter
    bool         drcEnabled;
    DRCparam      drcConf;

public: // accessor
    bool            audioFilterGetDrcMode(void) {return drcEnabled;};
    bool            audioFilterSetDrcMode(bool m) { drcEnabled=m;return true;};
    bool            audioFilterConfigure(void);
    bool            audioFilterSetResample(uint32_t newfq);  // Set 0 to disable frequency
    uint32_t        audioFilterGetResample(void);  // Set 0 to disable frequency
    bool            audioFilterSetFrameRate(FILMCONV conf);
    FILMCONV        audioFilterGetFrameRate(void);
    bool            audioFilterSetShift( bool enabled ,int32_t shift);
    bool            audioFilterGetShift( bool *enabled,int32_t *shift);
    bool            audioFilterSetNormalize( ADM_GAINMode mode,uint32_t gain);
    bool            audioFilterGetNormalize( ADM_GAINMode *mode,uint32_t *gain);

    bool            audioFilterSetMixer(CHANNEL_CONF conf); // Invalid to disable
    CHANNEL_CONF    audioFilterGetMixer(void); // Invalid to disable
    const char      *audioMixerAsString(void)
                    {
                        
                        if(!mixerEnabled) return "CHANNEL_INVALID";
                        return AudioMixerIdToString(mixerConf);
                    }

};

#endif

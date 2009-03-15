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

/**
    \class ADM_AUDIOFILTER_CONFIG
*/
class ADM_AUDIOFILTER_CONFIG
{
public    :

    ADM_AUDIOFILTER_CONFIG(void)
        {
                startTimeInUs=0;
                shiftInMs=0;
                mixerEnabled=false;
                mixerConf=CHANNEL_STEREO;

        }

    uint64_t     startTimeInUs;
    int32_t      shiftInMs;
    
    bool         mixerEnabled;
    CHANNEL_CONF mixerConf;



};

#endif
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

#ifndef  ADM_audiofilter_h
#define  ADM_audiofilter_h
#include "ADM_audioFilter.h"
/**
        \fn createPlaybackFilter
        \brief Create a float output filter for playback
        @param startTime: Starting time of the filter in us
        @param shift : Time Shift in ms
*/
AUDMAudioFilter *createPlaybackFilter(uint64_t startTime,int32_t shift);
/**
        \fn destroyPlaybaclFilter
        \brief Destroy a float output filter for playback
*/

bool            destroyPlaybackFilter(void);

#endif
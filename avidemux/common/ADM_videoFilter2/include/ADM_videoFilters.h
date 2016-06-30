/**
        \file  ADM_videoFilters.h
        \brief Handle current filter list


*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_VIDEO_FILTERS_H
#define ADM_VIDEO_FILTERS_H
#include "BVector.h"
#include "ADM_coreVideoFilter.h"

// Get list
uint32_t                ADM_vf_getSize(void);
ADM_coreVideoFilter     *ADM_vf_getInstance(int index);
uint32_t                ADM_vf_getTag(int index);
bool                    ADM_vf_getConfigurationFromIndex(int index,CONFcouple **c);

bool                    ADM_vf_configureFilterAtIndex(int index);
bool                    ADM_vf_moveFilterDown(int index);
bool                    ADM_vf_moveFilterUp(int index);
bool                    ADM_vf_partialize(int index);
#endif

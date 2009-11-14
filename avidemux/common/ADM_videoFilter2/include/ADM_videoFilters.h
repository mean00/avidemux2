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

class ADM_coreVideoFilter;
/**
    \struct ADM_VideoFilterElment
*/
typedef struct
{
    uint32_t            tag; // Temporary filter tag
    ADM_coreVideoFilter *instance;
}ADM_VideoFilterElement;

extern std::vector<ADM_VideoFilterElement> ADM_VideoFilters;

// Get list
uint32_t                ADM_vf_getSize(void);
ADM_coreVideoFilter     *ADM_vf_getInstance(int index);
uint32_t                ADM_vf_getTag(int index);

bool                    ADM_vf_addFilterFromTag(uint32_t tag);
bool                    ADM_vf_removeFilterAtIndex(int index);
#endif
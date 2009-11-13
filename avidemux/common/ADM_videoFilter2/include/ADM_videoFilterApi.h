/**
        \file ADM_videoFilterApi.h
        \brief Public functions
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_VIDEO_FILTER_API_H
#define ADM_VIDEO_FILTER_API_H

uint8_t ADM_vf_loadPlugins(const char *path);

uint32_t ADM_vf_getNbFilters(void);
uint32_t ADM_vf_getNbFiltersInCategory(VF_CATEGORY cat);

bool     ADM_vf_getFilterInfo(VF_CATEGORY cat,int filter, const char **name,const char **desc, uint32_t *major,uint32_t *minor,uint32_t *patch);


#endif //ADM_VIDEO_FILTER_API_H

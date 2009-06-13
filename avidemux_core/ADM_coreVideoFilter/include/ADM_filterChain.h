/***************************************************************************
                          \fn ADM_filterChain.h
                          \brief Base class for Video Filters
                           (c) Mean 2009



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_filterChain_H
#define ADM_filterChain_H
#include "ADM_coreVideoFilter.h"
#include <vector>
typedef std::vector <ADM_coreVideoFilter *>ADM_videoFilterChain;


ADM_videoFilterChain *createVideoFilterChain(uint64_t startAt,uint64_t endAt);
bool                 destroyVideoFilterChain(ADM_videoFilterChain *chain);


#endif
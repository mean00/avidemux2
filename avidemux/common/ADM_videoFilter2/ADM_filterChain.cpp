/***************************************************************************
                          \fn ADM_filterChain.cpp
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
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_filterChain.h"
#include "ADM_videoFilterBridge.h"
/**
    \fn createVideoFilterChain
    \brief Create a filter chain
*/
ADM_videoFilterChain *createVideoFilterChain(uint64_t startAt,uint64_t endAt)
{
    ADM_videoFilterChain *chain=new ADM_videoFilterChain;
    // 1- Add bridge always # 1
    ADM_videoFilterBridge *bridge=new ADM_videoFilterBridge(startAt,endAt);
    chain->push_back(bridge);
    return chain;
}
/**
        \fn destroyVideoFilterChain
        \brief Destroy a filter chain
*/
bool                 destroyVideoFilterChain(ADM_videoFilterChain *chain)
{
    ADM_assert(chain->size());
    for(int i=0;i<chain->size();i++)
    {
        ADM_coreVideoFilter *filter=(*chain)[i];
        delete filter;
    }
    chain->erase(chain->begin(),chain->end()-1);
    return true;
}

// EOF

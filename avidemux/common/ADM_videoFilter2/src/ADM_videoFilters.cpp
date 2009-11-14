/**
        \file  ADM_videoFilters.cpp
        \brief Handle current filter list


*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterBridge.h"
static ADM_coreVideoFilter *bridge=NULL;

std::vector<ADM_VideoFilterElement> ADM_VideoFilters;

/**
    \fn ADM_vf_getSize
    \brief Returns # of active filters
*/
uint32_t                ADM_vf_getSize(void)
{
    return (uint32_t)ADM_VideoFilters.size();
}
/**
    \fn ADM_vf_getInstance
    \brief Return filter instance  of rank index
*/
ADM_coreVideoFilter     *ADM_vf_getInstance(int index)
{
    ADM_assert(index<ADM_vf_getSize());
    return ADM_VideoFilters[index].instance;
}
/**
    \fn ADM_vf_getTag
    \brief Return tag of index active filter
*/
uint32_t                ADM_vf_getTag(int index)
{
    ADM_assert(index<ADM_vf_getSize());
    return ADM_VideoFilters[index].tag;
}
/**
    \fn getLastVideoFilter
*/
static ADM_coreVideoFilter *getLastVideoFilter(void)
{
ADM_coreVideoFilter *last=NULL;
   if(!ADM_vf_getSize())
    {
        if(!bridge)
            bridge=new ADM_videoFilterBridge(0,2*3600*1000*1000LL);
        last=bridge;
    }
    else    
        last=ADM_VideoFilters[ADM_vf_getSize()-1].instance;
    return last;
}
/**
        \fn ADM_vf_addFilterFromTag
        \brief Add a new video filter (identified by tag) at the end of the activate filter list
*/
bool                    ADM_vf_addFilterFromTag(uint32_t tag)
{
    ADM_info("Creating video filter using tag %"LU" \n",tag);
    // Fetch the descriptor...
    
    ADM_coreVideoFilter *last=getLastVideoFilter();
 
    ADM_coreVideoFilter *nw=ADM_vf_createFromTag(tag,last,NULL);
    if(nw->configure()==false)
    {
        delete nw;
        return false;
    }
    ADM_VideoFilterElement e;
    e.tag=tag;
    e.instance=nw;
    ADM_VideoFilters.push_back(e);
    return true;
}
/**
    \fn ADM_vf_removeFilterAtIndex
    
*/
bool ADM_vf_removeFilterAtIndex(int index)
{
    ADM_info("Deleting video filter at index %d\n",index);
    //
    ADM_assert(index<ADM_vf_getSize());
    if(index==ADM_vf_getSize()-1)
    {
        // last filter, destroy..
        ADM_VideoFilterElement *e=&(ADM_VideoFilters[index]);
        delete e->instance;
        ADM_VideoFilters.clear();
        ADM_info("Deleting last filter\n");
        return true;
    }
    // Else we have a==b==c => a==c==d ...
    // So we need to create an new string from c to the end...
    return true;
}
// EOF
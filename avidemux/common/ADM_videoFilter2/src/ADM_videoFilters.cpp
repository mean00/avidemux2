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
#include "ADM_filterChain.h"
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
    \fn ADM_vf_recreateChain
    \brief Rebuild the whole filterchain
*/
static bool ADM_vf_recreateChain(void)
{
    ADM_assert(bridge);
    ADM_coreVideoFilter *f=bridge;
    
    std::vector<ADM_coreVideoFilter *> bin;
    for(int i=0;i<ADM_VideoFilters.size();i++)
    {
            // Get configuration
            CONFcouple *c;
            ADM_coreVideoFilter *old=ADM_VideoFilters[i].instance;
            uint32_t tag=ADM_VideoFilters[i].tag;
            old->getCoupledConf(&c);
            ADM_coreVideoFilter *nw=ADM_vf_createFromTag(tag,f,c);
            ADM_VideoFilters[i].instance=nw;
            bin.push_back(old);
            if(c) delete c;
            f=nw;
    }
    // Now delete bin
    for(int i=0;i<bin.size();i++)
    {
        delete bin[i];
    }
    bin.clear();
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
    // last filter, destroy..
    ADM_VideoFilterElement *e=&(ADM_VideoFilters[index]);
    delete e->instance;
    ADM_VideoFilters.erase(ADM_VideoFilters.begin() + index);        
    return ADM_vf_recreateChain();
}
/**
    \fn ADM_vf_configureFilterAtIndex
*/
bool ADM_vf_configureFilterAtIndex(int index)
{
    ADM_info("Configuring filter at index %d\n",index);
    //
    ADM_assert(index<ADM_vf_getSize());
    ADM_VideoFilterElement *e=&(ADM_VideoFilters[index]);
    if(e->instance->configure())
    {
        if(e->instance->configure())
        {
            return ADM_vf_recreateChain();
        }
    }
    return true;
}

/**
    \fn ADM_vf_moveFilterUp
*/
bool ADM_vf_moveFilterUp(int index)
{
    ADM_info("Moving up filter at index %d\n",index);
    //
    ADM_assert(index);
    uint32_t top=index-1;
    ADM_VideoFilterElement scratch=ADM_VideoFilters[top];

    ADM_VideoFilters[top]=ADM_VideoFilters[top+1];
    ADM_VideoFilters[top+1]=scratch;
    return ADM_vf_recreateChain();
}

/**
    \fn ADM_vf_moveFilterDown
*/
bool ADM_vf_moveFilterDown(int index)
{
    ADM_info("Moving down filter at index %d\n",index);
    //
    ADM_assert(index+1<ADM_VideoFilters.size());
    uint32_t top=index;
    ADM_VideoFilterElement scratch=ADM_VideoFilters[top];

    ADM_VideoFilters[top]=ADM_VideoFilters[top+1];
    ADM_VideoFilters[top+1]=scratch;
    return ADM_vf_recreateChain();
}



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
    ADM_coreVideoFilter *f=bridge;
    // Now create a clone of the videoFilterChain we have here
    int nb=ADM_VideoFilters.size();
    for(int i=0;i<nb;i++)
    {
            // Get configuration
            CONFcouple *c;
            ADM_coreVideoFilter *old=ADM_VideoFilters[i].instance;
            uint32_t tag=ADM_VideoFilters[i].tag;
            old->getCoupledConf(&c);

            ADM_coreVideoFilter *nw=ADM_vf_createFromTag(tag,f,c);
            if(c) delete c;
            f=nw;
            chain->push_back(nw);
    }
    return chain;
}
/**
        \fn destroyVideoFilterChain
        \brief Destroy a filter chain
*/
bool                 destroyVideoFilterChain(ADM_videoFilterChain *chain)
{
    ADM_assert(chain->size());
    int nb=chain->size();
    for(int i=0;i<nb;i++)
    {
        ADM_coreVideoFilter *filter=(*chain)[i];
        delete filter;
        (*chain)[i]=NULL;
    }
    chain->clear();
    return true;
}
// EOF
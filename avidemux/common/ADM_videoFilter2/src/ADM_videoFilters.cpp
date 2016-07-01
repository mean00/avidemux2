/**
        \file  ADM_videoFilters.cpp
        \brief Handle current filter list
        \author mean fixounet@free.fr 2010

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "BVector.h"
#include "ADM_default.h"
#include "ADM_edit.hxx"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterBridge.h"
#include "ADM_filterChain.h"
#include "ADM_filterThread.h"
#include "ADM_coreVideoFilterFunc.h"

extern ADM_coreVideoFilter *bridge;
extern ADM_Composer *video_body;

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
    \fn ADM_vf_configureFilterAtIndex
*/
bool ADM_vf_configureFilterAtIndex(int index)
{
    ADM_info("Configuring filter at index %d\n",index);
    //
    ADM_assert(index<ADM_vf_getSize());
    ADM_VideoFilterElement *e=&(ADM_VideoFilters[index]);
    ADM_coreVideoFilter *instance=e->instance;
    ADM_assert(instance);

    if(instance->configure())
    {
        return ADM_vf_recreateChain();
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

bool ADM_vf_partialize(int index)
{
    ADM_info("Partializing filter at index %d\n",index);
    //
    ADM_assert(index<ADM_VideoFilters.size());
    uint32_t top=index;
    //--
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
    ADM_videoFilterBridge *bridge=new ADM_videoFilterBridge(video_body, startAt,endAt);
    chain->push_back(bridge);
    ADM_coreVideoFilter *f=bridge;
    // Now create a clone of the videoFilterChain we have here
    int nb=ADM_VideoFilters.size();
    bool openGl=false;
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

            VF_CATEGORY type=ADM_vf_getFilterCategoryFromTag(tag);
            if(type== VF_OPENGL) openGl=true;

    }
    // Last create the thread
#if 1
    // Make sure there is no openGl filter in the queue, it is not thread safe...

    if(openGl==true)
    {
        ADM_warning("The filter chain contains an openGl filter, disabling threads \n");
    }else
    {
        int m=chain->size();
        ADM_coreVideoFilter *last=(*chain)[m-1];
        ADM_videoFilterQueue *thread=new ADM_videoFilterQueue(last);
        chain->push_back(thread);
    }
#endif
    return chain;
}
/**
    \fn createEmptyVideoFilterChain
    \brief Create an empty filter chain
*/
ADM_videoFilterChain *createEmptyVideoFilterChain(uint64_t startAt,uint64_t endAt)
{
    ADM_videoFilterChain *chain=new ADM_videoFilterChain;
    // 1- Add bridge always # 1
    ADM_videoFilterBridge *bridge=new ADM_videoFilterBridge(video_body, startAt,endAt);
    chain->push_back(bridge);
    // Last create the thread
#if 1
    int m=chain->size();
    ADM_coreVideoFilter *last=(*chain)[m-1];
    ADM_videoFilterQueue *thread=new ADM_videoFilterQueue(last);
    chain->push_back(thread);
#endif
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
    if(!nb)
    {
        ADM_info("Empty filter chain\n");
        return true;
    }
    nb--;
    for(int i=nb;i>=0;i--) // delete from the end
    {
        ADM_coreVideoFilter *filter=(*chain)[i];
        delete filter;
        (*chain)[i]=NULL;
    }
    delete chain;
    return true;
}
/**
    \fn ADM_vf_getConfigurationFromIndex
*/
bool ADM_vf_getConfigurationFromIndex(int index,CONFcouple **c)
{
        ADM_assert(index<ADM_VideoFilters.size());
            ADM_coreVideoFilter *old=ADM_VideoFilters[index].instance;
            old->getCoupledConf(c);

        return true;
}
// EOF

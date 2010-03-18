/**
        \file  ADM_filterThread.cpp
        \brief Queue buffered filter. A dedicated thread is filling the queue. To be put just before encoder
        \author mean, fixounet@free.fr
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
#include "ADM_filterThread.h"
/**
    \fn     ADM_videoFilterQueue
    \brief
*/
ADM_videoFilterQueue::ADM_videoFilterQueue(ADM_coreVideoFilter *previous,CONFcouple *conf ):
                ADM_coreVideoFilter(previous,conf)
{
    // Allocate buffer
    for(int i=0;i<ADM_THREAD_QUEUE_SIZE;i++)
    {
        ADM_queuePacket item;
        item.data=(uint8_t *)new ADMImage(info.width,info.height);
        freeList.push_back(item);
    }
}
/**
    \fn ~ADM_videoFilterQueue
    \brief
*/
ADM_videoFilterQueue::~ADM_videoFilterQueue()
{
        ADM_info("Destroying video threadQueue\n");
        int i;
        i=freeList.size();
        for(int j=0;j<i;j++)
        {
            ADMImage *image=(ADMImage *)freeList[j].data;
            delete image;
        }
        freeList.clear();
        i=list.size();
        for(int j=0;j<i;j++)
        {
            ADMImage *image=(ADMImage *)list[j].data;
            delete image;
        }
        list.clear();

}
/**
    \fn     goToTime
    \brief
*/
bool         ADM_videoFilterQueue::goToTime(uint64_t usSeek)
{
        ADM_assert(0);
        return false;
}
/**
    \fn     getNextFrame
    \brief
*/
bool         ADM_videoFilterQueue::getNextFrame(uint32_t *frameNumber,ADMImage *image)
{
     if(false==started)
        {
            startThread();      
        }
        while(1)
        {
            mutex->lock();
            if(list.size())
            {
                //
                // Dequeue one item
                ADM_queuePacket *pkt=&(list[0]);
                ADM_assert(pkt->data);
                ADMImage *source=(ADMImage *)pkt->data;
                *frameNumber=pkt->pts;
                image->duplicateFull(source);
                list.erase(list.begin());
                freeList.push_back(*pkt);
                mutex->unlock();
                return true;
            }
            // If no item, thread still alive ?
            if(threadState==RunStateStopped)
            {
                ADM_info("Audio thread stopped, no more data\n");
                mutex->unlock();
                return false;
            }
            mutex->unlock();
            ADM_usleep(10*1000); // wait 10 ms
        }
        return false;
}
/**
    \fn     getInfo
    \brief
*/
FilterInfo   *ADM_videoFilterQueue::getInfo(void)    
{
        return previousFilter->getInfo();
}
/**
    \fn
    \brief
*/
bool         ADM_videoFilterQueue::runAction(void)
{
    while(1)
    {
        if(threadState==RunStateStopOrder)  
        {
            ADM_info("Audio Thread, received stop order\n");
            goto theEnd;
        }
        mutex->lock();
        if(!freeList.size())
        {
            mutex->unlock();
            ADM_usleep(2000);
            continue;
        }
        uint32_t fn=0;
        ADM_queuePacket pkt=(list[0]);
        ADM_assert(pkt.data);
        ADMImage *source=(ADMImage *)pkt.data;
        list.erase(freeList.begin());
        mutex->unlock();

        if(false==previousFilter->getNextFrame(&fn,source))
        {
            pkt.pts=fn;
            ADM_info("Video Thread, no more data\n");
            mutex->lock();
            freeList.push_back(pkt);
            mutex->unlock();
            goto theEnd;
        }
    }
theEnd:
        ADM_info("Exiting video thread loop\n");
}
//EOF

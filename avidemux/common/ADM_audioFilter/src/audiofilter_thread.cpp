/***************************************************************************
            \file audiofilter_thread.cpp
            \brief Wrap an access class into its own thread so that it can be parallelized
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

#include "ADM_cpp.h"
using std::string;
#include "ADM_default.h"
#include "ADM_edit.hxx"
#include "audiofilter_thread.h"
#include <math.h>

#define MAX_CHUNK_IN_QUEUE 10
#define CHUNK_SIZE (20*1024) // should be more than enough

static void *boomerang(void *x)
{
    ADM_audioAccess_thread *a=(ADM_audioAccess_thread *)x;
    a->run();
    return NULL;
}

/**
    \fn ADM_audioAccess_thread
    \brief
*/
ADM_audioAccess_thread::ADM_audioAccess_thread(ADM_audioAccess *son) :ADM_threadQueue()
{
    this->son=son;
    ADM_info("Swallowing audio access into a thread\n");
    for(int i=0;i<MAX_CHUNK_IN_QUEUE;i++)
    {
            ADM_queuePacket pkt;
            pkt.data=new uint8_t[CHUNK_SIZE];
            freeList.push_back(pkt);
    }
    
}
/**
    \fn ~ADM_audioAccess_thread
    \brief
*/

ADM_audioAccess_thread::~ADM_audioAccess_thread()
{
    stopThread();
    // Empty the list...
    int nb=list.size();
    for(int i=0;i<nb;i++)
    {
        ADM_queuePacket *pkt=&(list[i]);
        if(pkt->data) delete [] pkt->data;
        pkt->data=NULL;
    }
    list.clear();

    nb=freeList.size();
    for(int i=0;i<nb;i++)
    {
        ADM_queuePacket *pkt=&(freeList[i]);
        if(pkt->data) delete [] pkt->data;
        pkt->data=NULL;
    }
    freeList.clear();

    // Thread stopped, we can kill the son
    delete son;
}

/**
    \fn setPos
    \brief
*/

bool      ADM_audioAccess_thread::setPos(uint64_t pos) 
{
    return false;
}
/**
    \fn getPos
    \brief
*/

uint64_t  ADM_audioAccess_thread::getPos(void)
{
    return 0;
}
/**
    \fn    getExtraData
    \brief
*/
                                    
bool      ADM_audioAccess_thread::getExtraData(uint32_t *l, uint8_t **d)
{
    return son->getExtraData(l,d);
}
/**
    \fn    getPacket
    \brief
*/

bool    ADM_audioAccess_thread::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
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
            ADM_queuePacket pkt=list[0];
            list.erase(list.begin());
            mutex->unlock();
            ADM_assert(pkt.data);
            ADM_assert(pkt.dataLen<maxSize);
            ADM_assert(pkt.dataLen<CHUNK_SIZE);
            memcpy(buffer,pkt.data,pkt.dataLen);
            *dts=pkt.dts;
            //printf("popping Packet with DTS=%"LLD", size=%d\n",*dts,(int)pkt->dataLen);
            *size=pkt.dataLen;
            mutex->lock();
            freeList.push_back(pkt);
            if(cond->iswaiting())
            {
                cond->wakeup();
            }
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
        ADM_usleep(2*1000); // wait 10 ms
    }
    return false;
}
/**
    \fn runAction
    \brief entry point for thread
*/
bool ADM_audioAccess_thread::runAction(void)
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
            cond->wait();
            continue;
        }
        ADM_queuePacket pkt=(freeList[0]);
        ADM_assert(pkt.data);
        freeList.erase(freeList.begin());
        mutex->unlock();

        if(false==son->getPacket(pkt.data,&(pkt.dataLen),CHUNK_SIZE,&(pkt.dts)))
        {
            ADM_info("Audio Thread, no more data\n");
            goto theEnd;
        }
      
        mutex->lock();
        list.push_back(pkt);
        //printf("Pushing Packet with DTS=%"LLD",size=%d\n",dts,(int)size);
        mutex->unlock();
    }

theEnd:
    return true;
}
/**
    \fn ADM_threadifyAudioAccess
*/
ADM_audioAccess *ADM_threadifyAudioAccess(ADM_audioAccess *son)
{
    return new ADM_audioAccess_thread(son);
}

// EOF

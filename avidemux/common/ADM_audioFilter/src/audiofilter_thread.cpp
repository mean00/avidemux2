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


#include "ADM_default.h"
#include "ADM_editor/ADM_edit.hxx"
#include "audiofilter_thread.h"
#include <math.h>

#define MAX_CHUNK_IN_QUEUE 50

static void *boomerang(void *x)
{
    ADM_audioAccess_thread *a=(ADM_audioAccess_thread *)x;
    a->run();
}

/**
    \fn ADM_audioAccess_thread
    \brief
*/
ADM_audioAccess_thread::ADM_audioAccess_thread(ADM_audioAccess *son)
{
    this->son=son;
    ADM_info("Swallowing audio access into a thread\n");
    mutex=new admMutex("audioAccess");
    cond=new admCond(mutex);
    threadState=RunStateIdle;
    started=false;
    
}
/**
    \fn ~ADM_audioAccess_thread
    \brief
*/

ADM_audioAccess_thread::~ADM_audioAccess_thread()
{
    ADM_info("Killing audio thread and son\n");
    // ask the thread to stop
    
    if(started)
    {
        mutex->lock();
        if(threadState==RunStateRunning)
        {   
            ADM_info("Asking the thread to stop\n");
            threadState=RunStateStopOrder;
            mutex->unlock();
            int count=100;
            while(count)
            {
                if(threadState==RunStateStopped) break;
                ADM_usleep(1000*100); // Slep 100 ms
            }
        }else mutex->unlock();
    }
    // Empty the list...
    int nb=list.size();
    for(int i=0;i<nb;i++)
    {
        ADM_audioPacket *pkt=&(list[i]);
        if(pkt->data) delete [] pkt->data;
        pkt->data=NULL;
    }
    list.clear();
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
        ADM_info("Starting thread...\n");
        if(pthread_create(&myThread,NULL, boomerang, this))
        {
            ADM_error("ERROR CREATING THREAD\n");
            ADM_assert(0);
        }
        while(threadState==RunStateIdle)
        {
            ADM_usleep(10000);
        }
        ADM_info("Thread created and started\n");
        started=true;
    }
    while(1)
    {
        mutex->lock();
        if(list.size())
        {
            // Dequeue one item
            ADM_audioPacket *pkt=&(list[0]);
            ADM_assert(pkt->data);
            ADM_assert(pkt->dataLen<maxSize);
            memcpy(buffer,pkt->data,pkt->dataLen);
            *dts=pkt->dts;
            *size=pkt->dataLen;
            delete [] pkt->data;
            pkt->data=NULL;
            list.erase(list.begin());
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
    \fn run
    \brief entry point for thread
*/
void ADM_audioAccess_thread::run(void)
{
    #define CHUNK_SIZE (48000*sizeof(float)*6)
    threadState=RunStateRunning;
    uint8_t *buffer=new uint8_t[CHUNK_SIZE];
    uint32_t size;
    uint64_t dts;
    while(1)
    {
        if(false==son->getPacket(buffer,&size,CHUNK_SIZE,&dts))
        {
            ADM_info("Audio Thread, no more data\n");
            goto theEnd;
        }
        ADM_audioPacket p;
        p.data=new uint8_t[size];
        memcpy(p.data,buffer,size);
        p.dataLen=size;
        p.dts=dts;
        mutex->lock();
        list.push_back(p);
        mutex->unlock();
        if(threadState==RunStateStopOrder)  
        {
            ADM_info("Audio Thread, received stop order\n");
            goto theEnd;
        }
        while(1)
        {
            int n=list.size();
            if(n<MAX_CHUNK_IN_QUEUE) break;
            ADM_usleep(20*1000); // Fixme: replace by thread signals
        }
            
    }

theEnd:
    delete [] buffer;
    threadState=RunStateStopped;
}
/**
    \fn ADM_threadifyAudioAccess
*/
ADM_audioAccess *ADM_threadifyAudioAccess(ADM_audioAccess *son)
{
    return new ADM_audioAccess_thread(son);
}

// EOF

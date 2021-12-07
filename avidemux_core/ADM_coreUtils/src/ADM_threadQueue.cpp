/***************************************************************************
            \file ADM_threadQueue.cpp
            \brief Create a thread that fills a queue from another part
            \author  (c) 2010 Mean , fixounet@free.fr
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
#include "ADM_threadQueue.h"
#include <math.h>

static void *boomerang(void *x)
{
    ADM_threadQueue *a=(ADM_threadQueue *)x;
    a->run();
    return NULL;
}

/**
    \fn ADM_audioAccess_thread
    \brief
*/
ADM_threadQueue::ADM_threadQueue(void)
{
    mutex=new admMutex("audioAccess");
    producerCond=new admCond(mutex);
    consumerCond=new admCond(mutex);
    threadState=RunStateIdle;
    started=false;
}
/**
    \fn ~ADM_threadQueue
    \brief
*/

ADM_threadQueue::~ADM_threadQueue()
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
            if(producerCond->iswaiting())
            {
                producerCond->wakeup();
            }
            mutex->unlock();
            int count=100;
            while(count)
            {
                if(threadState==RunStateStopped) break;
                ADM_usleep(1000*100); // Slep 100 ms
            }
        }else mutex->unlock();
        void *ret;
        pthread_join(myThread, &ret);
    }
    
    if(producerCond) delete producerCond;
    if(consumerCond) delete consumerCond;
    if(mutex) delete mutex;
    producerCond=NULL;
    consumerCond=NULL;
    mutex=NULL;
}

/**
    \fn run
    \brief entry point for thread
*/
void ADM_threadQueue::run(void)
{

    threadState=RunStateRunning;
    runAction();

    // wait til consumer done
    do {
        bool done=false;
        mutex->lock();
        if (consumerCond->iswaiting())
            consumerCond->wakeup();
        if(!list.size())
            done = true;
        mutex->unlock();
        if (done)
            break;
        ADM_usleep(1*1000); // wait 1 ms        
    } while(1);
    
    threadState=RunStateStopped;

    // wait til consumer exit
    do {
        bool done=false;
        mutex->lock();
        if (consumerCond->iswaiting())
            consumerCond->wakeup();
        else
            done = true;
        mutex->unlock();
        ADM_usleep(1*1000); // wait 1 ms        
        if (done)
            break;
    } while(1);
}
/**
    \fn startThread
*/
bool ADM_threadQueue::startThread(void)
{
    ADM_info("Starting thread...\n");
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if(pthread_create(&myThread,&attr, boomerang, this))
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
    return true;
}

/**
    \fn stopThread
*/
bool ADM_threadQueue::stopThread(void)
{
    ADM_info("Destroying threadQueue\n");
    mutex->lock();

    if(threadState==RunStateRunning)
    {
        threadState=RunStateStopOrder;

        if(producerCond->iswaiting())
        {
            producerCond->wakeup();
        }

        mutex->unlock();

        int clockDown=10;

        while(threadState!=RunStateStopped && clockDown)
        {
            ADM_usleep(50*1000);
            clockDown--;
        };

        ADM_info("Thread stopped, continuing dtor\n");
    }
    else
    {
            mutex->unlock();
    }
    return true;
}
// EOF

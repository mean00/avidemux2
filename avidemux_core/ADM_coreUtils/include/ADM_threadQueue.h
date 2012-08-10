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
#ifndef ADM_THREAD_QUEUE_H
#define ADM_THREAD_QUEUE_H
#include "ADM_coreUtils6_export.h"
#include "ADM_threads.h"

using namespace std;
#include "BVector.h"

/**
    \struct ADM_audioPacket
*/
typedef struct
{
    uint8_t *data;
    uint32_t dataLen;
    uint64_t dts;
    uint64_t pts;
}ADM_queuePacket;

typedef  BVector <ADM_queuePacket> ListOfQueuePacket;

typedef enum
{
    RunStateIdle,
    RunStateRunning,
    RunStateStopOrder,
    RunStateStopped
}RunState;

/**
    \class ADM_threadQueue
    \brief Wrap queue/thread

*/
class ADM_COREUTILS6_EXPORT ADM_threadQueue
{
  protected:
                ListOfQueuePacket list;
                ListOfQueuePacket freeList;
                admMutex          *mutex;
                admCond           *cond;
                bool              started;
volatile       RunState          threadState;
                pthread_t         myThread;
  public:


                                    ADM_threadQueue() ;
                virtual             ~ADM_threadQueue();
                void                run(void);
protected:
        virtual bool                runAction(void)=0; 
                bool                startThread(void);
                bool                stopThread(void);
};


#endif


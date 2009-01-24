/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADM_default.h"
#include "ADM_threads.h"

#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audiofilter/audioeng_buildfilters.h"

#include "ADM_audioQueue.h"

/*
      Slave thread that will put data in the packetQueue
*/
int defaultAudioQueueSlave( audioQueueMT *context )
{
#if 0
#define QBUFFER 4096*8
  uint32_t total_sample=0;
  uint32_t samples,audioLen;
  PacketQueue *queue=context->packetQueue;
  AVDMGenericAudioStream *audioEncoder=context->audioEncoder;
  
  ADM_assert(queue);
  ADM_assert(audioEncoder);
  
  uint8_t buffer[QBUFFER];
  
  printf("[AudioQueueThread] Starting\n");
  while(audioEncoder->getPacket(buffer, &audioLen, &samples) && total_sample<context->audioTargetSample)
  {
    if(audioLen> QBUFFER)
    {
        printf("[AudioQueueThread] BufferOverflow %u/%u\n",audioLen,QBUFFER);
        ADM_assert(0);
    }
    if(queue->isAborted())
    { 
        printf("[AudioQueueThread] Aborting..\n");
        break;
    }
    total_sample+=samples;
    //printf("Audio %u\n",samples);
    accessMutex.lock();
    if(context->audioAbort)
    {
      context->audioDone=1;
      queue->Finished();
      printf("[AudioQueueThread] Aborting\n");
      printf("[AudioThread] Target %u, got %u, %f %%\n",context->audioTargetSample,total_sample,
             (float)total_sample/(float)context->audioTargetSample);
      accessMutex.unlock();
      return 1;
    }
    accessMutex.unlock();
    if(audioLen) 
    {
      queue->Push(buffer,audioLen,samples); 
    }
    accessMutex.lock();
    context->feedAudio+=audioLen;
    accessMutex.unlock();
  }
  accessMutex.lock();
  // Let's say audio is always ok, shall we :)
  context->audioDone=1;
  queue->Finished();
  accessMutex.unlock();
  printf("[AudioQueueThread] Exiting\n");
  printf("[AudioThread] Target %u, got %u, %f %%\n",context->audioTargetSample,total_sample,
         (float)total_sample/(float)context->audioTargetSample);
#endif
  return 1;
}

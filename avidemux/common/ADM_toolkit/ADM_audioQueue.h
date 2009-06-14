/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_AQUEUE_H
#define ADM_AQUEUE_H
typedef  void * (*THRINP)(void *p);

#include "ADM_toolkit/ADM_packetQueue.h"
extern admMutex accessMutex;

typedef struct 
{
  PacketQueue               *packetQueue;
  void                      *audioEncoder;
  uint32_t                  audioTargetSample;
  volatile uint32_t         audioDone;
  uint32_t                  feedAudio;
  volatile uint32_t         audioAbort;
  void                      *opaque;
}audioQueueMT;

extern int defaultAudioQueueSlave( audioQueueMT *context );

#endif
//EOF

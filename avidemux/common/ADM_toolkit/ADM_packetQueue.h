/***************************************************************************
              
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_PACKET_QUEUE_H
#define ADM_PACKET_QUEUE_H

typedef struct  
{
  uint32_t      size;       //< Size of the packet
  uint32_t      sample;
  uint32_t      startAt;    //< Index in the buffer where the packet start
}Slots;
/*!
  This class defines an packetQueue. The big buffer is split into slots
  (dynamically allocated).
  */
class PacketQueue
{
  protected:
            admMutex *_mutex;
            admCond  *_pusherCond;
            admCond  *_poperCond;
            uint32_t _nbSlots;
            Slots    *_slots;
            uint8_t  *_buffer;
            uint32_t  _bufferSize;
            
            uint32_t  _slotHead,_slotQueue;
            uint32_t  _bufferHead, _bufferQueue;
            char      *_name;
            uint8_t   _eof;
            uint32_t  availableSpace(void);
            uint8_t  _aborted;
  public:
          PacketQueue(const char *name,uint32_t nbSlot,uint32_t buffSize);
          ~PacketQueue();
          uint8_t   Push(uint8_t *ptr, uint32_t size,uint32_t sample);
          uint8_t   Pop(uint8_t *ptr, uint32_t *size,uint32_t *sample);
          uint8_t   IsFull(void);
          uint8_t   IsEmpty(void);
          uint8_t   Finished(void);
          uint8_t   Abort(void);
          uint8_t   isEof(void);
          uint8_t   isAborted(void) {return _aborted;}
};


#endif
//EOF


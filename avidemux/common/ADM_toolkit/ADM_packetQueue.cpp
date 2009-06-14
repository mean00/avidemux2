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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADM_default.h"
#include "ADM_threads.h"

#include "ADM_packetQueue.h"
/*!
  Constructor for packetQueue
  \param name   : name of the packetQueue, useful for debugging
  \param nbSlot : How many packets can the packetQueue hold
  \param buffSize : BufferSize in bytes
 */

PacketQueue::PacketQueue(const char *name,uint32_t nbSlot,uint32_t buffSize)
{
  
  _nbSlots=nbSlot;
  _bufferSize=buffSize;
  _name=ADM_strdup(name);
  _buffer=new uint8_t[_bufferSize];
  _slots=new Slots[_nbSlots];
  _mutex=new admMutex(_name);
  _pusherCond=new admCond(_mutex);
  _poperCond=new admCond(_mutex);
  _slotQueue=_slotHead=0;
  _bufferQueue=_bufferHead=0;
  _eof=0;
  _aborted=0;
  printf("PacketQueue %s created\n",_name);
}
/*!
  Destructor
 */

PacketQueue::~PacketQueue()
{
  if(_pusherCond) delete _pusherCond;
  _pusherCond=NULL;
  if(_poperCond) delete _poperCond;
  _poperCond=NULL;
  if(_mutex) delete _mutex;
  _mutex=NULL;
  if(_name) ADM_dealloc(_name);
  _name=NULL;
  if(_buffer) delete [] _buffer;
  _buffer=NULL;
  if(_slots) delete [] _slots;
}
/*!
  Signal the pusher has finished, wake the poper.
  Warning : Must be called with mutex held
 */

uint8_t   PacketQueue::IsEmpty(void)
{
  uint8_t r=0;
  
  if(_slotHead==_slotQueue)
  { 
      r=1;
  }
  
  return r;
}
/*!
  Signal the pusher has finished, wake the poper.

 */

uint8_t  PacketQueue::Finished(void)
{
  uint8_t r=0;
  _mutex->lock();
  _eof=1;
  _poperCond->abort();
  _mutex->unlock();
  return 1;
  
}
/*!
  Put a packet in the buffer
  Wait on _pusherCond if no slot available or buffer full
  \param ptr  : Ptr where to take the packet from
  \param size : packetsize

 */
uint8_t   PacketQueue::Push(uint8_t *ptr, uint32_t size,uint32_t sample)
{
  uint8_t r=0;
  uint32_t slot;
  uint32_t available;
  _mutex->lock();
  // First try to allocate a slot
  while(((_nbSlots+_slotHead-_slotQueue)%_nbSlots)==1)
  {
    _pusherCond->wait();
    _mutex->lock(); 
  }
  // Ok we have a slot,
  // Now lets's see if we have enough data in the buffer (we are still under lock)
  while(!_eof)
  {
      available=availableSpace(); 
      if(available>=size)
      {
        
         slot=_slotHead;
        _slotHead++;
        _slotHead%=_nbSlots;
        _slots[slot].size=size;
        _slots[slot].sample=sample;
        _slots[slot].startAt=_bufferHead;
        
       // printf("Pushing slot %d at %u\n",slot,_bufferHead,_slots[slot].startAt);
        if(_bufferSize>=(_bufferHead+size))
        {
          memcpy(&_buffer[ _bufferHead],ptr,size);
          _bufferHead+=size;
        }
        else  // Split
        {
          uint32_t part1=_bufferSize-_bufferHead;
          memcpy(&_buffer[ _bufferHead],ptr,part1);
          memcpy(&_buffer[ 0],ptr+part1,size-part1);
          _bufferHead=size-part1;
        }
        // Look if someone was waiting ...
        if(_poperCond->iswaiting())
        {
            _poperCond->wakeup();
        }
        _mutex->unlock();
        return 1;
      }
      _pusherCond->wait();
      _mutex->lock();
  }
  _mutex->unlock();
  printf("[PKTQ] %s is eof\n",_name);
  return 0;
}
/*!
  Read a packet from the queue
  Wait on the _poperCond if empty
  If aborted returns immediatly
  \param ptr : Ptr where to put the packet
  \param size : returns packetsize

*/
uint8_t   PacketQueue::Pop(uint8_t *ptr, uint32_t *size,uint32_t *sample)
{
  uint8_t r=0;
  uint32_t slot;
  uint32_t available,sz,position;
  _mutex->lock();
  // is there something ?
  while(IsEmpty() && !_eof)
  {
    _poperCond->wait();
    _mutex->lock();
  }
  if(IsEmpty() && _eof)
  {
    *size=0;
    _mutex->unlock();
    return 0;
  }
  //
  //printf("Pop : Head %u Tail %u\n",_slotHead,_slotQueue);
  // ok, which slot ?
  slot=_slotQueue;
  _slotQueue++;
  _slotQueue%=_nbSlots;
  sz=*size=_slots[slot].size;
  *sample=_slots[slot].sample;
  //printf("Poping slot %d at %u\n",slot,_bufferQueue,_slots[slot].startAt);
  if(!_bufferQueue==_slots[slot].startAt)
  {
    printf("Buffer Q:%u\n",_bufferQueue);
    printf("Slot :%u\n",_slots[slot].startAt);
    ADM_assert(0);
  }
  
  if(_bufferSize>=(_bufferQueue+sz))
  {
    memcpy(ptr,&(_buffer[_bufferQueue]),sz);
    _bufferQueue+=sz;
    _bufferQueue%=_bufferSize;
  }
  else  // Split
  {
    uint32_t part1=_bufferSize-_bufferQueue;
    memcpy(ptr,&_buffer[ _bufferQueue],part1);
    memcpy(ptr+part1,&_buffer[ 0],sz-part1);
    _bufferQueue=sz-part1;
  }
  // In case we made some space
  if(_pusherCond->iswaiting())
  {
    uint32_t third=(2*_bufferSize)/3;
    uint32_t available=availableSpace(); 
    // Only wakeup if we go over 1/3 of the buffer
    ADM_assert(available>=sz);
    if(available > third)
        _pusherCond->wakeup();
  }
  _mutex->unlock();
  return 1;
}
/*!
  Returns available space in bytes in the buffer
  Warning, this method must be called with mutex
    hold!.

*/
uint32_t PacketQueue::availableSpace(void)
{
  
  uint32_t space=_bufferSize+_bufferHead-_bufferQueue;
  space=space%_bufferSize;
  space=_bufferSize-space;
  
  return space;
}
uint8_t PacketQueue::Abort(void)
{
  
  Finished();
  
  _mutex->lock();
  _aborted=1;
  _slotHead=_slotQueue=0;
  _bufferQueue=_bufferHead=0;
  _mutex->unlock();
  _pusherCond->abort();
  
  return 1;
}
uint8_t   PacketQueue::isEof(void)
{
uint8_t r=0;
_mutex->lock();
    if((_eof||_aborted) && _slotHead==_slotQueue) r=1;
_mutex->unlock();
return r;
}
//EOF


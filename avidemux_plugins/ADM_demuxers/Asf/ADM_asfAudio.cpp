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


#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_asf.h"
/**
    \fn getDurationInUs
*/
uint64_t  asfAudioAccess::getDurationInUs(void)
{
    return 0;
}
/**
    \fn asfAudioAccess
*/

asfAudioAccess::~asfAudioAccess()
{
	printf("[asfAudio] Destroying track\n");

	fclose(_fd);
	_fd = NULL;


	delete _packet;


	_packet = NULL;
}

/**
    \fn asfAudioAccess
*/
                               
asfAudioAccess::asfAudioAccess(asfHeader *father,uint32_t myRank)
{
  printf("[asfAudio] Creating track\n");
    _myRank=myRank;
    _father=father;
    _track=&(_father->_allAudioTracks[myRank]);
    extraDataLen=_track->extraDataLen;
    extraData= _track->extraData;
    _streamId=_track->streamIndex;
    _dataStart=_father->_dataStartOffset;
    _fd=fopen(_father->myName,"rb");
    ADM_assert(_fd);
    fseeko(_fd,_dataStart,SEEK_SET);
    _packetSize=_father->_packetSize;
    _packet=new asfPacket(_fd,_father->_nbPackets,_packetSize,
                          &readQueue,_dataStart);
    
    printf("[asfAudio] Length %u\n",getLength());
}

uint64_t  asfAudioAccess::getPos(void)
{
    return 0;
}

/**
    \fn setPos
*/

bool   asfAudioAccess::setPos(uint64_t newoffset)
{
  // Look into the index until we find the audio
  // just after the wanted value
  for(int i=0;i<_father->nbImage;i++)
  {
    if(!_father->_index[i].audioSeen[_myRank]) continue;
    if(_father->_index[i].audioSeen[_myRank]>=newoffset)
    {
      // Flush queue
      _packet->purge();
      // Seek
      if(!_packet->goToPacket(_father->_index[i].packetNb))
      {
        printf("[asfAudio] Cannot seek to frame %u\n",i);
        return 0; 
      }
      printf("[asfAudio]For audio %"LLU", seeking to packet %"LU"\n",newoffset,_father->_index[i].packetNb);
      _packet->nextPacket(_streamId);
      _packet->skipPacket();
      return 1;
    }
  }
  printf("[asfAudio] Seek failed for offset=%"LLU"\n",newoffset);
  return 1; 
}

/**
    \fn goToTime
*/

bool   asfAudioAccess::goToTime(uint64_t dts_us)
{
    AsfVectorIndex     *idx=&(_father->_index);
    // Search
    int size=idx->size();
    if(dts_us<=(*idx)[0].audioDts[_myRank])
    {
          return setPos( 0);
    }
    for(int i=0;i<size-1;i++)
    {
        if(dts_us>=(*idx)[i].audioDts[_myRank] && dts_us<(*idx)[i+1].audioDts[_myRank])
        {
            return _packet->goToPacket( (*idx)[i].packetNb);
        }
    }
    return false;
}

/**
    \fn getPacket

*/
bool  asfAudioAccess::getPacket(uint8_t *dest, uint32_t *len, uint32_t maxSize,uint64_t *dts)
{
  *len=0;
  uint32_t delta;
  uint8_t r;
  while(1)
  {
   
    while(!readQueue.isEmpty())
    {
      asfBit *bit;
      ADM_assert(readQueue.pop((void**)&bit));
      //printf("[Asf] Audio found packet of size %d seq %d\n",bit->len,bit->sequence);
      
      // still same sequence ...add
      memcpy(dest,bit->data,bit->len);
      *len=bit->len;
      *dts=ADM_NO_PTS;
      delete[] bit->data;
      delete bit;
      return 1;
    }
    r=_packet->nextPacket(_streamId);
    _packet->skipPacket();
    if(!r)
    {
      printf("[ASF] Audio Packet Error\n");
      return 0; 
    }
    
   // _packet->skipPacket();
  }
  
  return 0; 
}
//EOF

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
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"


#include "ADM_asf.h"


/*
    __________________________________________________________
*/

asfAudio::~asfAudio()
{
	printf("[asfAudio] Destroying track\n");

	fclose(_fd);
	_fd = NULL;

	delete _wavheader;
	delete _packet;

	_wavheader = NULL;
	_packet = NULL;
}
/*
    __________________________________________________________
*/
                               
asfAudio::asfAudio(asfHeader *father,uint32_t myRank)
{
  printf("[asfAudio] Creating track\n");
    _myRank=myRank;
    _father=father;
    _track=&(_father->_allAudioTracks[myRank]);
    
    _wavheader=new WAVHeader;
    memcpy(_wavheader,&(_track->wavHeader),sizeof(WAVHeader));
    _extraDataLen=_track->extraDataLen;
    _extraData= _track->extraData;
    _length=_track->length;
    _streamId=_track->streamIndex;
    _dataStart=_father->_dataStartOffset;
    _fd=fopen(_father->myName,"rb");
    ADM_assert(_fd);
    fseeko(_fd,_dataStart,SEEK_SET);
    _packetSize=_father->_packetSize;
    _packet=new asfPacket(_fd,_father->_nbPackets,_packetSize,
                          &readQueue,_dataStart);
    _destroyable=1;
    printf("[asfAudio] Length %u\n",_length);
  
}
/*
    __________________________________________________________
*/

uint32_t            asfAudio::read(uint32_t len,uint8_t *buffer)
{
  uint32_t lan,samples;
  if(!getPacket(      buffer,    &lan, &samples)) return 0;
  return lan;
}
/*
    __________________________________________________________
*/

uint8_t   asfAudio::goTo(uint32_t newoffset)
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
      printf("[asfAudio]For audio %u, seeking to packet %u\n",newoffset,_father->_index[i].packetNb);
      _packet->nextPacket(_streamId);
      _packet->skipPacket();
      return 1;
    }
  }
  printf("[asfAudio] Seek failed for offset=%u\n",newoffset);
  return 1; 
}
/*
    __________________________________________________________
*/

uint8_t   asfAudio::goToTime(uint32_t newoffset)
{
  // Compute the linear version
  float f;
  uint32_t val;
  
  f=newoffset;
  f*=_wavheader->byterate;
  
  val=(uint32_t)((f+459.)/1000.);
  goTo(val);
  return 1; 
}
/*
    __________________________________________________________
*/

uint8_t   asfAudio::extraData(uint32_t *l,uint8_t **d)
{
  if(_extraData)
  {
    *l=_extraDataLen;
    *d=_extraData;  
  }
  else
  {
    *l=0;
    *d=NULL; 
  }
  return 1;
}
/*
    __________________________________________________________
*/

uint8_t  asfAudio::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
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
      printf("Audio found packet of size %d seq %d\n",bit->len,bit->sequence);
      
      // still same sequence ...add
      memcpy(dest,bit->data,bit->len);
      *len=bit->len;
#warning FIXME      
      *samples=384; // ?
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
    _packet->skipPacket();
  }
  
  return 0; 
}
//EOF

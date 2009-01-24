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

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"


#include "ADM_amv.h"

#define vprintf(...) {}

/**
    \fn ~amvAudio
    
*/
 amvAudio ::~amvAudio()
{
  
  if(_fd) fclose(_fd);
  _fd=NULL;
}
/**

*/
uint32_t    amvAudio::read(uint32_t len,uint8_t *buffer)
{
  uint32_t l,sam;
  if(curIndex>=_track->nbIndex) 
  {
    printf("[AMV] Packet %u/%u\n",curIndex,_track->nbIndex);
    return 0;
  }
  uint32_t sz=_track->index[curIndex].size;
  uint32_t left=sz-curOffset;
  if(left>=len)
  {
    fread(buffer,len,1,_fd);
    curOffset+=len;
    return len;
  }
  // Read remaining
  
       fread(buffer,left,1,_fd);
       curIndex++;
       curOffset=0;
       if(curIndex>=_track->nbIndex)  return left;
       fseeko(_fd,_track->index[curIndex].pos,SEEK_SET);
       curOffset=0;
       return left+read(len-left,buffer+left);
}
/**
      \fn goTo
*/
uint8_t             amvAudio::goTo(uint32_t newoffset)
{
  curIndex=0;
  curOffset=0;
  fseeko(_fd,_track->index[0].pos,SEEK_SET);  
  return 1;
}
/**
      \fn extraData
*/
uint8_t             amvAudio::extraData(uint32_t *l,uint8_t **d)
{
  *l=0;
  *d=NULL;
}
#if 0
/**
    \fn getPacket
*/
uint8_t             amvAudio::getPacket(uint8_t *dest, uint32_t *packlen, uint32_t *samples)
{
  *packlen=0;
   if(curIndex>=_track->nbIndex) return 0;
  fseeko(_fd,_track->index[curIndex].pos,SEEK_SET);
  fread(dest,_track->index[curIndex].size,1,_fd);
  curIndex++;
  *packlen=_track->index[curIndex].size;
  *samples=384;
  return 1;
}
#endif
/**
      \fn amvAudio 
*/
   amvAudio::amvAudio(const char *name,amvTrack *track,WAVHeader *hdr)
  : AVDMGenericAudioStream()
{
  _destroyable = 0; // Will be destroyed with master track
  _track=track;
    ADM_assert(_track);
  _fd=fopen(name,"rb");
  ADM_assert(_fd);

  _wavheader=new WAVHeader;
  memcpy(_wavheader,hdr,sizeof(WAVHeader));
  
  // Compute total length in byte
  _length=0;
  // 
  for(int i=0;i<_track->nbIndex;i++)
    _length+=_track->index[i].size;
  printf("[AMVAUDIO] found %lu bytes\n",_length);
  curIndex=0;
  curOffset=0;
  fseeko(_fd,_track->index[0].pos,SEEK_SET);  
}
//EOF


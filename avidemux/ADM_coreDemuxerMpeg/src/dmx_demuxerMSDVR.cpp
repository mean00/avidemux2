/***************************************************************************
       MSDVR aka mpeg ES in asf file
       
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
#if 0
#include <string.h>
#include <math.h>

#include "ADM_default.h"

#include "dmx_demuxerMSDVR.h"

#include "../ADM_asf/ADM_asfPacket.h"
#include "../ADM_asf/ADM_asf.h"

#define PARSER ((asfPacket *)aParser)
// ******************************************************
uint8_t         dmx_demuxerMSDVR::changePid(uint32_t newpid,uint32_t newpes)
{
  myPid=newpid ;
  myPes=newpes;
  _pesBufferStart=0;  // Big value so that we read
  _pesBufferLen=0;
  _pesBufferIndex=0;
}
// ******************************************************
dmx_demuxerMSDVR::dmx_demuxerMSDVR(uint32_t nb,MPEG_TRACK *tracks,uint32_t multi)
{
  consumed=0;
  aParser=NULL;
  stampAbs=0;
  _pesBuffer=new uint8_t [MAX_MSDVR_BUFFER];

  memset(seen,0,sizeof(seen));

  _pesBufferStart=0;  // Big value so that we read
  _pesBufferLen=0;
  _pesBufferIndex=0;
  ADM_assert(nb>0);
  tracked=NULL;

  nbTracked=nb;
  for(int i=0;i<256;i++) trackPTS[i]=ADM_NO_PTS;
  myPid=tracks[0].pid;
  
  if(nb!=256)     // Only pick one track as main, and a few as informative
  {
                
    memset(mask,0,256);
    tracked=new uint8_t[nbTracked];
    for(int i=1;i<nb;i++)
    {
      mask[tracks[i].pid&0xff]=1;
      tracked[i]=tracks[i].pid&0xff;
    }                
                
  }else
  {
    memset(mask,1,256); // take all tracks
  }

  _probeSize=0; 
  memset(seen,0,255*sizeof(uint64_t));     
  printf("Creating mpeg PS demuxer  main Pid: %X \n",myPid);
  _multi=multi;
  _fd=NULL;
  _dataStart=0;
}
// ******************************************************
dmx_demuxerMSDVR::~dmx_demuxerMSDVR()
{
  if(aParser)
  {
    asfPacket *p=PARSER;
    delete p;
    aParser=NULL;
  }
  if(_fd)
  {
    fclose(_fd);
    _fd=NULL; 
  }
  if(_pesBuffer) delete [] _pesBuffer;
  _pesBuffer=NULL;
  if(tracked) delete [] tracked;
  tracked=NULL;
}
/*
        Get stats about the PES ids tracked in tracked order
        if nbTracked=256 it means tracks all possible PES id
        The 0 rank is video
*/
// ******************************************************
uint8_t       dmx_demuxerMSDVR::getStats(uint64_t *oseen)
{
  if(nbTracked!=256)
  {
    oseen[0]=0;
    for(int i=1;i<nbTracked;i++)
    {
      oseen[i]=seen[tracked[i]];
    }
  }
  else
  {
    for(int i=0;i<nbTracked;i++)
    {
      oseen[i]=seen[i];
    }
  }
  return 1;
}
// ******************************************************
uint8_t         dmx_demuxerMSDVR::getAllPTS(uint64_t *stat)
{
  if(nbTracked!=256)
  {
    stat[0]=0;
    for(int i=1;i<nbTracked;i++)
    {
      stat[i]=trackPTS[tracked[i]];
    }
  }
  else
  {
    for(int i=0;i<nbTracked;i++)
    {
      stat[i]=trackPTS[i];
    }
  }
  return 1;
}
// ******************************************************
uint8_t dmx_demuxerMSDVR::setProbeSize(uint32_t sz)
{
  _probeSize=sz;
  return 1;
}
// ******************************************************
uint8_t dmx_demuxerMSDVR::forward(uint32_t f)
{
  uint32_t left;        
  if(_pesBufferIndex+f<=_pesBufferLen) 
  {
    _pesBufferIndex+=f;
    consumed+=f;
    return 1;
  }
        // else read another packet
  left=_pesBufferLen-_pesBufferIndex;
  f-=left;
  consumed+=left;
  if(!refill()) return 0;
  return forward(f);
}
// ******************************************************
uint8_t  dmx_demuxerMSDVR::stamp(void)
{
  consumed=0;        
}
// ******************************************************
uint64_t dmx_demuxerMSDVR::elapsed(void)
{
  return consumed;        
}
// ******************************************************
uint8_t  dmx_demuxerMSDVR::getPos( uint64_t *abs,uint64_t *rel)
{
  *rel=_pesBufferIndex;
  *abs=_pesBufferStart;       
  return 1;
}
// ******************************************************
uint64_t dmx_demuxerMSDVR::getSize( void) 
{
  return (uint64_t)_nbPackets; 
}
// ******************************************************
uint8_t dmx_demuxerMSDVR::setPos( uint64_t abs,uint64_t  rel)
{
				// Need to move ?
  if(abs==_pesBufferStart && _pesBufferLen)
  {
    if(_pesBufferLen<rel)
    {
      printf("Asked setpos to go %lu whereas %lu is max\n",
             rel,_pesBufferLen);
      ADM_assert(rel<_pesBufferLen);
    }
    _pesBufferIndex=rel;
    return 1;
  }
  // Seek...
  if(!PARSER->goToPacket(abs))
  {
    printf("[MSDVR] Seek to %u failed\n",abs); 
  }
  _pesBufferStart=abs;
  PARSER->purge(); // just in case
  if(!refill())
  {
    printf("DMX_PS: refill failed\n");
    return 0;
  }
                
  if(rel>_pesBufferLen)
  {
    printf("Set pos failed : asked rel:%lu max: %lu, absPos:%llu absPosafterRefill:%llu\n",
           rel,_pesBufferLen,abs,_pesBufferStart);
    ADM_assert(rel<_pesBufferLen);                        
  }

  _pesBufferIndex=rel;
  return 1;
               
}
/*
        Sync on mpeg sync word, returns the sync point in abs/r
*/


// ******************************************************
uint32_t         dmx_demuxerMSDVR::read(uint8_t *w,uint32_t len)
{
  uint32_t mx;
                // enough in buffer ?
  if(_pesBufferIndex+len<=_pesBufferLen)
  {
    memcpy(w,_pesBuffer+_pesBufferIndex,len);
    _pesBufferIndex+=len;
    consumed+=len;
    return len;
  }
                // flush
  mx=_pesBufferLen-_pesBufferIndex;
  if(mx)
  {
    memcpy(w,_pesBuffer+_pesBufferIndex,mx);
    _pesBufferIndex+=mx;
    consumed+=mx;
    w+=mx;
    len-=mx;
  }
  if(!refill())
  {
    printf("Refill failed at %d  \n",_pesBufferStart);
    _lastErr=1;
    return 0;
  }
  return mx+read(w,len);
}
// ******************************************************
uint8_t         dmx_demuxerMSDVR::sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
  uint32_t val,hnt;
  *r=0;

  val=0;
  hnt=0;

                // preload
  hnt=read32i();
  if(_lastErr)
  {
    _lastErr=0;
    printf("\n io error , aborting sync 1\n");
    return 0;       
  }
  val=hnt&0xff;
  while(((hnt&0xffffff00)!=0x100))
  {

    hnt<<=8;
    val=read8i();
    hnt+=val;
    if(_lastErr)
    {
      _lastErr=0;
      printf("\n io error , aborting sync 2\n");
      return 0;
    }

  }

  *stream=val;
                // Case 1 : assume we are still in the same packet
  if(_pesBufferIndex>=4)
  {
    *abs=_pesBufferStart;
    *r=_pesBufferIndex-4;
    *pts=_pesPTS;
    *dts=_pesDTS;
  }
  else
  {       // pick what is needed from oldPesStart etc...
                        // since the beginning in the previous packet
    uint32_t left=4-_pesBufferIndex;
    left=_oldPesLen-left;
#if 0
                                 printf("Next packet : %I64X Len :%lu, using previous packet %I64X len:%u as pos=%lu\n",
                                 		_pesBufferStart,_pesBufferLen,_oldPesStart,_oldPesLen,_pesBufferIndex);
#endif
                                 if(left>_oldPesLen)
{
  printf("Need %lu bytes from previous packet, which len is %lu\n",left,_oldPesLen);
  ADM_assert(0);
}
                                *abs=_oldPesStart;
                                *r=left;
                                *pts=_oldPTS;
                                *dts=_oldDTS;
  }
  return 1;
}
// ******************************************************

uint8_t dmx_demuxerMSDVR::refill(void)
{
  // read one packet
                
          _pesDTS=ADM_NO_PTS;
          _pesPTS=ADM_NO_PTS;
          _pesBufferStart=0;
          _pesBufferLen=0;
          _pesBufferIndex=0;
          while(!_pesBufferLen)
          {
              if(!PARSER->nextPacket(0xff))
              {
                  printf("[MSDVR] Packet Error\n");
                  return 0; 
              }
          
              PARSER->skipPacket();
              // Now fill buffer
              if(demuxerQueue.isEmpty()) continue;
              while(!demuxerQueue.isEmpty())
              {
                asfBit *bit;
                ADM_assert(demuxerQueue.pop((void**)&bit));
                if(bit->stream!=myPid)
                {
                  if(bit->stream<0x100 && mask[bit->stream &0xff])
                  {
                    seen[bit->stream]+=bit->len;
                  }
		  if (bit->data) delete[] bit->data;
                  delete bit;
                }else
                {
                  _pesBufferStart=bit->packet;
                  memcpy(&(_pesBuffer[_pesBufferLen]),bit->data,bit->len);
                  _pesBufferLen+=bit->len;
                  if (bit->data) delete[] bit->data;
                  delete bit;
                }
              }
              return 1;
          }
          return 0;
}
/***********************************************/
uint8_t dmx_demuxerMSDVR::open(const char *name)
{
  int r=5;
  const chunky *id=NULL;
  //
  _fd=fopen(name,"rb");
  if(!_fd)
  {
    printf("Demuxer MSDVR open failed\n");
    return 0; 
  }
  // Get the data chunk and ignore others
  asfChunk h(_fd);

  printf("[MSDVR] Searching data\n");
  while(r--)
  {
    h.nextChunk();    // Skip headers
    id=h.chunkId();
    h.dump();
    if(id->id==ADM_CHUNK_DATA_CHUNK) break;
    h.skipChunk();
  }
  if(id->id!=ADM_CHUNK_DATA_CHUNK)
  {
    printf("[MSDVR] Cannot find data chunk\n");
    return 0; 
  }
  h.read32();
  h.read32();
  h.read32();
  h.read32();
  _nbPackets=(uint32_t) h.read64();
  h.read16();
  //********** Ready
  _dataStart=ftello(_fd);
  asfPacket *packet;
  packet=new asfPacket(_fd,_nbPackets,0x2000,&demuxerQueue,_dataStart);
  aParser=(void *)packet;
  printf("[MSDVR] Opened ok\n");
  return 1;
}
#endif
// ******************************************************
// EOF

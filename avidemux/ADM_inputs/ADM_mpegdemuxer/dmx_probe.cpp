/***************************************************************************
                        Probe for a stream                                              
                             
    
    copyright            : (C) 2005 by mean
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

#include "ADM_assert.h"


#include "DIA_fileSel.h"
#include "fourcc.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_working.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_MPEG
#include "ADM_osSupport/ADM_debug.h"

#include "dmx_demuxerPS.h"
#include "dmx_demuxerTS.h"
#include "dmx_identify.h"
#include "dmx_probe.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_busy.h"
#include "ADM_mp3info.h"
#include "ADM_a52info.h"
#include "ADM_dcainfo.h"



#define MAX_PROBE (10*1024*1024LL) // Scans the 4 first meg
#define MIN_DETECT (10*1024) // Need this to say the stream is present
//****************************************************************************************
typedef struct MPEG_PMT
{
   uint32_t         programNumber;
   uint32_t         tid;
}MPEG_PMT;
//****************************************************************************************

static uint8_t dmx_probePS(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks);
extern uint8_t dmx_probeTS(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE t);
static uint8_t dmx_probeMSDVR(const char *file, uint32_t *nbTracks,MPEG_TRACK **tracks);


//****************************************************************************************
uint8_t dmx_probe(const char *file, DMX_TYPE  *type, uint32_t *nbTracks,MPEG_TRACK **tracks)
{

        printf("Probing %s for streams...\n",file);
        *type=dmxIdentify(file);
        switch(*type)
        {
        case DMX_MPG_MSDVR:
                {
                  return dmx_probeMSDVR(file,nbTracks,tracks);
                  printf("This is MSDVR file\n"); 
                  *nbTracks=1;
                  *tracks=new MPEG_TRACK;
                  (*tracks)->pes=0xE0;
                  (*tracks)->pid=1;
                  return 1;
                }
        case DMX_MPG_ES:
        case DMX_MPG_H264_ES:
                {
                *nbTracks=1;
                *tracks=new MPEG_TRACK;
                (*tracks)->pes=0xE0;
                (*tracks)->pid=0;
                printf("It is ES, no need to look for audio\n");
                return 1;
                }
        case DMX_MPG_TS:
        case DMX_MPG_TS2:
                return dmx_probeTS(file,nbTracks,tracks, *type);

        case DMX_MPG_PS:
                return dmx_probePS(file,nbTracks,tracks);
        }
        return 0;
}
uint8_t dmx_probePS(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks)
{
uint8_t dummy[10];
uint64_t seen[256],abs,rel;
int     audio,video;

        // It is mpeg PS
        // Create a fake demuxer, set a probe limite and collect info for all streams found
        dmx_demuxerPS *demuxer;
        MPEG_TRACK    pseudo;

               pseudo.pes=0xea; // Hopefully not used
               pseudo.pid=0;
               demuxer=new dmx_demuxerPS(256,&pseudo,0);
               if(!demuxer->open(file))
                {
                        delete demuxer;
                        printf("Cannot open file file demuxer (%s)\n",file);
                        return 0;
                }
                DIA_StartBusy();
                demuxer->setProbeSize(MAX_PROBE);
                demuxer->read(dummy,1);
                demuxer->getStats(seen);
                demuxer->getPos( &abs,&rel);
                //abs>>=20;
                printf("Stopped at %"LLU", %"LLU" MB\n",abs,abs>>20);

                DIA_StopBusy();
        // Now analyze...
        video=0;
        // Take the first video track suitable
        for(int i=0xE0;i<0xE9;i++)
        {
                if(seen[i]>MIN_DETECT)
                {
                        video=i;
                        break;
                }
        }
        if(!video)
        {
                printf("Cannot find any video stream\n");
                delete demuxer;
                return 0;
        }
        
        audio=0;
#if 1
        for(int i=0;i<256;i++)
        {
                if(seen[i]) printf("%x: there is something %lu kb\n",i,seen[i]>>10);
        }
#endif
        // 1 count how much audio we have
        for(int i=0;i<9;i++) if(seen[i]>MIN_DETECT) audio++;
        for(int i=0xc0;i<0xc9;i++) if(seen[i]>MIN_DETECT) audio++;
        for(int i=0xA0;i<0xA9;i++) if(seen[i]>MIN_DETECT) audio++;
        for(int i=0x40;i<0x49;i++) if(seen[i]>MIN_DETECT) audio++;

        *nbTracks=audio+1;      
        *tracks=new MPEG_TRACK[*nbTracks];
        
        memset(*tracks,0,(audio+1)*sizeof(MPEG_TRACK));
        (*tracks)[0].pes=video;
        (*tracks)[0].streamType=ADM_STREAM_MPEG_VIDEO;
        audio=1;
#define DOME {(*tracks)[audio].pes=i;audio++;}
        for(int i=0;i<9;i++) if(seen[i]>MIN_DETECT) DOME;
        for(int i=0xc0;i<0xc9;i++) if(seen[i]>MIN_DETECT) DOME;
        for(int i=0xA0;i<0xA9;i++) if(seen[i]>MIN_DETECT) DOME;
        for(int i=0x40;i<0x49;i++) if(seen[i]>MIN_DETECT) DOME;

        // Now go a bit deeper and try to extract infos


        uint8_t buffer[BUFFER_SIZE];
        uint32_t read;
        uint32_t br,fq,offset,pes,chan;
        MpegAudioInfo mpegInfo;        

        for(int i=1;i<audio;i++)
        {

                pes=(*tracks)[i].pes;
                // Anything but PCM
                if((pes<0xC9 && pes>=0xc0) || ((pes<9)) || ((pes>=0x40 && pes<=0x49)))
                {
                        demuxer->changePid(0,pes);
                        demuxer->setPos(0,0);
                        read=demuxer->read(buffer,BUFFER_SIZE);
                        // We need about 5 Ko...
                        if(read>BUFFER_SIZE>>1)
                        {
                                if(pes<9)
                                {
                                        if(ADM_AC3GetInfo(buffer,read,&fq,&br,&chan,&offset))
                                        {
                                                (*tracks)[i].channels=chan;
                                                (*tracks)[i].bitrate=(8*br)/1000;
                                                (*tracks)[i].streamType=ADM_STREAM_AC3;
                                        }
                                }else
                                {

                                    if(pes>=0x40 && pes<=0x49)
                                    {
                                    uint32_t chan,samplerate,bitrate,framelength,syncoff,flags,nbs;
                                        if(ADM_DCAGetInfo(buffer,read,&samplerate,&bitrate,&chan, &syncoff,&flags,&nbs))
                                        {
                                                (*tracks)[i].channels=chan;
                                                (*tracks)[i].bitrate=bitrate;
                                                (*tracks)[i].streamType=ADM_STREAM_DTS;
                                                if(syncoff) printf("[probe] There are some %u heading bytes\n",syncoff);
                                        }

                                    }
                                    else
                                        if(getMpegFrameInfo(buffer,read,&mpegInfo,NULL,&offset))
                                        {
                                                if(mpegInfo.mode!=3)  (*tracks)[i].channels=2;
                                                         else  (*tracks)[i].channels=1;
                                                
                                                (*tracks)[i].bitrate=mpegInfo.bitrate;
                                                (*tracks)[i].streamType=ADM_STREAM_MPEG_AUDIO;
                                        }
                                }                

                        }
                }

        }

        delete demuxer;

        printf("Found video as %x, and %d audio tracks\n",video,audio-1);
        return 1;
}
/* ****************************************************** */
#include "../ADM_asf/ADM_asfPacket.h"
#include "../ADM_asf/ADM_asf.h"

#define PROBE_BUF 32000
#define MAX_MSVDR_STREAMS 6
#define MAX_PACKET_PROBE 2000;   // assuming a packet is 8kB we will probe around 15 Meg
#define DETECT_MIN 7000

uint8_t dmx_probeMSDVR(const char *file, uint32_t *nbTracks,MPEG_TRACK **ztracks)
{
  int r=5;
  const chunky *id=NULL;
  uint32_t nbPackets,dataStart;
  ADM_queue queue;
  MPEG_TRACK *tracks;
  uint8_t     buffer[MAX_MSVDR_STREAMS][PROBE_BUF*2];
  uint32_t    streamlen[MAX_MSVDR_STREAMS];
  
  memset(streamlen,0,MAX_MSVDR_STREAMS*sizeof(uint32_t));
  
      printf("**** Probing MSDVR file ****\n");
      // Assume first track is video 
      *nbTracks=1;
      tracks=new MPEG_TRACK[MAX_MSVDR_STREAMS];
      *ztracks=tracks;
      tracks[0].pes=0xE0;
      tracks[0].pid=1;
      tracks[0].pid=1;
      tracks[0].streamType=ADM_STREAM_MPEG_VIDEO;
      // Now check for track up to 5
      
      FILE *fd=NULL;
      
      fd=fopen(file,"rb");
      if(!fd)
      {
        printf("Demuxer MSDVR open failed\n");
        return 0; 
      }
  // Get the data chunk and ignore others
      asfChunk h(fd);

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
      nbPackets=(uint32_t) h.read64();
      h.read16();
      
  //********** Ready

      uint32_t probeceil=MAX_PACKET_PROBE;
      uint32_t count=0;
      
      if(probeceil>nbPackets) probeceil=nbPackets;
      
      dataStart=ftello(fd);
      asfPacket *packet;
      packet=new asfPacket(fd,nbPackets,0x2000,&queue,dataStart);
      
      printf("[MSDVR] Opened ok\n");
      // Parse and collect
      while(count++<probeceil)
      {
        if(!packet->nextPacket(0xff))
        {
          break; 
        }
        packet->skipPacket();
        // Now look into it
        while(!queue.isEmpty())
        {
          asfBit *bit;
          ADM_assert(queue.pop((void**)&bit));
          if(bit->stream>=MAX_MSVDR_STREAMS)
          {
            printf("Found stream %u, ignored\n",bit->stream); 
          }else
          {
            uint32_t len=streamlen[bit->stream];
            if(len<PROBE_BUF)
            {
              uint8_t *ptr=buffer[bit->stream];
              memcpy(ptr+len,bit->data,bit->len);
              len+=bit->len;
              streamlen[bit->stream]=len;
            }
          }
          delete bit;
      }
      } // /while
      // Now we have filled the buffers
      // identifies the content
      uint32_t sync;
      for(int i=2;i<MAX_MSVDR_STREAMS;i++)
      {
        printf("We have found %u bytes for streamId %u\n",streamlen[i],i);
        if(streamlen[i]>=PROBE_BUF)
        {  // We have a candidate
           // What is it ? AC3 or MP2 or subs ? 
          // Maybe mpegaudio ?
          MpegAudioInfo mpegInfo;
          if(getMpegFrameInfo(buffer[i],streamlen[i],&mpegInfo,NULL,&sync))
          {
            tracks[*nbTracks].channels=2;
            tracks[*nbTracks].streamType=ADM_STREAM_MPEG_AUDIO;
            if(mpegInfo.mode==3) 
              tracks[*nbTracks].channels=1;
            tracks[*nbTracks].bitrate=mpegInfo.bitrate;
            tracks[*nbTracks].pid=i;
            tracks[*nbTracks].pes=0xC0;
            *nbTracks=*nbTracks+1;
            continue;
          }
          // AC3 ?
          uint32_t fq, br, chan, sync;
          
          if(ADM_AC3GetInfo(buffer[i], streamlen[i],&fq, &br,&chan,&sync))
          {
            tracks[*nbTracks].channels=chan;
            tracks[*nbTracks].bitrate=(8*br)/1000;
            tracks[*nbTracks].pid=i;
            tracks[*nbTracks].pes=0;
            tracks[*nbTracks].streamType=ADM_STREAM_AC3;
            *nbTracks=*nbTracks+1;
            continue;
          }
          
        }
        
      } // /for
      packet->purge();
      delete packet;
      fclose(fd);
      return 1;
}
/****EOF**/

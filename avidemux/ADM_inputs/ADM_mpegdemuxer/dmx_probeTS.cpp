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

#include "ADM_default.h"
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
#define MAX_NB_PMT 50

//#define PROBE_TS_VERBOSE

//****************************************************************************************
typedef struct MPEG_PMT
{
   uint32_t         programNumber;
   uint32_t         tid;
}MPEG_PMT;
//****************************************************************************************

uint8_t dmx_probeTS(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE t);
//****************************************************************************************
static uint8_t dmx_probeTSBruteForce(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE type);
static uint8_t dmx_probeTSPat(const char *file, uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE type);
static uint8_t dmx_probePat(dmx_demuxerTS *demuxer, uint32_t *nbPmt,MPEG_PMT *pmts,uint32_t maxPmt);
static uint8_t dmx_probePMT(dmx_demuxerTS *demuxer, uint32_t pmtId,MPEG_TRACK *pmts,uint32_t *cur, uint32_t max);

static const char *dmx_streamTypeAsString(ADM_STREAM_TYPE st);
 
extern uint32_t mpegTsCRC(uint8_t *data, uint32_t len);
//*********************************************************
uint8_t runProbe(const char *file)
{
  uint32_t nb;
  MPEG_TRACK *t;
  return  dmx_probeTSPat(file, &nb,&t,DMX_MPG_TS);
  
}

uint8_t dmx_probeTS(const char *file,  uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE type)
{
  
    // Try through PMT/PAT first
      if( type==DMX_MPG_TS2 || !dmx_probeTSPat(file,nbTracks,tracks,type))
      //if( !dmx_probeTSPat(file,nbTracks,tracks,type))
      {
        
        printf("PAT/PMT Failed, using brute force\n");
        return dmx_probeTSBruteForce(file,nbTracks,tracks,type);
      }
      return 1;
}
/**************************************
****************************************************************
    Brute force pid scanning in mpeg TS file
    We seek all PES packets and store their PID and PES id
*****************************************************************/
#define MAX_FOUND_PID 100
#define CHECK(x) val=parser->read8i(); left--;if(val!=x) goto _next;
typedef struct myPid
{
  uint32_t pid;
  uint32_t pes;

}myPid;
/**
        \fn dmx_probeTSBruteForce
        \brief Extract PID by scanning the file and guessing what they are
        @param file Filename to scan
        @param *nbTrack # of tracks found
        @param **tracks Tracks found
        @param type demuxer type to use (TS or TS2)
        @return 1 on success, 0 on failure
*/
uint8_t dmx_probeTSBruteForce(const char *file, uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE type)
{

  // Brute force indexing
  //
  // Build a dummy track
MPEG_TRACK dummy[TS_ALL_PID];
fileParser *parser;
uint32_t   foundPid=0;
myPid      allPid[MAX_FOUND_PID];
uint8_t    buffer[BUFFER_SIZE];
MpegAudioInfo mpegInfo; 

    dummy[0].pid=0x1; // should no be in use
    dummy[0].pes=0xE0;

        dmx_demuxerTS demuxer(TS_ALL_PID,dummy,0,type);
        if(!demuxer.open(file))
        {
          return 0;
        }
    // Set probe to 10 Meg
      demuxer.setProbeSize(10*1024*1024L);
      parser=demuxer.getParser();
      // And run

      uint32_t pid,left,isPayloadStart,cc,val;
      uint64_t abs;
      while(demuxer.readPacket(&pid,&left, &isPayloadStart,&abs,&cc))
      {
        if(isPayloadStart)
        {
            // Is it a PES type packet
            // it should then start by 0x0 0x0 0x1 PID

            CHECK(0);
            CHECK(0);
            CHECK(1);
            val=parser->read8i();
            left--;
            // Check it does not exist already
            int present=0;
            for(int i=0;i<foundPid;i++) if(pid==allPid[i].pid) {present=1;break;}
            if(!present)
            {
              allPid[foundPid].pes=val;
              allPid[foundPid].pid=pid;
              foundPid++;
            }
            ADM_assert(foundPid<MAX_FOUND_PID);
        } 
_next:
        parser->forward(left);
      }
      if(!foundPid)
      {
         printf("ProbeTS: No PES packet header found\n");
         return 0;
      }
      //****************************************
      // Build information from the found Pid
      //****************************************
      for(int i=0;i<foundPid;i++) printf("Pid : %04x Pes :%02x \n",allPid[i].pid,allPid[i].pes);

      // Search for a pid for video track
      //
      *tracks=new MPEG_TRACK[foundPid];
      MPEG_TRACK *trk=*tracks;
      uint32_t vPid=0,vIdx;
      uint32_t offset,fq,br,chan;

      for(int i=0;i<foundPid;i++)
      {
        if(allPid[i].pes>=0xE0 && allPid[i].pes<=0xEA)
        {
            vPid=trk[0].pes=allPid[i].pes;
            trk[0].pid=allPid[i].pid;
            trk[0].streamType=ADM_STREAM_MPEG_VIDEO;
            vIdx=i;
            break;
        }
      }
      if(!vPid)
      {
        delete [] trk;
        *tracks=0;
        printf("probeTs: No video track\n");
        return 0;
      }
      // Now build the other audio (?) tracks
      allPid[vIdx].pid=0;
      uint32_t start=1,code,id,read;
      for(int i=0;i<foundPid;i++)
      {
        code=allPid[i].pes;
        id=allPid[i].pid;

        if(!id) continue;

        if((code>=0xC0 && code <= 0xC9) || ((code==0xbd)&& (type==DMX_MPG_TS)) || ((code==0xfd)&& (type==DMX_MPG_TS2)))
        {
            demuxer.changePid(id,code);
            demuxer.setPos(0,0);
            read=demuxer.read(buffer,BUFFER_SIZE);
            if(read!=BUFFER_SIZE) continue;
            if(code>=0xC0 && code <= 0xC9) // Mpeg audio
            {
              if(getMpegFrameInfo(buffer,read,&mpegInfo,NULL,&offset))
                   {
                      if(mpegInfo.mode!=3)  trk[start].channels=2;
                          else  trk[start].channels=1;
 
                      trk[start].bitrate=mpegInfo.bitrate;
                      trk[start].pid=id;
                      trk[start].pes=code;
                      trk[start].streamType=ADM_STREAM_MPEG_AUDIO;
                      start++;

                    }
            }
            else // AC3
            {
                  if(ADM_AC3GetInfo(buffer,read,&fq,&br,&chan,&offset))
                  {
                          trk[start].channels=chan;
                          trk[start].bitrate=(8*br)/1000;
                          trk[start].pid=id;
                          trk[start].pes=0;
                          trk[start].streamType=ADM_STREAM_AC3;
                          start++;
                  }

            }

        }
      }
      *nbTracks=start;
      return 1;
}

/**
      \fn     dmx_probeTSPat(char *file, uint32_t *nbTracks,MPEG_TRACK **tracks)
      \brief  Try to extract info from a Mpeg TS file using PAT, PMT etc..
      @return 1 on success, 0 on failure
      @param file: File to scan
      @param *nbTrack : number of track found (out)
      @param **tracks : contains info about the tracks found (out)

*/
uint8_t dmx_probeTSPat(const char *file, uint32_t *nbTracks,MPEG_TRACK **tracks,DMX_TYPE type)
{
MPEG_TRACK dummy[TS_ALL_PID];
fileParser *parser;
uint32_t   foundPid=0;
myPid      allPid[MAX_FOUND_PID];
uint8_t    buffer[BUFFER_SIZE];
MpegAudioInfo mpegInfo; 
#define MAX_STREAM 50
#define MAX_NB_TRACK 50
    dummy[0].pid=0x00; // should no be in use
    dummy[0].pes=0xE0;

        dmx_demuxerTS demuxer(1,dummy,1,type);
        if(!demuxer.open(file))
        {
          return 0;
        }
    // Set probe to 10 Meg
      demuxer.setProbeSize(10*1024*1024L);
      uint32_t nbPmt;
      MPEG_PMT pmts[MAX_NB_PMT];
      MPEG_TRACK xtracks[MAX_NB_TRACK];
      
      if(!dmx_probePat(&demuxer,&nbPmt,pmts,MAX_NB_PMT))
      {
        aprintf("[PSI Probe]Cannot find Pat\n"); 
        parser=NULL;
        return 0;
      }
      printf("Found %d PMT..\n",nbPmt);
      demuxer.setProbeSize(40*1024*1024L); // We can can increase the probe size
      uint32_t cur=0;
      for(int i=0;i<nbPmt;i++)
      {
         dmx_probePMT(&demuxer, pmts[i].tid,xtracks,&cur,MAX_NB_TRACK);
      }
      printf("***********************\n");
      printf("***********************\n");
      printf("Summary of stream found\n");
      printf("***********************\n");
      printf("***********************\n");
      for(int i=0;i<cur;i++)
      {
        printf("Tid : %04x Type :%d %s\n", xtracks[i].pid,xtracks[i].streamType,
               dmx_streamTypeAsString(xtracks[i].streamType));
      }
      printf("******************************\n");
      printf("******************************\n");
      printf("End of summary of stream found\n");
      printf("******************************\n");
      printf("******************************\n");
      
      if(!cur)        return 0;
      
      // Search first video track
      *tracks=new MPEG_TRACK[cur];
      int found=-1;
      for(int j=0;j<cur;j++)
      {
        ADM_STREAM_TYPE type=xtracks[j].streamType;
        if(type==ADM_STREAM_MPEG_VIDEO ||  type==ADM_STREAM_MPEG4 || type==ADM_STREAM_H264)
        {
          found=j;
          break;  
        }
      }
      if(found<0)
      {
        printf("No video track\n");
        delete [] *tracks;
        return 0; 
      }
      memcpy(*tracks,&(xtracks[found]),sizeof(MPEG_TRACK));
      *nbTracks=1;
      // Now do audio
      for(int j=0;j<cur;j++)
      {
         MPEG_TRACK *t=&(xtracks[j]);
         ADM_STREAM_TYPE type=t->streamType;
          if(type!=ADM_STREAM_MPEG_AUDIO && type!=ADM_STREAM_AC3
            &&type!=ADM_STREAM_AAC) continue; // Only mpega & AC3 for now
          switch(type)
          {
            case ADM_STREAM_AAC:
                  t->pes=0xb0;
            case ADM_STREAM_MPEG_AUDIO:
            case ADM_STREAM_AC3:
            
              memcpy(&((*tracks)[*nbTracks]),t,sizeof(MPEG_TRACK));
              ADM_assert(*nbTracks<cur);
              (*nbTracks)++;
              break;
            
            default: ADM_assert(0); 
          }
      }
      printf("Found %u tracks\n",*nbTracks);
      //
      return 1;
      
}
/**
      \fn     dmx_searchAndSkipHeader
      \brief  Search for a given PSI and skip header
      @return 1 on success, 0 on failure
      @param myPid : Pid of the looked for psi
      @param demuxer: mpegTS demuxer *(input)
      @param *currentSec : current section (output)
      @param *maxSec : #of sections (output)
      @param *leftbyte : Total #of bytes left in the packet
      @param *payloadSize : #of bytes of usable payload

*/
uint8_t dmx_searchAndSkipHeader(uint32_t myPid,dmx_demuxerTS *demuxer,uint32_t *currentSec, uint32_t *maxSec,
                                    uint32_t *leftbyte,uint32_t *payloadSize)
{
  
  uint8_t packet[TS_PACKET_SIZE*2];
  uint32_t tableId;
  uint32_t misc;
  uint32_t sectionLength;
  uint32_t tId,pid,left,cc,nbPmt;
  uint32_t version,isPayloadStart;
  uint32_t sectionNumber;
  uint32_t lastSectionNumber;
  uint32_t programInfoLength;
  uint32_t crc,crccomputed;
  uint64_t startPos,endPos,abso;
  fileParser *parser;
      demuxer->changePid(myPid,myPid); // Search PAT
      parser=demuxer->getParser();
      
        while(demuxer->readPacket(&pid,&left, &isPayloadStart,&abso,&cc))
        {
          if(pid!=myPid)
          {
            printf("Wrong Pid %x/%x\n",pid,myPid);
            parser->forward(left);
            continue;
          }
          if(!isPayloadStart || left <= (9+4))
          {
            parser->forward(left);
            continue;
          }

          /* Found something that looks good...*/
            
            /* Decode PSI header */
            parser->read8i(); /* Pointer field, can be ignored (?) */
              
              parser->getpos(&startPos); /* Memorize beginning */
              tableId=parser->read8i();
              misc=parser->read16i(); // +3
              tId=parser->read16i();  
              version=parser->read8i(); // +6
              sectionNumber=parser->read8i();
              lastSectionNumber=parser->read8i(); // +8
              
              sectionLength=misc&0xFFF;
              
              if(sectionLength<=9 || sectionLength > (left-4) || left<9)
              {
                printf("SectionLength too short :%d,left %d\n", sectionLength,left);
                 parser->setpos(startPos-1+left); // skip packet
                 continue;
              }
              
#ifdef PROBE_TS_VERBOSE
              printf("******************************************\n");
              printf("tableId        : %d\n",tableId);
              
              printf("sectionLength  : %d\n",sectionLength);
              printf("0              : %x\n",misc&0x40);
              printf("section syntax : %x\n",misc&0x80);
              printf("Transport ID   : 0x%x\n",tId);
              printf("Version Number : 0x%x\n",(version>>1)&0x1F);
              printf("CurrentNext    : 0x%x\n",version&1);
              
              printf("Section        : %d\n",sectionNumber);
              printf("LastSection    : %d\n",lastSectionNumber);
              
#endif
              // Check for error FIXME TODO
              
              // Check CRC
              parser->getpos(&endPos); // Here payload begins
              parser->setpos(startPos);
              parser->read32(sectionLength-1,packet); // Go back & Read Whole packet +3 for header -4 CRC
              crc=parser->read32i();
              crccomputed=mpegTsCRC(packet,sectionLength-1);
              if(crc!=crccomputed) // Bad CRC, skip packet
              {
                aprintf("Bad CRC\n");
                parser->setpos( startPos+left-1); // skip
                continue;
              }
              // CRC is ok, go back to interesting place
              aprintf("CRC OK\n");
              parser->setpos(endPos);
              *currentSec=sectionNumber;
              *maxSec=lastSectionNumber;
              
              *leftbyte=left-9;               // Total bytes left in packet
              *payloadSize=sectionLength-9; // No CRC, No header
              return 1;
          } // /while
      return 0;
}
/**
      \fn     dmx_probePat(dmx_demuxerTS *demuxer, uint32_t *nbPmt,MPEG_PMT *pmts,uint32_t maxPMT)
      \brief  Search for PAT and returns PMT info if found
      @return 1 on success, 0 on failure
      @param demuxer: mpegTS demuxer (input)
      @param *nbPmt : number of PMTS found (output)
      @param *pmts : contains info about the PMT found (out but must be allocated by caller)
      @param maxPMT : Maximum # of PMT we accept in pmts (in)

*/
uint8_t dmx_probePat(dmx_demuxerTS *demuxer, uint32_t *nbPmt,MPEG_PMT *pmts,uint32_t maxPmt)
{
  
  fileParser *parser;
  uint32_t curSection,maxSection;
  uint32_t left,toScan;
  
      parser=demuxer->getParser();
      *nbPmt=0;
      if(dmx_searchAndSkipHeader(0,demuxer,&curSection, &maxSection,&left,&toScan))
      {
        
              while(toScan >=4 && left>=8)
              {
                  printf("**\n");
                  pmts[*nbPmt].programNumber=parser->read16i()&0xFFFF;
                  pmts[*nbPmt].tid=parser->read16i()&0x1FFF;
                  aprintf(" [PAT]Program Number :%03x\n",pmts[*nbPmt].programNumber);
                  aprintf(" [PAT]PID for this   :%03x\n",pmts[*nbPmt].tid);
                  aprintf(" toScan :%u left :%u\n",toScan,left);
                  if((*nbPmt)<maxPmt)
                      (*nbPmt)++;
                  left-=4;
                  toScan-=4;
              }
              return 1;
        
      }
      return 0;
}
const char *dmx_streamType(uint32_t type,ADM_STREAM_TYPE *streamType)
{
 switch(type)
 {
   case 1:case 2: *streamType=ADM_STREAM_MPEG_VIDEO;return "Mpeg Video";
   case 3:case 4: *streamType=ADM_STREAM_MPEG_AUDIO;return "Mpeg Audio";
   case 0x11: case 0xF:        *streamType=ADM_STREAM_AAC;return "AAC  Audio";
   case 0x10:        *streamType=ADM_STREAM_MPEG4;return "MP4 Video";
   case 0x1B: *streamType=ADM_STREAM_H264;return "H264";
   case 0x81: *streamType=ADM_STREAM_AC3;return "Private (AC3?)";
 }
 *streamType=ADM_STREAM_UNKNOWN;
  return "???";
}
/**
      \fn     dmx_probePat(dmx_demuxerTS *demuxer, uint32_t *nbPmt,MPEG_PMT *pmts,uint32_t maxPMT)
      \brief  Search for PAT and returns PMT info if found
      @return 1 on success, 0 on failure
      @param demuxer: mpegTS demuxer (input)
      @param *nbPmt : number of PMTS found (output)
      @param *pmts : contains info about the PMT found (out but must be allocated by caller)
      @param maxPMT : Maximum # of PMT we accept in pmts (in)

*/
uint8_t dmx_probePMT(dmx_demuxerTS *demuxer, uint32_t pmtId,MPEG_TRACK *pmts,uint32_t *cur, uint32_t max)
{
  
  fileParser *parser;
  uint32_t curSection,maxSection;
  uint32_t left=0,toScan,programInfo=0;
  
      printf("Searching for PMT, pid=0x%x\n",pmtId);
      demuxer->changePid(pmtId,pmtId); // change pid as setPos will seek for them
      demuxer->setPos(0,0);
      parser=demuxer->getParser();
      if(dmx_searchAndSkipHeader(pmtId,demuxer,&curSection, &maxSection,&left,&toScan))
      {
          uint16_t alpha;
               alpha=parser->read16i();
               aprintf("[PMT]PCR for it    :x%x\n",alpha&0x1FFF);
               programInfo=parser->read16i() & 0x0FFF;
               aprintf("[PMT]Program Info  :%d\n",programInfo);
               if( (programInfo+2 > left) || (programInfo+2>toScan))
               {
                 printf("Program Info too big :%u\n",programInfo);
                 return 0;
               }
               parser->forward(programInfo);
               toScan-=(2+programInfo);
               left-=(2+programInfo);
               while(toScan >=5 )
              {
                  uint8_t stream;
                  uint32_t pid,esDescLen;
                  const char *idString;
                  ADM_STREAM_TYPE streamType;
                  aprintf("**\n");
                  stream    =parser->read8i();
                  pid       =parser->read16i()&0x1FFF;
                  esDescLen =parser->read16i()&0x0FFF;
                  idString=dmx_streamType(stream,&streamType);
                  aprintf("[PMT]Stream Type :0x%x (%s)\n",stream,idString);
                  aprintf("[PMT]Pid         :0x%x\n",pid);
                  aprintf("[PMT]ES Len      :%d\n",esDescLen);
                  
                  parser->forward(esDescLen);
                  left-=(5+esDescLen);
                  toScan-=(5+esDescLen);
                  if(*cur<max)
                  {
                    pmts[*cur].pid=pid;
                    pmts[*cur].streamType=streamType;
                    pmts[*cur].pes=0x0;
                    if(streamType==ADM_STREAM_MPEG_AUDIO) pmts[*cur].pes=0xC0;
                    if(streamType==ADM_STREAM_MPEG_VIDEO) pmts[*cur].pes=0xE0;
                    
                    (*cur)++;
                  }
                  aprintf("[PMT]left %u toscan %u\n",left,toScan);
              }
              
      }
      return 0;
}
/**
      \fn dmx_streamTypeAsSTring
      \brief returns stream type as a printable string
*/
static const char *dmx_streamTypeAsString(ADM_STREAM_TYPE st)
{
#define MST(x,y) case x: return #y;
  switch(st)
  {
  MST(ADM_STREAM_UNKNOWN,UNKNOWN)
  MST(ADM_STREAM_MPEG_VIDEO,MPEG12VIDEO)
  MST(ADM_STREAM_MPEG_AUDIO,MPEG12AUDIO)
  MST(ADM_STREAM_AC3,AC3)
  MST(ADM_STREAM_DTS,DTS)
  MST(ADM_STREAM_H264,H264)
  MST(ADM_STREAM_MPEG4,MPEG4)
  MST(ADM_STREAM_AAC,AAC)
    
  }
  return "???";
  
}
/****EOF**/

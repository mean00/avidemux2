/** *************************************************************************
    \file ADM_tsPatPmt.cpp
    \brief Analyze pat & pmt
    copyright            : (C) 2007 by mean
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
#include "ADM_ts.h"
#include "ADM_demuxerInternal.h"
#include "ADM_tsPatPmt.h"

#include "ADM_a52info.h"
#include "ADM_mp3info.h"

#include "ADM_coreUtils.h"

#define TS_MAX_PACKET_SCAN     2000
#define TS_NB_PACKET_THRESHOLD 5
#define MAX_PID (1<<17)
#define MAX_BUFFER_SIZE (10*1024)
static bool idAAC_ADTS(int pid,tsPacket *ts);
static bool idContent(int pid,tsPacket *ts,ADM_TS_TRACK_TYPE & trackType);
static bool idMP2(int pid,tsPacket *ts);
/**
    \fn scanForPrograms
    \brief Lookup PAT & PMT to get tracks
*/
bool TS_guessContent(const char *file,uint32_t *nbTracks, ADM_TS_TRACK **outTracks)
{
    ADM_info("[TS demuxer] Brute force reading...\n");
    bool result=false;
    ADM_TS_TRACK *tracks=NULL;
    *outTracks=NULL;
    *nbTracks=0;
    uint32_t nb=0;


    tsPacket *ts=new tsPacket();
    ts->open(file,FP_PROBE);

    int packetRead=0;
    int *map=new int[MAX_PID];
    memset(map,0,MAX_PID*sizeof(int));
    uint8_t *buffer=new uint8_t[MAX_BUFFER_SIZE];
    int pid;
    uint32_t packetSize;
    uint64_t pts,dts,at;
    while(packetRead++<TS_MAX_PACKET_SCAN)
    {
            if(false==ts->getNextPid(&pid))
            {
                    printf("Read %d packets\n",packetRead);
                    break;
            }
            if(pid<MAX_PID)
            {
                map[pid]++;
            }
    }
    // Keep only seen packet with more than 5 packets and >16
    std::vector <int>listOfPid;    
    for(int i=17;i<MAX_PID;i++)
    {
        if(map[i]>TS_NB_PACKET_THRESHOLD) listOfPid.push_back(i);
    }
    delete [] map;
    map=NULL;
    if(listOfPid.size())
    {
        tracks=new ADM_TS_TRACK[listOfPid.size()];
        memset(tracks,0,sizeof(ADM_TS_TRACK)*listOfPid.size());
        int validTracks=0;
        ADM_TS_TRACK_TYPE trackType;
        printf("List of found PID:\n");
        for(int i=0;i<listOfPid.size();i++)
            printf("\t Pid=%d\n",listOfPid[i]);
        printf("List end.\n");
        for(int i=0;i<listOfPid.size();i++)
        {
            printf("Found stuff in pid=%d\n",listOfPid[i]);
            //  Read PES
            if(true==idContent(listOfPid[i],ts,trackType))
            {
                tracks[validTracks].trackPid=listOfPid[i];
                tracks[validTracks].trackType=trackType;
                validTracks++;
            }  else ADM_info("Cannot identify type\n");
        }
        if(!validTracks)
        {
            delete [] tracks;
        }else
        {
            *outTracks=tracks;
            *nbTracks=validTracks;
            result=true;
        }
    }
    ts->close();
    delete ts;
    ts=NULL;
    delete [] buffer;
    buffer=NULL;
    // Swap tracks if needed, track [0] must be video...
    int videoIndex=-1;
    for(int i=0;i<*nbTracks;i++)
    {
        ADM_TS_TRACK_TYPE type=tracks[i].trackType;
        switch(type)
        {
            case ADM_TS_MPEG2:
            case ADM_TS_VC1:
            case ADM_TS_H264:
                    videoIndex=i;
                    break;
            default: break;
        }
    }
    if(videoIndex>0)
    {
        // Swap!
        ADM_TS_TRACK trk=tracks[0];
        tracks[0]=tracks[videoIndex];
        tracks[videoIndex]=trk;
    }
    ADM_info("Summary : found %d tracks\n",(int)*nbTracks);
    for(int i=0;i<*nbTracks;i++)
    {
        ADM_info("  Track : %d, pid=%d, type =%d\n",
                    i,tracks[i].trackPid,tracks[i].trackType);
    }
    ADM_info("End of summary.\n");
    return result;
}
/**
    \fn mpeg2StartCode
    \brief returns true if the start code is potentially the 1st startcode of a PES packet inside DVB
            We only accept pic 1st slice / Seq start/Gop start and extension
*/
static bool mpeg2StartCode(int code)
{
    if(code==0) return true;
    if(code==0xB3) return true; // seq start
    if(code==0xB8) return true; // GOP start
    if(code==0xB5) return true; // GOP start
    if(code==0xBA) return true; // GOP start
    if(code==0xB2) return true; // GOP start
    return false;
}
/**
    \fn idContentE0
    \brief try to identify video content, with pes-id=0xe0
    We read 5 startcode and if 4 or 5 looks like mpeg2/h264 ones we mark the stream as mpeg2/h264
*/
static bool idContentE0(int pid,tsPacket *ts,ADM_TS_TRACK_TYPE & trackType)
{
    int nbTry=5;
    int threshold=3;
    int nbSuccessMpeg2=0;
    int nbSuccessH264=0;
    for(int i=0;i<nbTry;i++)
    {
        TS_PESpacket pes(pid);
        if(false==ts->getNextPES( &pes)) 
        {
            ADM_warning("\tCannot get PES\n");
            return false;
        }
        uint8_t *ptr=pes.payload;
        uint32_t len=pes.payloadSize,syncoff;
        uint8_t  scode;
        if(ADM_findMpegStartCode(ptr+4,ptr+len,&scode,&syncoff))
        {
            ADM_warning("Found startcode1 =%x\n",scode);
        }else continue;
        if(scode==9) // AU delimiter => H264
        {
            nbSuccessH264++;
            continue;
        }
        if(mpeg2StartCode(scode))
        {
            nbSuccessMpeg2++;
            continue;
        }
    }
    if(nbSuccessH264>=threshold)
    {
        trackType=ADM_TS_H264;
        ADM_warning("Probably H264\n");
        return true;
    }
    if(nbSuccessMpeg2>=threshold)
    {
        trackType=ADM_TS_MPEG2;
        ADM_warning("Probably Mpeg2\n");
        return true;
    }
    ADM_warning("dont know what it is... (mpeg2 score=%d, h264 score=%d, threshold=%d)\n",nbSuccessMpeg2,nbSuccessH264,threshold);
    return false;
}

static bool mp2Match(const MpegAudioInfo *array,int a, int b)
{
    const MpegAudioInfo *x=array+a;
    const MpegAudioInfo *y=array+b;
    if(x->bitrate<=32) return false;
    if(x->bitrate!=y->bitrate) return false;
    if(x->mode!=y->mode) return false;
    if(x->samplerate!=y->samplerate) return false;
    return true;
}
/**
    \fn idContent
*/
bool idContent(int pid,tsPacket *ts,ADM_TS_TRACK_TYPE & trackType)
{
TS_PESpacket pes(pid);
TS_PESpacket pes2(pid);

        ts->setPos(0);
        if(false==ts->getNextPES( &pes)) 
        {
            ADM_warning("\tCannot get PES\n");
            return false;
        }
        uint8_t *ptr=pes.payload;
        uint32_t len=pes.payloadSize;

        ADM_info("PES start : %02x%02x%02x%02x\n",ptr[0],ptr[1],ptr[2],ptr[3]);
        if(!ptr[0] && !ptr[1] && ptr[2]==1 && (ptr[3]&0xe0)==0xe0) // probably video
        {
          return idContentE0(pid,ts,trackType);
        }

        // Read a 2nd packet for confirmation

        if(false==ts->getNextPES( &pes2)) 
        {
            ADM_warning("\tCannot get PES2\n");
            return false;
        }
        uint8_t *ptr2=pes2.payload;
        uint32_t len2=pes2.payloadSize;
        
        printf("\t Read %d bytes\n",(int)pes.payloadSize);  
        uint32_t fq,br,chan,syncoff;
        uint32_t fq2,br2,chan2,syncoff2;

        // Is it AC3 ?? No false positive with A52/AC3..
        if( ADM_AC3GetInfo(ptr,len, &fq, &br, &chan,&syncoff))
        {
                ADM_info("Maybe AC3... \n");
                // Try on 2nd packet...
                if( ADM_AC3GetInfo(ptr2,len2, &fq2, &br2, &chan2,&syncoff2))
                {
                    if((fq==fq2) && (br2==br) && (chan==chan2))
                    {
                        ADM_warning("\tProbably AC3 : Fq=%d br=%d chan=%d\n",(int)fq,(int)br,(int)chan);
                        trackType=ADM_TS_AC3;
                        return true;
                    }
                }

        }
        if(idMP2(pid,ts))
        {
                ADM_info("\t probably MP2\n");
                trackType=ADM_TS_MPEG_AUDIO;
                return true;
        }

        if(idAAC_ADTS(pid,ts))
        {
                ADM_info("\t probably AAC/ADTS\n");
                trackType=ADM_TS_AAC_ADTS;
                return true;
        }

        ADM_info("Cannot identify track\n");
        return false;
}
/**
    \fn idMP2
    \brief return true if the tracks is mp2
*/
bool idMP2(int pid,tsPacket *ts)
{
#define MP2_PROBE_SIZE (16*1024)
uint8_t  mp2Buffer[MP2_PROBE_SIZE*2];

        int limit=0;
        TS_PESpacket pes(pid);
        while(limit<MP2_PROBE_SIZE)
        {
            if(!ts->getNextPES(&pes))
            {
                ADM_warning("Cannot get PES for pid=%d\n",pid);
                return false;
            }
            int left=pes.payloadSize-pes.offset;
            memcpy(mp2Buffer+limit,pes.payload+pes.offset,left);
            limit+=left;
        }
        ADM_info("\t read % bytes\n",limit);
        // Now read buffer until we have 3 correctly decoded packet
        int probeIndex=0;
        while(probeIndex<limit)
        {

            uint8_t *ptr=mp2Buffer+probeIndex;
            int     len=limit-probeIndex;
            if(len<4)
            {
                    ADM_info("\t no sync(3)\n");
                    return false;
            }
            uint32_t syncoff,syncoff2;
            MpegAudioInfo mp2info,confirm;
            if( !getMpegFrameInfo(ptr,len,&mp2info,NULL,&syncoff))
            {
                    ADM_info("\t no sync\n");
                    return false;
            }
          // Skip this packet
            int next=probeIndex+syncoff+mp2info.size;
            len=limit-next;
            if(len<4)
            {
                    ADM_info("\t no sync(2)\n");
                    return false;
            }
            if(getMpegFrameInfo(mp2Buffer+next,len,&confirm,&mp2info,&syncoff2))
            {
                    if(!syncoff2)
                    {
                            ADM_warning("\tProbably MP2 : Fq=%d br=%d chan=%d\n", (int)mp2info.samplerate,
                                                                (int)mp2info.bitrate,
                                                                (int)mp2info.mode);
                            return true;
                    }
            }
            probeIndex+=syncoff+1;
        }
        return false;
}
        /**
    \fn idMP2
    \brief return true if the tracks is AAC/ADTS
*/
bool idAAC_ADTS(int pid,tsPacket *ts)
{
#define ADTS_NB_PACKET 10
#define ADTS_MIN_MATCH 7
        int match=0;
        TS_PESpacket pes(pid);
        for(int i=0;i<ADTS_NB_PACKET;i++)
        {
            if(!ts->getNextPES(&pes))
            {
                ADM_warning("ADTS:Cannot get PES for pid=%d\n",pid);
                return false;
            }
            uint8_t *p=pes.payload+pes.offset;
            printf("%02x %02x\n",p[0],p[1]);
            if((*p)!=0xff)
            {
                continue;
            }
            if((p[1]&0xF0)!=0xF0)
                continue;
            match++;
        }
        ADM_info("\t Adts match : %d/%d\n",match,ADTS_NB_PACKET);
        if(match>=ADTS_MIN_MATCH) 
        {
            
            return true;
        }
        
        return false;
}

//EOF

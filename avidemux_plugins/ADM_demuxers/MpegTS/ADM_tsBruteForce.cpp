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

static bool idContent(int pid,tsPacket *ts,ADM_TS_TRACK_TYPE & trackType);

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
            }        
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
        if(false==ts->getNextPES( &pes2)) 
        {
            ADM_warning("\tCannot get PES2\n");
            return false;
        }
        
        printf("\t Read %d bytes\n",(int)pes.payloadSize);
        // 
        uint32_t fq,br,chan,syncoff;
        uint32_t fq2,br2,chan2,syncoff2;
        uint8_t *ptr=pes.payload;
        uint32_t len=pes.payloadSize;
        uint8_t *ptr2=pes2.payload;
        uint32_t len2=pes2.payloadSize;

        if(!ptr[0] && !ptr[1] && ptr[2]==1 && ptr[3]==0xe0) // probably video
        {
            // Only supports Mpeg2 video and H264
            // For mpeg2 we scan for slice i.e. 00 00 01 00 etc...
            // we need 2 consecutive slice
            uint8_t scode,scode2;
            if(ADM_findMpegStartCode(ptr+4,ptr+len,&scode,&syncoff))
            {
                ADM_warning("Found startcode1 =%x\n",scode);
            }
            if(ADM_findMpegStartCode(ptr2+4,ptr2+len2,&scode2,&syncoff))
            {
                ADM_warning("Found startcode2 =%x\n",scode2);
            }
            if(scode==9 && scode2==9) // AU delimiter => H264
            {
                trackType=ADM_TS_H264;
                ADM_warning("Probably H264\n");
                return true;
            }
            if(mpeg2StartCode(scode) && mpeg2StartCode(scode2)) // Maybe Mpeg2 ?
            {
                trackType=ADM_TS_MPEG2;
                ADM_warning("Probably Mpeg2\n");
                return true;
            }
            ADM_warning("dont know what it is...\n");
            return false;
        }
        // Is it AC3 ??
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
        // We easily have false positive with mp2...
        MpegAudioInfo minfo,minfo2;
        if( getMpegFrameInfo(ptr,len,&minfo,NULL,&syncoff))
        {
                    if(minfo.bitrate>=128)
                    {
                        ADM_info("Maybe MP2... \n");
                        if( getMpegFrameInfo(ptr2,len2,&minfo2,&minfo,&syncoff))
                        {
                            ADM_warning("\tProbably MP2 : Fq=%d br=%d chan=%d\n", (int)minfo.samplerate,
                                                                        (int)minfo.bitrate,
                                                                        (int)minfo.mode);
                            trackType=ADM_TS_MPEG_AUDIO;
                            return true;
                        }
                    }
        }
    return false;
}
        

//EOF

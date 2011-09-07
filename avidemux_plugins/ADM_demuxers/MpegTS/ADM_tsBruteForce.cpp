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


#define TS_MAX_PACKET_SCAN 500
#define MAX_PID (1<<17)
#define MAX_BUFFER_SIZE (10*1024)

static bool idContent(int pid,tsPacket *ts);

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
        if(map[i]>5) listOfPid.push_back(i);
    }
    delete [] map;
    map=NULL;
    
    for(int i=0;i<listOfPid.size();i++)
    {
        printf("Found stuff in pid=%d\n",listOfPid[i]);
        //  Read PES
        idContent(listOfPid[i],ts);
        // Try to id the packet...
        
    }
    ts->close();
    delete ts;
    ts=NULL;
    delete [] buffer;
    buffer=NULL;
    return result;
}
/**
    \fn idContent
*/
bool idContent(int pid,tsPacket *ts)
{
TS_PESpacket pes(pid);
TS_PESpacket pes2(pid);
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
                        return true;
                    }
                }

        }
        MpegAudioInfo minfo,minfo2;
        if( getMpegFrameInfo(ptr,len,&minfo,NULL,&syncoff))
        {
                    ADM_info("Maybe MP2... \n");
                    if( getMpegFrameInfo(ptr2,len2,&minfo2,&minfo,&syncoff))
                    {
                        ADM_warning("\tProbably MP2 : Fq=%d br=%d chan=%d\n", (int)minfo.samplerate,
                                                                    (int)minfo.bitrate,
                                                                    (int)minfo.mode);
                        return true;
                    }
        }
        
        
    ADM_info("\t Unrecognized.\n");
    return false;
}
        

//EOF

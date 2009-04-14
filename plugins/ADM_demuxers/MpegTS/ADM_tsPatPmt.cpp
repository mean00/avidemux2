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
/**
    \class TrackTypeDescriptor
*/
class TrackTypeDescriptor
{
public:
    int                 type;
    ADM_TS_TRACK_TYPE   trackType;
    const char          *desc;
    static TrackTypeDescriptor *find(int t);
};
TrackTypeDescriptor TrackTypes[]=
{
    {0x002,ADM_TS_MPEG2,     "Mpeg2 Video"},
    {0x003,ADM_TS_MPEG_AUDIO,"Mpeg1 Audio"},
    {0x004,ADM_TS_MPEG_AUDIO,"Mpeg2 Audio"},
    {0x01b,ADM_TS_H264,      "H264 Video"},
    {0x081,ADM_TS_AC3,       "AC3 Audio"},
    {0x006,ADM_TS_UNKNOWN,   "Private Stream"},
    {0xfff,ADM_TS_UNKNOWN,   "Unknown"}   // Last one must be "unknown!"
};
/**
    \fn find
*/
TrackTypeDescriptor *TrackTypeDescriptor::find(int t)
{
    int size=sizeof(TrackTypes)/sizeof(TrackTypeDescriptor);
    for(int i=0;i<size;i++)
    {
        TrackTypeDescriptor *tp=TrackTypes+i;
        if(tp->type==t) return tp;
    }
    return TrackTypes+size-1;
}
/**
    \fn scanForPrograms
    \brief Lookup PAT & PMT to get tracks
*/
bool TS_scanForPrograms(const char *file,uint32_t *nbTracks, ADM_TS_TRACK *tracks)
{
    
    uint32_t len;
    TS_PSIpacketInfo psi;

    vector <uint32_t>listOfPmt;

    tsPacket *t=new tsPacket();
    t->open(file,FP_PROBE);
    // 1 search the pat...
    if(t->getNextPSI(0,&psi)==true)
    {
        uint8_t *r=psi.payload;
        len=psi.payloadSize;
        while(len>=4)
        {
            uint32_t prg=((0x1F&r[0])<<8)+r[1];
            uint32_t pid=((0x1F&r[2])<<8)+r[3];
            r+=4;
            len-=4;
            printf("[TsDemuxer] Pat : Prg:%d Pid: 0x%04x\n",prg,pid);
            if(prg) // if prg==0, it is network Pid, dont need it
                listOfPmt.push_back(pid);
        }
        int size=listOfPmt.size();
        if(size)
        {
            for(int i=0;i<size;i++) // First PMT is PCR lock ?
            {
                printf("<<< PMT : %d/%d>>>\n",i,size);
                uint32_t pid=listOfPmt[i];
                TS_scanPmt(t,pid,NULL,NULL);
            }

        }
    }
    delete t;
    printf("[TS Demuxer] Probed...\n");
    return 0;
}
/**
    \fn TS_scanPmt
    \brief Lookup one PMT and returns content
*/
bool TS_scanPmt(tsPacket *t,uint32_t pid,uint32_t *nbTracks, ADM_TS_TRACK *tracks)
{

    uint32_t len;
    TS_PSIpacketInfo psi;
    uint8_t *r=psi.payload;

    printf("[TsDemuxer] Looking for PMT : 0x%x\n",pid);
    if(t->getNextPSI(pid,&psi)==true)
     {
        len=psi.payloadSize;
        // We should be protected by CRC here
        int packLen=len;
        printf("[TsDemuxer] PCR 0x%x, len=%d\n",(r[0]<<8)+r[1],packLen);
        r+=2;  
        int programInfoLength=(r[0]<<8)+r[1];
        programInfoLength&=0xff;
        r+=2;
        printf("[PMT] PMT :%02x Program Info Len: %d\n",pid,programInfoLength);    
        packLen-=(2+4);
        while(packLen>4)
        {
                int streamType,streamPid,esInfoLength;
                streamType=r[0];
                streamPid=(r[1]<<8)+r[2]&0x1fff;
                esInfoLength=((r[3]<<8)+r[4])&0xfff;
                r+=5;
                packLen-=5;
                packLen-=esInfoLength;
                TrackTypeDescriptor *td=TrackTypeDescriptor::find(streamType);
                printf("[PMT] PMT :%02x StreamType: 0x%x,<<%s>>\n",pid,streamType,td->desc);    
                printf("[PMT]           Pid:        0x%x\n",streamPid);
                printf("[PMT]           Es Info Length: %d (0x%x)\n",esInfoLength,esInfoLength);
                uint8_t *p=r,*pend=r+esInfoLength;
                r+=esInfoLength;
                while(p<pend)
                {
                    uint32_t tag,taglen;
                    tag=p[0];
                    taglen=p[1];
                    printf("[PMT]           Tag :%x len:%d =",tag,taglen);
                    switch(tag) //http://www.coolstf.com/tsreader/descriptors.html
                    {
                        
                        case 0x05: printf("Registration Descriptor :%c%c%c%c",p[2],p[3],p[4],p[5]);break;
                        case 0x0a: printf("Language descriptor :%c%c%c",p[2],p[3],p[4]);break;
                        case 0x11: printf("STD\n");break;
                        case 0x1e: printf("SL descriptor (H264 AAC ????)");break;
                        case 0x45: printf("VBI data\n");break;
                        case 0x52: printf("Stream identifier");break;
                        case 0x56: printf("Teletext");break;
                        case 0x59: printf("DVB subtitles");break;
                        case 0x7b: printf("DTS Descriptor");break;
                        case 0x7a:
                        case 0x6a: printf("AC3 Descriptor");break;
                        
                                                break;
                        
                        default : printf("unknown");break;
                    }
                    printf("\n");
                    p+=2+taglen;
                }
        }
        return true;
     }
    return false;
}
//EOF

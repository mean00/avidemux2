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
    \fn scanForPrograms
*/
bool scanForPrograms(const char *file)
{
    uint8_t buffer[200];
    uint32_t len,current,max;

    vector <uint32_t>listOfPmt;

    tsPacket *t=new tsPacket();
    t->open(file,FP_PROBE);
    // 1 search the pat...
    if(t->getNextPSI(0,buffer,&len,&current,&max)==true)
    {
        uint8_t *r=buffer;
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
        if(listOfPmt.size())
        {
            for(int i=0;i<listOfPmt.size();i++) // First PMT is PCR lock ?
            {
                
                uint32_t pid=listOfPmt[i];
                scanPmt(t,pid);
            }

        }
    }
    delete t;
    printf("[TS Demuxer] Probed...\n");
    return 0;
}
/**
    \fn scanPmt
*/
bool scanPmt(tsPacket *t,uint32_t pid)
{
    uint8_t buffer[200];
    uint32_t len,current,max;
    uint8_t *r=buffer;
    printf("[TsDemuxer] Looking for PMT : %x\n",pid);
    if(t->getNextPSI(pid,buffer,&len,&current,&max)==true)
         {
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
                            r+=esInfoLength;
                            packLen-=5;
                            packLen-=esInfoLength;

                            printf("[PMT] PMT :%02x StreamType: 0x%x\n",pid,streamType);    
                            printf("[PMT] PMT :%02x Pid:        0x%x\n",pid,streamPid);
                            printf("[PMT] PMT :%02x Es Info Length: %d (0x%x)\n",pid,esInfoLength,esInfoLength);

                    }


         }
        return false;
}
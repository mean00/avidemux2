/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
    
      See lavformat/flv[dec/env].c for detail
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
#include "fourcc.h"
#include "avidemutils.h"

ADM_DEMUXER_BEGIN( tsHeader,
                    1,0,0,
                    "ts",
                    "mpeg ts demuxer plugin (c) Mean 2007/2009"
                );

static bool detectTs(const char *file);
static bool checkMarker(uint8_t *buffer, uint32_t bufferSize,uint32_t block);
static bool scanForPrograms(const char *file);
uint8_t   tsIndexer(const char *file);
/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
char index[strlen(fileName)+4];
int count=0;
    printf("[TS Demuxer] Probing...\n");
    if(0 && !detectTs(fileName))
    {
        printf(" [TS Demuxer] Not a ts file\n");
        return false;
    }

   

    sprintf(index,"%s.idx",fileName);
again:    
    if(ADM_fileExist(index)) 
    {
        printf(" [TS Demuxer] There is an index for that file \n");
        FILE *f=fopen(index,"rt");
        char signature[10];
        fread(signature,4,1,f);
        signature[4]=0;
        fclose(f);
        if(!strcmp(signature,"PSD1")) return 50;
        printf("[TSDemuxer] Not a valid index\n");
        return false;
    }
    if(count) return false;
    printf("[TSDemuxer] Creating index..\n");
    count++;
    if(scanForPrograms(fileName)==false) return 0;
    if(true==tsIndexer(fileName)) goto again;
    printf("[TSDemuxer] Failed..\n");
   return 0;
}
#define PROBE_SIZE (1024*1024)
/**
    \fn detectTs
    \brief returns true if the file seems to be mpeg PS

*/
bool detectTs(const char *file)
{
    uint8_t *buffer=new uint8_t [PROBE_SIZE];
    uint32_t bufferSize;
    uint32_t nbPacket,nbMatch=0;

    FILE *f=fopen(file,"rb");
    if(!f) return false;
    bufferSize=fread(buffer,1,PROBE_SIZE,f);
    fclose(f);
    // Do a simple check by checking we have 0x47....0x47 several time in a raw
    if(true==checkMarker(buffer,bufferSize,188))
    {
        printf("[TS Demuxer] 188 bytes packet detected\n");
        return true;
    }
    // Do a simple check by checking we have 0x47....0x47 several time in a raw
    if(true==checkMarker(buffer,bufferSize,192))
    {
        printf("[TS Demuxer] 192 bytes packet detected\n");
        return true;
    }
    printf("[TS Demuxer] Not a TS file\n");
    return false;
}

/**
        \fn checkMarker
        \brief return true if the mpeg TS markers are there separated by block bytes
*/
bool checkMarker(uint8_t *buffer, uint32_t bufferSize,uint32_t block)
{
    uint8_t *end=buffer+bufferSize;
    int syncOk=0;
    int syncKo=0;
    // Search Marker    
    while(buffer<end)
    {
        if(*buffer!=TS_MARKER)
        {
            while(*buffer!=TS_MARKER && buffer<end) 
            {
                buffer++;
            }
            syncKo++;
        }
        if(*buffer!=TS_MARKER) break;
        while(buffer<end && buffer[block]==TS_MARKER)
        {   
                syncOk++,
                buffer+=block;
        }    
        buffer++;
    }
    printf("[Ts Demuxer] Sync ok :%d Sync ko :%d\n",syncOk,syncKo);
    if(!syncOk) return false;
    if(syncOk>5*syncKo) return true;
}
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
        while(len>4)
        {
            uint32_t prg=((0x1F&r[0])<<8)+r[1];
            uint32_t pid=((0x1F&r[2])<<8)+r[3];
            r+=4;
            len-=4;
            printf("[TsDemuxer] Pat : Prg:%d Pid: 0x%04x\n",prg,pid);
            listOfPmt.push_back(pid);
        }
        if(listOfPmt.size())
        {
            for(int i=0;i<listOfPmt.size();i++)
            {
                uint32_t pid=listOfPmt[i];
                 if(t->getNextPSI(pid,buffer,&len,&current,&max)==true)
                 {
                    // We should be protected by CRC here
                    r=buffer;
                    printf("[TsDemuxer] PCR 0x%x\n",(r[0]<<8)+r[1]);
                    r+=2;  
                    int programInfoLength=(r[0]<<8)+r[1];
                            programInfoLength&=0xfff;
                            r+=2;
                            while(programInfoLength)
                            {
                                    printf("[PMT] PMT :%02x StreamType: 0x%x\n",*r++);    
                                    printf("[PMT] PMT :%02x Pid:        0x%x\n",(r[0]<<8)+r[1]);
                                    r+=2;
                                    int esInfoLength=((r[0]<<8)+r[1])&0xfff;
                                    printf("[PMT] PMT :%02x Es Info Length: %d\n",esInfoLength);
                                    r+=esInfoLength;
                                    programInfoLength-=4;
                                    programInfoLength=esInfoLength;

                            }


                 }

            }

        }

    }
    delete t;
    printf("[TS Demuxer] Probed...\n");
    return 0;
}
//EOF

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
#include "ADM_tsPatPmt.h"
ADM_DEMUXER_BEGIN( tsHeader,
                    1,0,0,
                    "ts",
                    "mpeg ts demuxer plugin (c) Mean 2007/2009"
                );

static bool detectTs(const char *file);
static bool checkMarker(uint8_t *buffer, uint32_t bufferSize,uint32_t block);
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
    printf("[TSDemuxer] Analyzing file..\n");
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
//EOF

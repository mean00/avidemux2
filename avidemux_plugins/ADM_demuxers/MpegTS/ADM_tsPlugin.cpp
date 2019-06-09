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
#include "ADM_tsPatPmt.h"

ADM_DEMUXER_BEGIN( tsHeader, 90,
                    1,0,0,
                    "ts",
                    "mpeg ts demuxer plugin (c) Mean 2007/2009"
                );

static bool detectTs(const char *file);
static bool checkMarker(uint8_t *buffer, uint32_t bufferSize,uint32_t block);

/**
    \fn Probe
*/

extern "C"  uint32_t ADM_PLUGIN_EXPORT        probe(uint32_t magic, const char *fileName)
{
    printf("[TS Demuxer] Probing...\n");
    if( !detectTs(fileName))
    {
        printf(" [TS Demuxer] Not a ts file\n");
        return false;
    }
    
    return 50;
}
#define PROBE_SIZE (1024*1024)
/**
    \fn detectTs
    \brief returns true if the file seems to be mpeg PS

*/
bool detectTs(const char *file)
{
    uint32_t bufferSize;
    uint32_t nbPacket,nbMatch=0;

    FILE *f=ADM_fopen(file,"rb");
    if(!f) return false;
    uint8_t *buffer=new uint8_t[PROBE_SIZE];
    bufferSize=fread(buffer,1,PROBE_SIZE,f);
    fclose(f);
    // Do a simple check by checking we have 0x47....0x47 several time in a raw
    if(true==checkMarker(buffer,bufferSize,TS_PACKET_LEN))
    {
        ADM_info("[TS Demuxer] 188 bytes packet detected\n");
        delete [] buffer;
        return true;
    }
    // Do a simple check by checking we have 0x47....0x47 several time in a raw
    if(true==checkMarker(buffer,bufferSize,TS_PACKET_LEN+4))
    {
        ADM_info("[TS Demuxer] 192 bytes packet detected\n");
        delete [] buffer;
        return true;
    }
    ADM_info("[TS Demuxer] Not a TS file\n");
    delete [] buffer;
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
    while(buffer+block<end)
    {
        if(*buffer!=TS_MARKER)
        {
            while(buffer<end && *buffer!=TS_MARKER  ) 
            {
                buffer++;
            }
            syncKo++;
        }
        if(buffer>=end || *buffer!=TS_MARKER) break;
        while(buffer+block<end && buffer[block]==TS_MARKER)
        {   
                syncOk++,
                buffer+=block;
        }    
        buffer++;
    }
    ADM_info("[Ts Demuxer] Sync ok :%d Sync ko :%d\n",syncOk,syncKo);

    if(syncOk>5*syncKo) return true;
    return false;
}
//EOF

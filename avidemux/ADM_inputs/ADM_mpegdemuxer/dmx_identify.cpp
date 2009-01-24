/***************************************************************************
                          PS demuxer
                             -------------------
                
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_assert.h"

#include "dmx_io.h"
#include "dmx_demuxerTS.h"
#include "dmx_demuxerPS.h"
#include "dmx_demuxerEs.h"
#include "dmx_identify.h"

#define MAX_PROBE (5*1024*1024)
#define MIN_DETECT 40
#define H264MIN_DETECT 50

static uint8_t probeTs(fileParser *parser,uint32_t packetSize);
uint8_t probeH264(fileParser *parser);

DMX_TYPE dmxIdentify(const char *name)
{
DMX_TYPE ret=DMX_MPG_UNKNOWN;
uint64_t pos;
uint32_t head1,head2;
uint8_t stream;
FP_TYPE fp=FP_DONT_APPEND;
        uint64_t size;
        uint32_t typeES=0,typePS=0;
        fileParser *parser;
        parser=new fileParser();

        printf("Probing : %s\n",name);
        if(!parser->open(name,&fp))
        {
                goto _fnd;
        }
        // Maybe ASF, MS-DVR file ?
        head1=parser->read32i();
        head2=parser->read32i();
        parser->setpos(0);
        if(head1==0x3026B275 && head2 ==0x8e66CF11)
        {
          delete parser;
          return DMX_MPG_MSDVR;
        }
        // Try to see if it is a TS:
        if(probeTs(parser,TS_PACKET_SIZE))
                {
                        delete parser;
                        printf("Detected as TS file\n");
                        return DMX_MPG_TS;
                }
        parser->setpos(0);
        if(probeTs(parser,TS2_PACKET_SIZE))
                {
                        delete parser;
                        printf("Detected as TS2 file\n");
                        return DMX_MPG_TS2;
                }
        parser->setpos(0);

        while(1)
        {
                parser->getpos(&pos);
                if(pos>MAX_PROBE) goto _nalyze;
                if(!parser->sync(&stream)) goto _nalyze;

#define INSIDE(x,y) (stream>=x && stream<y)
                if(stream==00 || stream==0xb3 || stream==0xb8) 
                {                        
                        typeES++;
                        continue;
                }

                if( INSIDE(0xE0,0xE9)  ) 
                        typePS++;

        }
_nalyze:
                if(typeES>MIN_DETECT)
                {
                        if(typePS>MIN_DETECT)
                        {
                                printf("%s looks like Mpeg PS (%lu/%lu)\n",name,typeES,typePS);
                                ret=DMX_MPG_PS;
                        }
                        else
                        {
                                printf("%s looks like Mpeg ES (%lu/%lu)\n",name,typeES,typePS);
                                ret=DMX_MPG_ES;
                        }
                }
                else
                {
                        // Try TS here
                        printf("Cannot identify %s as mpeg (%lu/%lu)\n",name,typeES,typePS);
                        // Try to see if it is H264 ES
                        if(probeH264(parser))
                        {                         
                            ret=DMX_MPG_H264_ES;
                        }

                }
     
                           
_fnd:
                delete parser;
                return ret;
}
/**
    \fn probeTs
    \brief Try to detect if the stream is TS type with packet size packetSize
*/
uint8_t probeTs(fileParser *parser,uint32_t packetSize)
{
uint64_t size,pos,left;
uint32_t count,discarded;
        // Try to sync on a 0x47 and verify we have several of them in a raw
        size=parser->getSize();
        while(1)
        {
                parser->getpos(&pos);
                if(size-pos<packetSize || pos> 100*1024)
                {
                        return 0;
                }
                left=size-pos;
                count=0;
                discarded=0;
                while(parser->read8i()!=0x47 && left > packetSize)
                        {
                                left--;
                                discarded++;
                                if(discarded>packetSize*3) return 0;
                        }
                if(left<=packetSize) return 0;
                parser->getpos(&pos);

                parser->forward(packetSize-1);
                if(parser->read8i()!=0x47) goto loop;
                parser->forward(packetSize-1);
                if(parser->read8i()!=0x47) goto loop;
                parser->forward(packetSize-1);
                if(parser->read8i()!=0x47) goto loop;
                // It is a TS file:
                return 1;
loop:
                parser->setpos(pos);
        }

        return 1;
}
/**
      \fn probeH264
      \brief Try to identidy H264 streams
*/
uint8_t probeH264(fileParser *parser)
{
  uint32_t nal=0;
  uint8_t ret=0;
  uint64_t size,pos,left;
  uint8_t stream;
        parser->setpos(0);
        while(1)
        {
                parser->getpos(&pos);
                if(pos>MAX_PROBE) goto _nalyze2;
                if(!parser->syncH264(&stream)) goto _nalyze2;

#define INSIDE(x,y) (stream>=x && stream<y)
                if(stream==01 || stream==7 || stream==9) 
                {                        
                        nal++;
                        continue;
                }


        }
_nalyze2:
                if(nal>H264MIN_DETECT)
                {
                        
                        printf("Identified as H264 ES \n");
                        ret=1;
                }
                else
                {
                        // Try TS here
                        printf("Cannot identify as H264 ES (%lu/%lu)\n",nal,H264MIN_DETECT);

                }
     
               return ret;
}            

// EOF

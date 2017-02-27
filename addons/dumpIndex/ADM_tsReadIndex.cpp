/***************************************************************************
        \file ADM_tsReadIndex.cpp
        \brief Read ts index

    copyright            : (C) 2007/2009 by mean

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vector>
#include <string>
using std::vector;
using std::string;

#include "ADM_default.h"
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_string.h"
#include "ADM_vidMisc.h"

#include <math.h>
#define TS_MAX_LINE 10000
#include "ADM_ts.h"

extern uint8_t  mk_hex(uint8_t a, uint8_t b);;
/**
        \fn readIndex
        \brief Read the [video] section of the index file

*/
bool    tsHeader::readIndex(indexFile *index)
{
char buffer[TS_MAX_LINE];
bool firstAudio=true;
        printf("[TsDemuxerer] Reading index\n");
        if(!index->goToSection("Data")) return false;
      
        while(1)
        {
            if(!index->readString(TS_MAX_LINE,(uint8_t *)buffer)) return true;
            if(buffer[0]=='[') return true;
            if(buffer[0]==0xa || buffer[0]==0xd) continue; // blank line
            // Now split the line
            if(!strncmp(buffer,"Video ",6))
            {
                processVideoIndex(buffer+6);
            }
            if(!strncmp(buffer,"Audio ",6))
            {
                if(firstAudio) 
                    firstAudio=false; // Ignore first line
            }
        }
    return true;
}

/**
    \fn processVideoIndex
    \brief process an mpeg index entry from a line from the index file
*/
bool tsHeader::processVideoIndex(char *buffer)
{
            char *head=buffer;
            uint64_t pts,dts,startAt;
            uint32_t offset;
            if(4!=sscanf(head,"at:%" PRIx64":%" PRIx32" Pts:%" PRId64":%" PRId64,&startAt,&offset,&pts,&dts))
            {
                    printf("[TsDemuxerer] cannot read fields in  :%s\n",buffer);
                    return false;
            }
            
            char *start=strstr(buffer," I");
            if(!start) start=strstr(buffer," D");
            if(!start) return true;
            start+=1;
            int count=0;
            uint64_t lastDts=ADM_NO_PTS;
            while(1)
            {
                char *cur=start;
                char type=1;
                char picStruct='F';
                char *next;
                uint32_t len;
                type=*cur;
                if(type==0x0a || type==0x0d || !type) break;
                cur++;
                picStruct=*cur;
                cur++;
                if(*(cur)!=':')
                {
                    printf("[TsDemuxer]  instead of : (%c %x %x):\n",*cur,*(cur-1),*cur);
                }
                cur++;
                next=strstr(start," ");
                int64_t ppts,ddts;
                ADM_assert(3==sscanf(cur,"%" PRIx32":%" PRId64":%" PRId64,&len,&ppts,&ddts));
                
                
                dmxFrame *frame=new dmxFrame;
                if(!count)
                {
                    frame->pts=pts;
                    frame->dts=dts;

                    frame->startAt=startAt;
                    frame->index=offset;

                }else       
                {
                    if(ppts==-1 || pts==-1)
                        frame->pts=ADM_NO_PTS;
                    else
                        frame->pts=pts+ppts;
                    if(ddts==-1 || dts==-1)
                        frame->dts=ADM_NO_PTS;
                    else
                        frame->dts=dts+ddts;

                    frame->startAt=0;
                    frame->index=0;
                }
                switch(type)
                {
                    case 'I': frame->type=1;break;
                    case 'P': frame->type=2;break;
                    case 'B': frame->type=3;break;
                    case 'D': frame->type=4;break;
                    default: ADM_assert(0);
                }
                switch(picStruct)
                {
                        default: ADM_warning("Unknown picture structure %c\n",picStruct);
                        case 'F': ;break;
                        case 'T': ;break;
                        case 'B': ;break;

                }
                frame->len=len;
                
                printf("Frame %c len = %6d dts = %s ",type,len,ADM_us2plain(frame->dts));
                printf(" pts = %s  ",ADM_us2plain(frame->pts));
                if(lastDts!=ADM_NO_PTS && frame->dts!=ADM_NO_PTS)
                {
                    printf("delta dts=%d",(int)(frame->dts-lastDts));
                }
                printf("\n");
                lastDts=frame->dts;
                
                count++;
                if(!next) 
                {
                    break;
                }
                start=next+1;
            }

        return true;
}

/**
        \fn readVideo
        \brief Read the [video] section of the index file

*/

bool    tsHeader::readVideo(indexFile *index)
{
    printf("[TsDemuxerer] Reading Video\n");
    if(!index->readSection("Video")) return false;
    uint32_t w,h,fps,ar;
    
    w=index->getAsUint32("Width");
    h=index->getAsUint32("height");
    fps=index->getAsUint32("Fps");
    char *type=index->getAsString("VideoCodec");



    return true;
}
//EOF

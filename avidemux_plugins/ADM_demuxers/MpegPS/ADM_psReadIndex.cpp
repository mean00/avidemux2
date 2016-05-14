/***************************************************************************
    copyright            : (C) 2007/2009 by mean
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_indexFile.h"
#include "ADM_ps.h"
#include "ADM_string.h"
#include <math.h>
/**
        \fn readIndex
        \brief Read the [video] section of the index file

*/
bool    psHeader::readIndex(indexFile *index)
{
char buffer[2000];
bool firstAudio=true;
        printf("[psDemuxer] Reading index\n");
        if(!index->goToSection("Data")) return false;
      
        while(1)
        {
            if(!index->readString(2000,(uint8_t *)buffer)) return true;
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
                else
                    processAudioIndex(buffer+6);
            }
        }
    return true;
}
/**
    \fn processAudioIndex
    \brief process audio seek points from a line from the index file
*/
bool psHeader::processAudioIndex(char *buffer)
{
    int64_t startAt,dts;
    uint32_t size;
    uint32_t pes;
    char *head,*tail;
    int trackNb=0;
        sscanf(buffer,"bf:%" PRIx64,&startAt);
        head=strstr(buffer," ");
        if(!head) return false;
        head++;
        while((tail=strstr(head," ")))
        {
            if(4!=sscanf(head,"Pes:%" PRIx32":%" PRIx64":%" PRIi32":%" PRId64" ",&pes,&startAt,&size,&dts))
            {
// qfprintf(index,"Pes:%x:%08" PRIx64":%" PRIi32":%PRId64 ",e,s->startAt,s->startSize,s->startDts);
                printf("[PsHeader::processAudioIndex] Reading index %s failed\n",buffer);
            }
            head=tail+1;
            ADM_psAccess *track=listOfAudioTracks[trackNb]->access;
            track->push(startAt,dts,size);

            trackNb++;
            //printf("[%s] => %" PRIx32" Dts:%" PRId64" Size:%" PRId64"\n",buffer,pes,dts,size);
            if(strlen(head)<4) break;
        }
        return true;

}

/**
    \fn processVideoIndex
    \brief process an mpeg index entry from a line from the index file
*/
bool psHeader::processVideoIndex(char *buffer)
{
            char *head=buffer;
            uint64_t pts,dts,startAt;
            uint32_t offset;
            if(4!=sscanf(head,"at:%" PRIx64":%" PRIx32" Pts:%" PRId64":%" PRId64,&startAt,&offset,&pts,&dts))
            {
                    printf("[psDemuxer] cannot read fields in  :%s\n",buffer);
                    return false;
            }
            
            char *start=strstr(buffer," I");
            if(!start) return true;
            start+=1;
            int count=0;
            int64_t gopDts=-1;
            while(1)
            {
                char *cur=start;
                char type=1;
                char picStruct='F';
                char *next;
                uint32_t len;
                int64_t framePts,frameDts;
                type=*cur;
                if(type==0x0a || type==0x0d || !type) break;
                cur++;
                picStruct=*cur;
                cur++;
                if(*(cur)!=':')
                {
                    printf("[psDemux]  instead of : (%c %x %x):\n",*cur,*(cur-1),*cur);
                    break;
                }
                cur++;
                next=strstr(start," ");
                *next=0;
                {
                    string me=string(cur); // pts:dts:length (dec/dec/hex)
                    vector <string> result;
                    ADM_splitString(":",me,result);
                    
                    ADM_assert(1==sscanf(result[2].c_str(),"%" PRIx32,&len));
                    ADM_assert(1==sscanf(result[0].c_str(),"%" PRId64,&framePts));
                    ADM_assert(1==sscanf(result[1].c_str(),"%" PRId64,&frameDts));
                }
                
                dmxFrame *frame=new dmxFrame;
                if(!count) // First item.. PTS & DTS are provided as header
                {
                    frame->pts=pts;
                    frame->dts=dts;
                    gopDts=frame->dts;
                    frame->startAt=startAt;
                    frame->index=offset;

                }else       
                {
                    if(gopDts!=ADM_NO_PTS && frameDts!=ADM_NO_PTS) frame->dts=frameDts+gopDts;
                            else frame->dts=ADM_NO_PTS;
                    if(gopDts!=ADM_NO_PTS && framePts!=ADM_NO_PTS) frame->pts=framePts+gopDts;
                            else frame->pts=ADM_NO_PTS;
                    frame->startAt=0;
                    frame->index=0;
                }
                switch(type)
                {
                    case 'I': frame->type=1;break;
                    case 'P': frame->type=2;break;
                    case 'B': frame->type=3;break;
                    default: ADM_assert(0);
                }
                switch(picStruct)
                {
                        default: ADM_warning("Unknown picture structure %c\n",picStruct);
                        case 'F': frame->pictureType=AVI_FRAME_STRUCTURE;break;
                        case 'T': frame->pictureType=AVI_FIELD_STRUCTURE+AVI_TOP_FIELD;break;
                        case 'B': frame->pictureType=AVI_FIELD_STRUCTURE+AVI_BOTTOM_FIELD;break;
                }
                frame->len=len;
                ListOfFrames.append(frame);
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
bool    psHeader::readVideo(indexFile *index)
{
    printf("[psDemuxer] Reading Video\n");
    if(!index->readSection("Video")) return false;
    uint32_t w,h,fps,ar;
    
    w=index->getAsUint32("Width");
    h=index->getAsUint32("height");
    fps=index->getAsUint32("Fps");

    if(!w || !h || !fps) return false;

    interlaced=index->getAsUint32("Interlaced");
    
    _video_bih.biWidth=_mainaviheader.dwWidth=w ;
    _video_bih.biHeight=_mainaviheader.dwHeight=h;             
    _videostream.dwScale=1000;
    _videostream.dwRate=fps;

    _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"MPEG");

    return true;
}
/**
        \fn readAudio
        \brief Read the [Audio] section of the index file

*/
bool    psHeader::readAudio(indexFile *index,const char *name)
{
    printf("[psDemuxer] Reading Audio\n");
    if(!index->readSection("Audio")) return false;
    uint32_t nbTracks;
    
    nbTracks=index->getAsUint32("Tracks");
    if(!nbTracks)
    {
        printf("[PsDemux] No audio\n");
        return true;
    }
    for(int i=0;i<nbTracks;i++)
    {
        char header[40];
        char body[40];
        uint32_t fq,chan,br,codec,pid;
        sprintf(header,"Track%d.",i);
#define readInt(x,y) {sprintf(body,"%s"#y,header);x=index->getAsUint32(body);printf("%02d:"#y"=%" PRIu32"\n",i,x);}
#define readHex(x,y) {sprintf(body,"%s"#y,header);x=index->getAsHex(body);printf("%02x:"#y"=%" PRIu32"\n",i,x);}
        readInt(fq,fq);
        readInt(br,br);
        readInt(chan,chan);
        readInt(codec,codec);
        readHex(pid,pid);
        WAVHeader hdr;
            hdr.frequency=fq;
            hdr.byterate=br;
            hdr.channels=chan;
            hdr.encoding=codec;
        ADM_psAccess *access=new ADM_psAccess(name,pid,true);
            ADM_psTrackDescriptor *desc=new ADM_psTrackDescriptor;
            desc->stream=NULL;
            desc->access=access;
            memcpy(&(desc->header),&hdr,sizeof(hdr));
            listOfAudioTracks.append(desc);


    }
    return true;
}

/**
    \fn readScrReset
*/
bool    psHeader::readScrReset(indexFile *index)
{
    ADM_info("[psDemuxer] Reading ScrResets\n");
    if(!index->readSection("ScrResets"))
    {
        ADM_info("No ScrResets\n");
        return false;
    }
    uint32_t nbTracks=0;
    
    nbTracks=index->getAsUint32("NbResets");
    if(!nbTracks)
    {
        printf("[PsDemux] No ScrResets\n");
        return false;
    }
    ADM_info("Found %d scrResets\n",(int)nbTracks);
    for(int i=0;i<nbTracks;i++)
    {
        char header[40];
        char body[40];
        sprintf(header,"Reset%d.",i);
        uint64_t position,timeOffset;
#define readInt64(x,y) {sprintf(body,"%s"#y,header);x=index->getAsUint64(body);printf("->%02d:"#y"=%" PRIu64"\n",i,x);}
        
        readInt64(position,position);
        readInt64(timeOffset,timeOffset);
        scrGap gap;
        gap.position=position;
        gap.timeOffset=timeOffset;
        listOfScrGap.append(gap);
    }
  
    return true;
}
//EOF
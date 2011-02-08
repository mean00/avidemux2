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
#include "ADM_ts.h"
#include "ADM_string.h"

#include <math.h>
#define TS_MAX_LINE 10000
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
bool tsHeader::processAudioIndex(char *buffer)
{
    int64_t startAt,dts;
    uint32_t size;
    uint32_t pes;
    char *head,*tail;
    int trackNb=0;
        sscanf(buffer,"bf:%"LLX,&startAt);
        head=strstr(buffer," ");
        if(!head) return false;
        head++;
        while(tail=strstr(head," "))
        {
            if(4!=sscanf(head,"Pes:%"LX":%"LLX":%"LD":%"LLD" ",&pes,&startAt,&size,&dts))
            {
// qfprintf(index,"Pes:%x:%08"LLX":%"LD":%LLD ",e,s->startAt,s->startSize,s->startDts);
                printf("[tsHeader::processAudioIndex] Reading index %s failed\n",buffer);
            }
            head=tail+1;
            ADM_tsAccess *track=listOfAudioTracks[trackNb]->access;
            track->push(startAt,dts,size);

            trackNb++;
            //printf("[%s] => %"LX" Dts:%"LLD" Size:%"LLD"\n",buffer,pes,dts,size);
            if(strlen(head)<4) break;
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
            if(4!=sscanf(head,"at:%"LLX":%"LX" Pts:%"LLD":%"LLD,&startAt,&offset,&pts,&dts))
            {
                    printf("[TsDemuxerer] cannot read fields in  :%s\n",buffer);
                    return false;
            }
            
            char *start=strstr(buffer," I");
            if(!start) start=strstr(buffer," D");
            if(!start) return true;
            start+=1;
            int count=0;
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
                ADM_assert(3==sscanf(cur,"%"LX":%"LLD":%"LLD,&len,&ppts,&ddts));
                
                
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
                        case 'F': frame->pictureType=AVI_FRAME_STRUCTURE;break;
                        case 'T': frame->pictureType=AVI_FIELD_STRUCTURE+AVI_TOP_FIELD;break;
                        case 'B': frame->pictureType=AVI_FIELD_STRUCTURE+AVI_BOTTOM_FIELD;break;

                }
                frame->len=len;
                ListOfFrames.push_back(frame);
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
    if(type)
    {
        printf("[TsIndex] codec :<%s>\n",type);
        if(!strcmp(type,"H264"))
        {
             _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"H264");
        }else
        if(!strcmp(type,"VC1"))
        {
            _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"VC1 ");
            videoNeedEscaping=true;
        }else
        {
            _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"MPEG");
        }
    }else
    {
       _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"MPEG");
    }

    char *extra=index->getAsString("ExtraData");
    if(extra)
    {
            vector<string> result;
            // From is int=nb, hex hex
            ADM_splitString(string(" "), string(extra), result);
            if(result.size())
            {
                int nb=atoi(result[0].c_str());
                printf("[tsDemux] Found %d bytes of video extra data\n",nb);
                if(nb)
                {
                    _videoExtraLen=nb;
                    _videoExtraData=new uint8_t[nb];
                    ADM_assert(nb+1==result.size());
                    for(int i=0;i<nb;i++)
                    {
                        const char *m=result[i+1].c_str();
                        _videoExtraData[i]=mk_hex(m[0],m[1]);
                    }
                }
            }
    }

    videoPid=index->getAsUint32("Pid");
    if(!videoPid)
    {
        printf("[tsDemux] Cannot find Pid\n");
        return false;
    }
    printf("[tsDemux] Video pid is 0x%x %d\n",videoPid,videoPid);
    if(!w || !h || !fps) 
    {
        ADM_error("Width, height or fps1000 missing...\n");
        return false;
    }

    interlaced=index->getAsUint32("Interlaced");
    
    _video_bih.biWidth=_mainaviheader.dwWidth=w ;
    _video_bih.biHeight=_mainaviheader.dwHeight=h;             
    _videostream.dwScale=1000;
    _videostream.dwRate=fps;
    return true;
}
/**
        \fn readAudio
        \brief Read the [Audio] section of the index file

*/
bool    tsHeader::readAudio(indexFile *index,const char *name)
{
    printf("[psDemuxer] Reading Audio\n");
    if(!index->readSection("Audio")) return false;
    uint32_t nbTracks;
    
    nbTracks=index->getAsUint32("Tracks");
    if(!nbTracks)
    {
        printf("[TsDemuxer] No audio\n");
        return true;
    }
    for(int i=0;i<nbTracks;i++)
    {
        char header[40];
        char body[40];
        uint32_t fq,chan,br,codec,pid;
        sprintf(header,"Track%d.",i);
#define readInt(x,y) {sprintf(body,"%s"#y,header);x=index->getAsUint32(body);printf("%02d:"#y"=%"LU"\n",i,x);}
#define readHex(x,y) {sprintf(body,"%s"#y,header);x=index->getAsHex(body);printf("%02x:"#y"=%"LU"\n",i,x);}
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
        bool aacAdts=false;
        if(hdr.encoding==WAV_AAC) aacAdts=true;
        // Look up Track0.extraData line....
        sprintf(body,"Track%d.extraData",i);
        int extraLen=0;
        uint8_t *extraData=NULL;
        char *extra=index->getAsString(body);
        if(extra) // If present, the first part is the size in decimal..
        {
            vector<string> result;
            // From is int=nb, hex hex
            ADM_splitString(string(" "), string(extra), result);
            if(result.size())
            {
                int nb=atoi(result[0].c_str());
                printf("[tsDemux] Found %d bytes of video extra data (%s)\n",nb,result[0].c_str());
                if(nb)
                {
                    extraLen=nb;
                    extraData=new uint8_t[nb];
                    ADM_assert(nb+1==result.size());
                    for(int i=0;i<nb;i++)
                    {
                        const char *m=result[i+1].c_str();
                        extraData[i]=mk_hex(m[0],m[1]);
                    }
                }
            }
        }else
        {
            ADM_info("No extradata (%s)\n",body);
        }
        ADM_tsAccess *access=new ADM_tsAccess(name,pid,true,aacAdts,extraLen,extraData);
        if(extraData) delete [] extraData;
        extraData=NULL;
        ADM_tsTrackDescriptor *desc=new ADM_tsTrackDescriptor;
        desc->stream=NULL;
        desc->access=access;
        memcpy(&(desc->header),&hdr,sizeof(hdr));
        listOfAudioTracks.push_back(desc);
    }
    return true;
}
//EOF

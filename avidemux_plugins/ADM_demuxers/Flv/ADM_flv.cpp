/***************************************************************************
    \file ADM_flv.cpp
    \author (C) 2007 by mean    email                : fixounet@free.fr


Not sure if the timestamp is PTS or DTS (...)

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
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_aacinfo.h"
#include "ADM_flv.h"

#include <math.h>
#include <algorithm>
#include <vector>
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
// Borrowed from lavformt/flv.h
#include "libavformat/flv.h"
#include "libavutil/intfloat.h"
// Borrowed from lavformt/flv.h
uint32_t ADM_UsecFromFps1000(uint32_t fps1000);

#ifdef USE_BUFFERED_IO
uint8_t flvHeader::Skip(uint32_t len)
{
    return parser->forward(len);
}
uint8_t flvHeader::read(uint32_t len, uint8_t *where)
{
    if(len==parser->read32(len,where))
        return 1;
    return 0;
}
uint8_t flvHeader::read8(void)
{
    return parser->read8i();
}
uint32_t flvHeader::read16(void)
{
    uint32_t r=parser->read16i();
    return r;
}
uint32_t flvHeader::read24(void)
{
    uint32_t r=parser->read16i();
    return (r<<8)+(uint32_t)parser->read8i();
}
uint32_t flvHeader::read32(void)
{
    return parser->read32i();
}
#else
/**
    \fn Skip
    \brief Skip some bytes from the file
*/
uint8_t flvHeader::Skip(uint32_t len)
{
  fseeko(_fd,len,SEEK_CUR);
  return 1;
}
/**
    \fn read
    \brief read the given size from file
*/

uint8_t flvHeader::read(uint32_t len, uint8_t *where)
{

    uint32_t got=fread(where,1,len,_fd);
    if(len!=got)
    {
      printf("[FLV] Read error : asked %u, got %u\n",len,got);
      return 0;
    }
    return 1;
}
//__________________________________________________________
uint8_t flvHeader::read8(void)
{
  uint8_t r;
    aprintf("[Read]At 0x%x ",ftello(_fd));
    fread(&r,1,1,_fd);
    aprintf("uint8_t =%d ",r);
    return r;
}
uint32_t flvHeader::read16(void)
{
  uint8_t r[2];
    aprintf("[Read]At 0x%x ",ftello(_fd));
    fread(r,2,1,_fd);
    aprintf("uint16_t =%d ",(r[0]<<8)+r[1]);
    return (r[0]<<8)+r[1];
}
uint32_t flvHeader::read24(void)
{
  uint8_t r[3];
    aprintf("[Read]At 0x%x ",ftello(_fd));
    fread(r,3,1,_fd);
    aprintf("uint24_t =%d ",(r[0]<<16)+(r[1]<<8)+r[2]);
    return (r[0]<<16)+(r[1]<<8)+r[2];
}
uint32_t flvHeader::read32(void)
{
  uint8_t r[4];
    aprintf("[Read]At 0x%x ",ftello(_fd));
    fread(r,4,1,_fd);
    aprintf("uint32_t =%d ",(r[0]<<24)+(r[1]<<16)+(r[2]<<8)+r[3]);
    return (r[0]<<24)+(r[1]<<16)+(r[2]<<8)+r[3];
}
#endif
/**
    \fn     readFlvString
    \brief  read pascal like string
*/
#define FLV_MAX_STRING 255
char *flvHeader::readFlvString(void)
{
static uint8_t stringz[FLV_MAX_STRING+1];
    int size=read16();
    if(size>FLV_MAX_STRING)
    {
        int pre=FLV_MAX_STRING;
        read(pre,stringz);
        ADM_warning("String way too large :%d\n",size);
        mixDump(stringz,pre);
        stringz[0]='X';
        stringz[1]='X';
        stringz[2]=0;
        stringz[FLV_MAX_STRING]=0;
        Skip(size-FLV_MAX_STRING);
        return (char *)stringz;
    }
    read(size,stringz);
    stringz[size]=0;
    return (char *)stringz;
}

static bool timeBaseFromFps1000(uint32_t v, uint32_t *num, uint32_t *den)
{
    if(!v || !num || !den)
        return false;
    switch(v)
    {
        case 23976:
            *num=1001;
            *den=24000;
            break;
        case 29970:
            *num=1001;
            *den=30000;
            break;
        case 59940:
            *num=1001;
            *den=60000;
            break;
        default:
            *num=1000;
            *den=v;
            break;
    }
    return true;
}

/**
    \fn setProperties
    \brief get a couple key/value and use it if needed...
*/
void flvHeader::setProperties(const char *name,float value)
{
    if(!strcmp(name,"framerate"))
    {
        value*=1000.;
        value+=0.49;
        uint32_t v=(uint32_t)value;
        uint32_t num,den;
        if(timeBaseFromFps1000(v,&num,&den))
        {
            _videostream.dwScale=num;
            _videostream.dwRate=den;
            _mainaviheader.dwMicroSecPerFrame=0; // conformance with constant fps will be probed later
        }
        return;
    }
    if(!strcmp(name,"width"))
    {
        metaWidth=(uint32_t)value;
    }
    if(!strcmp(name,"height"))
    {
        metaHeight=(uint32_t)value;
    }
    if(!strcmp(name,"frameWidth"))
    {
        metaFrameWidth=(uint32_t)value;
    }
    if(!strcmp(name,"frameHeight"))
    {
        metaFrameHeight=(uint32_t)value;
    }

}
#define Nest() {for(int xxx=0;xxx<nesting;xxx++) printf("\t");}
/**
    \fn parseOneMeta
*/
bool flvHeader::parseOneMeta(const char *stri,uint64_t endPos,bool &end)
{
    static int nesting=0;
    nesting++;
    int type=read8();
    Nest();
    printf("\n>> type :%d ",type);
#ifdef USE_BUFFERED_IO
    uint64_t pos=0;
    parser->getpos(&pos);
    aprintf("nesting = %d, at %d, : ,end=%d",nesting,pos,endPos);
    switch(type)
    {
        case AMF_DATA_TYPE_NULL:
            parser->setpos(endPos);
            break;
        case AMF_DATA_TYPE_OBJECT_END:
            Nest(); printf("** Object end**.\n");
            parser->getpos(&pos);
            if(pos>=endPos-4)
                parser->setpos(endPos);
            end=true;
            nesting--;
            break;
        case AMF_DATA_TYPE_OBJECT:
        {
            printf("\n");
            bool myEnd=false;
            parser->getpos(&pos);
            while(pos<endPos-4 && myEnd==false)
            {
                Nest();
                parser->getpos(&pos);
                aprintf("Pos = %d, end=%d (object)\n",pos,endPos-4);
                char *o=readFlvString();
                Nest(); printf("\t ** Object**:%s",o);
                if(false==parseOneMeta(o,endPos,myEnd)) return false;
/*
                char objType=read8();
                printf("-->%d",objType);
                if(objType!=10)
                {
                    ADM_warning("type is not strict array\n");
                    goto xxer;
                }
                int count=read32();
                printf(", count=%d\n",count);
                for(int i=0;i<count;i++)
                {
                    if(false==parseOneMeta(endPos)) return false;
                }
                break;
*/
                parser->getpos(&pos);
            }
            break;
        }
        case AMF_DATA_TYPE_ARRAY:
        {
            uint32_t len=read32();
            Nest();printf("\n**[FLV] Array : %" PRIu32" entries**\n",len);
            bool theend;
            parser->getpos(&pos);
            for(int i=0;i<len && pos<endPos-4;i++)
                if(false==parseOneMeta("",endPos,theend)) return false;
            Nest();printf("\n");
            break;
        }
        case AMF_DATA_TYPE_DATE: Skip(8+2);break;
        case AMF_DATA_TYPE_NUMBER:
        {
            float val;
            uint64_t hi,lo;
            hi=read32();lo=read32();
            hi=(hi<<32)+lo;
            val=(float)av_int2double(hi);
            printf("->%f",val);
            setProperties(stri,val);
            break;
        }
        case AMF_DATA_TYPE_STRING:
        {
            int r=read16();
#if 1
            Nest();printf("<");
            for(int i=0;i<r;i++)
            {
                printf("%c",read8());
            }
            printf(">");
#else
            Skip(r);
#endif
        }
            break;
        case AMF_DATA_TYPE_BOOL: read8();break;
        case AMF_DATA_TYPE_MIXEDARRAY:
        {
            read32();
            parser->getpos(&pos);
            while(pos<endPos-4)
            {
                char *o=readFlvString();
                bool theend;
                if(!o) break;
                Nest();printf("** MixedArray:%s **",o);
                if(false==parseOneMeta(o,endPos,theend)) return false;
                parser->getpos(&pos);
            }
            if(read8()!=AMF_END_OF_OBJECT) return false;
         }
             break;
        default: printf("Unknown type=%d\n",type);ADM_assert(0);
    }
    printf("\n");

    nesting--;
    return true;
#else
            aprintf("nesting = %d, at %d, : ,end=%d",nesting,ftello(_fd),endPos);
            switch(type)
            {
                case AMF_DATA_TYPE_NULL:
                                fseek(_fd,endPos,SEEK_SET);
                                break;
                case AMF_DATA_TYPE_OBJECT_END:
                                Nest(); printf("** Object end**.\n");
                                if(ftello(_fd)>=endPos-4)
                                {
                                    fseek(_fd,endPos,SEEK_SET);
                                }
                                end=true;
                                nesting--;
                                break;
                case AMF_DATA_TYPE_OBJECT: 
                {
                        printf("\n");
                        bool myEnd=false;
                        while(ftello(_fd)<endPos-4 && myEnd==false)
                        {
                                Nest();aprintf("Pos = %d, end=%d (object)\n",ftello(_fd),endPos-4);
                                char *o=readFlvString();
                               Nest(); printf("\t ** Object**:%s",o);
                                if(false==parseOneMeta(o,endPos,myEnd)) return false;
/*
                                char objType=read8();
                                printf("-->%d",objType);
                                if(objType!=10) 
                                {
                                        ADM_warning("type is not strict array\n");
                                        goto xxer;
                                }
                                int count=read32();
                                printf(", count=%d\n",count);
                                for(int i=0;i<count;i++)
                                {
                                        if(false==parseOneMeta(endPos)) return false;
                                }
                                break;
*/

                        }
                        break;
                        
                }
                case AMF_DATA_TYPE_ARRAY:
                                    {
                                            uint32_t len=read32();
                                            Nest();printf("\n**[FLV] Array : %" PRIu32" entries**\n",len);
                                            bool theend;
                                            for(int i=0;i<len && ftello(_fd)<endPos-4;i++) 
                                                if(false==parseOneMeta("",endPos,theend)) return false;
                                            Nest();printf("\n");
                                            break;
                                    }
                case AMF_DATA_TYPE_DATE: Skip(8+2);break;
                case AMF_DATA_TYPE_NUMBER:
                                        {
                                            float val;
                                            uint64_t hi,lo;
                                            hi=read32();lo=read32();
                                            hi=(hi<<32)+lo;
                                            val=(float)av_int2double(hi);
                                            printf("->%f",val);
                                            setProperties(stri,val);
                                        }
                                        ;break;
                case AMF_DATA_TYPE_STRING: 
                                                {
                                                int r=read16();
                                                
                                                
                                                #if 1
                                                    Nest();printf("<");
                                                    for(int i=0;i<r;i++)
                                                    {
                                                        printf("%c",read8());
                                                    }
                                                    printf(">");
                                                #else
                                                    Skip(r);
                                                #endif
                                                }
                                                
                                                break;
                case AMF_DATA_TYPE_BOOL: read8();break;
                case AMF_DATA_TYPE_MIXEDARRAY:
                                            {
                                                read32();
                                                 while(ftello(_fd)<endPos-4)
                                                {
                                                            char *o=readFlvString();
                                                            bool theend;
                                                            if(!o) break;
                                                            Nest();printf("** MixedArray:%s **",o);
                                                            if(false==parseOneMeta(o,endPos,theend)) return false;
                                                            
                                                }
                                                if(read8()!=AMF_END_OF_OBJECT) return false;

                                             }
                                            break;
                default : printf("Unknown type=%d\n",type);ADM_assert(0);
            }
            printf("\n");

            nesting--;
            return true;
#endif
xxer:
    nesting --;
    return false;
}
/**
    \fn parseMetaData
    \brief
*/
uint8_t flvHeader::parseMetaData(uint32_t remaining)
{
#ifdef USE_BUFFERED_IO
    uint64_t pos=0;
    parser->getpos(&pos);
    pos+=remaining;
    ADM_assert(!(pos&0xffffffff00000000));
    uint32_t endPos=pos;
#else
    uint32_t endPos=ftello(_fd)+remaining;
#endif
    // Check the first one is onMetaData...
    uint8_t type=read8();
    char *z;
    if(type!=AMF_DATA_TYPE_STRING) // String!
        goto endit;
    z=readFlvString();
    printf("[FlashString] %s\n",z);
    if(z && strncmp(z,"onMetaData",10)) goto endit;
    // Normally the next one is mixed array
#ifdef USE_BUFFERED_IO
    parser->getpos(&pos);
    while(pos<endPos-4)
    {
        bool theend;
        printf("\n----------------------- Parse---------------------\n");
        if(false==parseOneMeta("meta",endPos,theend)) goto endit;
        parser->getpos(&pos);
    }

#else
    while(ftello(_fd)<endPos-4)
    {
        bool theend;
        printf("\n----------------------- Parse---------------------\n");
        if(false==parseOneMeta("meta",endPos,theend)) goto endit;
    }
#endif
endit:
#ifdef USE_BUFFERED_IO
    parser->setpos(endPos);
#else
    fseeko(_fd,endPos,SEEK_SET);
#endif
    updateDimensionWithMeta(videoCodec);
    return 1;
}
/**
    \fn searchMinimum
    \brief returns minimum time in us between 2 video frames

*/
uint32_t flvHeader::searchMinimum(void)
{
    uint32_t delta=0xF000000;
    for(int i=0;i<videoTrack->_nbIndex-1;i++)
    {
        flvIndex *x=&(videoTrack->_index[i]);
        if((x[1].dtsUs-x[0].dtsUs)<delta) delta=x[1].dtsUs-x[0].dtsUs;

    }
    return delta;
}
/**
    \fn extraHeader
    \brief if returns true means we must skip the remainder
*/
bool flvHeader::extraHeader(flvTrak *trk,uint32_t *remain,bool have_cts,int32_t *cts)
{
    int type=read8();
    int r=*remain;
    r--;
    if(have_cts)
    {
        uint32_t c=read24();
         *cts=(c+0xff800000)^0xff800000;
        //printf("Type :%d\n",type);
        r-=3;
    }
    if(!type)
    {  // Grab extra data
        if(trk->extraData) 
        {
            Skip(r);
            r=0;
        }
        else    
        {
            ADM_info("[FLV] found some extradata %" PRIu32"\n",r);
            trk->extraData=new uint8_t[r];
            trk->extraDataLen=r;
            read(r,trk->extraData);
            mixDump(trk->extraData,r);
            r=0;            
        }
        *remain=r;
        return true;
    }
    *remain=r;
    return false;
}
/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t flvHeader::open(const char *name)
{
  uint32_t prevLen, type, size, dts;
  uint64_t pos=0;
  bool firstVideo=true;
  bool tryProbedAvgFps=false;
  int32_t firstCts=0;
  _isvideopresent=0;
  _isaudiopresent=0;
  audioTrack=NULL;
  videoTrack=NULL;
  _videostream.dwRate=0;
  _videostream.dwScale=1000;
  _filename=ADM_strdup(name);
#ifdef USE_BUFFERED_IO
  parser=new fileParser(CACHE_SIZE);
  ADM_assert(parser);
  int append=0;
  if(!parser->open(name,&append))
  {
    ADM_error("[flv] Cannot open %s\n",name);
    return 0;
  }
  uint64_t fileSize=parser->getSize();
#else
  _fd=ADM_fopen(name,"rb");
  if(!_fd)
  {
    printf("[FLV] Cannot open %s\n",name);
    return 0;
  }
  // Get size
  uint64_t fileSize=0;
  fseeko(_fd,0,SEEK_END);
  fileSize=ftello(_fd);
  fseeko(_fd,0,SEEK_SET);
  printf("[FLV] file size :%" PRIu64 " bytes\n",fileSize);
#endif
  // It must begin by F L V 01
  uint8_t four[4];

  read(4,four);
  if(four[0]!='F' || four[1]!='L' || four[2]!='V')
  {
     printf("[FLV] Not a flv file %s\n",name);
    return 0;
  }
  // Next one is flags
  uint32_t flags=read8();
  if(flags & 1) // VIDEO
  {
    _isvideopresent=1;
    printf("[FLV] Video flag\n");
  }else
    {
    GUI_Info_HIG(ADM_LOG_INFO,QT_TRANSLATE_NOOP("flvdemuxer","Warning"),QT_TRANSLATE_NOOP("flvdemuxer","This FLV file says it has no video.\nI will assume it has and try to continue"));
    _isvideopresent=1;
    }
  if(flags & 4) // Audio
  {
    _isaudiopresent=1;
    printf("[FLV] Audio flag\n");
  }


  // Skip header
  uint32_t skip=read32();
#ifdef USE_BUFFERED_IO
  parser->setpos(skip);
  printf("[flv] Skipping %u header bytes\n",skip);
  parser->getpos(&pos);
#else
  fseeko(_fd,skip,SEEK_SET);
  printf("[FLV] Skipping %u header bytes\n",skip);
  pos=ftello(_fd);
#endif
  printf("pos:%" PRIu64 "/%" PRIu64 "\n",pos,fileSize);
  // Create our video index
  videoTrack=new flvTrak(50);
  if(_isaudiopresent)
    audioTrack=new flvTrak(50);
  else
    audioTrack=NULL;
  // Loop
  while(pos<fileSize-14)
  {
    int32_t cts=0;
    uint32_t pts=0xffffffff;
#ifdef USE_BUFFERED_IO
    parser->getpos(&pos);
#else
    pos=ftello(_fd);
#endif
    prevLen=read32();
    type=read8();
    size=read24();
    dts=read24();
    dts|=((uint32_t)read8())<<24;
    read24(); // StreamID, always 0
    aprintf("--------\n");
    aprintf("prevLen=%d\n",(int)prevLen);
    aprintf("type  =%d\n",(int)type);
    aprintf("size  =%d\n",(int)size);
    aprintf("dts   =%d\n",(int)dts);
    if(!size)
    {
#ifdef USE_BUFFERED_IO
        parser->getpos(&pos);
#endif
        continue;
    }
    uint32_t remaining=size;
    //printf("[FLV] At %08" PRIu64 " found type %x size %u pts%u\n",pos,type,size,dts);
    switch(type)
    {
      case FLV_TAG_TYPE_AUDIO:
          {
            aprintf("** Audio **\n");
            if(!_isaudiopresent)
            {
                audioTrack=new flvTrak(50);
                _isaudiopresent=1; /* Damn  lying headers...*/
            };
            uint8_t flags=read8();
            int of=1+4+3+3+1+4;
            remaining--;
            int format=flags>>4;
            int fq=(flags>>2)&3;
            int bps=(flags>>1) & 1;
            int channel=(flags) & 1;
            if(!audioTrack->_nbIndex) // first frame..
            {
               setAudioHeader(format,fq,bps,channel);
            }
            if(format==10)
            {
                if(extraHeader(audioTrack,&remaining,false,&cts)) continue;
            }
#ifdef USE_BUFFERED_IO
            if(remaining)
            {
                parser->getpos(&pos);
                insertAudio(pos,remaining,dts);
            }
#else
            if(remaining)
                insertAudio(ftello(_fd),remaining,dts);
#endif
          }
          break;
      case FLV_TAG_TYPE_META:
                aprintf("** Meta **\n");
                parseMetaData(remaining);
                remaining=0;
                break;
      case FLV_TAG_TYPE_VIDEO:
          {
            int of=1+4+3+3+1+4;
            aprintf("** Video **\n");
            uint8_t flags=read8();
            remaining--;
            int frameType=flags>>4;

            videoCodec=(flags)&0xf;

            if(videoCodec==FLV_CODECID_VP6 || videoCodec==FLV_CODECID_VP6A)
            {
              read8(); // 1 byte of extraData
              remaining--;
              of++;
            }
            
            if(firstVideo==true) // first frame..
            {
                if(!setVideoHeader(videoCodec,&remaining)) return 0;
                firstVideo=false;
            }
            if(videoCodec==FLV_CODECID_H264)
            {
                if(true==extraHeader(videoTrack,&remaining,true,&cts))
                {
                    ADM_info("Retrieving H.264 info...\n");
                    nalsize=ADM_getNalSizeH264(videoTrack->extraData,videoTrack->extraDataLen);
                    spsinfo=new ADM_SPSInfo;
                    if(false==extractSPSInfo_mp4Header(videoTrack->extraData,videoTrack->extraDataLen,spsinfo))
                    {
                        ADM_warning("Cannot decode SPS\n");
                        delete spsinfo;
                        spsinfo=NULL;
                    }
                    continue;
                }
                if(!videoTrack->_nbIndex) firstCts=cts;
                if(!bFramesPresent && firstCts!=cts)
                    bFramesPresent=true;
                int64_t sum=cts+dts;
                if(sum<0) pts=0xffffffff;
                    else pts=dts+(int32_t)cts;

            }
#ifdef USE_BUFFERED_IO
            if(remaining)
            {
                bool check=bFramesPresent;
                parser->getpos(&pos);
                insertVideo(pos,remaining,frameType,dts,pts);
                if(!ptsInvalid && bFramesPresent!=check)
                {
                    ADM_warning("Invalid PTS detected at frame %d\n",(int)videoTrack->_nbIndex-1);
                    ptsInvalid=true;
                }
                pos+=remaining;
                remaining=0;
                parser->setpos(pos);
            }
#else
            if(remaining)
            {
                bool check=bFramesPresent;
                pos=ftello(_fd);
                insertVideo(pos,remaining,frameType,dts,pts);
                if(!ptsInvalid && bFramesPresent!=check)
                {
                    ADM_warning("Invalid PTS detected at frame %d\n",(int)videoTrack->_nbIndex-1);
                    ptsInvalid=true;
                }
                pos+=remaining;
                remaining=0;
                fseeko(_fd,pos,SEEK_SET);
            }
#endif
          }
           break;
      default: printf("[FLV]At 0x%" PRIx64 ", unhandled type %u\n",pos,type);
    }
    Skip(remaining);
  } // while

  // Udpate frame count etc..
  ADM_info("[FLV] Found %u frames\n",videoTrack->_nbIndex);
  
    if(videoCodec==FLV_CODECID_H264 && spsinfo && spsinfo->width && spsinfo->height)
    {
        ADM_info("Setting width and height to values obtained from codec\n");
        ADM_info("W %d\n",spsinfo->width);
        ADM_info("H %d\n",spsinfo->height);
        metaWidth=spsinfo->width;
        metaHeight=spsinfo->height;
        updateDimensionWithMeta(FLV_CODECID_H264);
    }

   _videostream.dwLength= _mainaviheader.dwTotalFrames=videoTrack->_nbIndex;

    // Can we force a constant frame rate based on metadata?
    uint64_t delay=(firstCts>0)? 1000LL*firstCts : 0;
    if(false==enforceConstantFps(_videostream.dwScale,_videostream.dwRate,delay,bFramesPresent))
    {
        ADM_info("Cannot force constant frame rate for timebase %u / %u\n",_videostream.dwScale,_videostream.dwRate);
        tryProbedAvgFps=true;
    }
   // Compute average fps
    float f=_videostream.dwLength;
    uint64_t duration=videoTrack->_index[videoTrack->_nbIndex-1].dtsUs;
    duration+=(uint64_t)((double)duration/(double)f+0.49);

    if(duration)
          f=1000.*1000.*1000.*f/duration;
     else  f=25000;
    // If it was available from the metadata, use the one from metadata
    if(tryProbedAvgFps)
    {
        float d=searchMinimum();
        printf("[FLV] minimum delta :%d\n",(uint32_t)d);
        d=1/d;
        d*=1000*1000*1000;

        uint32_t avg=(uint32_t)floor(f);
        uint32_t max=(uint32_t)floor(d);
        if(max<2) max=2; // 500 fps max
        printf("[FLV] Avg fps :%d, max fps :%d\n",avg,max);
        // Can we use the probed average fps for timebase?
        bool skip=false;
        uint32_t num,den;
        if(timeBaseFromFps1000(avg,&num,&den))
        {
            skip = (num==_videostream.dwScale) && (den==_videostream.dwRate);
            if(!skip)
            {
                _videostream.dwScale=num;
                _videostream.dwRate=den;
                _mainaviheader.dwMicroSecPerFrame=0; // assume a perfectly regular stream
            }
        }
        if(skip || false==enforceConstantFps(_videostream.dwScale,_videostream.dwRate,delay,bFramesPresent))
        {
            if(skip)
                ADM_info("Average fps matches metadata, skipping the check.\n");
            else
                ADM_info("Cannot force CFR based on avg fps for timebase %u / %u\n",_videostream.dwScale,_videostream.dwRate);
            // Can we use at least a timebase derived from max fps?
            if(timeBaseFromFps1000(max,&num,&den))
            {
                _videostream.dwScale=num;
                _videostream.dwRate=den;
            }
            _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(avg); // should be safe now
            if(false==checkTimeBase(_videostream.dwScale,_videostream.dwRate))
            {
                ADM_info("Cannot use timebase %u / %u from max fps, falling back to 1 / 1000 equivalent.\n",_videostream.dwScale,_videostream.dwRate);
                {
                    _videostream.dwRate=16000; // lavf doesn't like low clocks like e.g. 1000 // FIXME
                    _videostream.dwScale=16;
                }
            }
        }
    }

    ADM_info("[FLV] Duration: %" PRIu64" ms, time base: %u / %u\n",duration/1000,
        _videostream.dwScale,_videostream.dwRate);

   //
    _videostream.fccType=fourCC::get((uint8_t *)"vids");
    _video_bih.biBitCount=24;
    _videostream.dwInitialFrames= 0;
    _videostream.dwStart= 0;
    videoTrack->_index[0].flags &= ~AVI_B_FRAME;
    videoTrack->_index[0].flags |= AVI_KEY_FRAME;

    // if it is AAC and we have extradata...
    if(_isaudiopresent && wavHeader.encoding && audioTrack->extraDataLen>=2)
    {
        AacAudioInfo info;
        // Check frequency..
        if(ADM_getAacInfoFromConfig(audioTrack->extraDataLen,audioTrack->extraData,info))
        {
            ADM_info("AAC detected with fq=%d, sbr=%d\n",info.frequency,info.sbr);
            wavHeader.frequency=info.frequency;
        }
    }
    
    // audio track
    if(_isaudiopresent)
    {
        _access=new ADM_flvAccess(name,audioTrack);
        _audioStream=ADM_audioCreateStream(&wavHeader,_access);
    }
    else
    {
        _audioStream = NULL;
       _access=NULL;
    }
#ifdef USE_BUFFERED_IO
    // Too big cache may be detrimental for keyframe-based navigation, reduce cache size
    parser->setBufferSize(DMX_BUFFER);
#endif
  printf("[FLV]FLV successfully read\n");

  return 1;
}
/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t flvHeader::getVideoDuration(void)
{
    uint64_t pts=videoTrack->_index[videoTrack->_nbIndex-1].ptsUs;
    if(pts==ADM_NO_PTS) pts=videoTrack->_index[videoTrack->_nbIndex-1].dtsUs;
    pts+=frameToUs(1);
    return pts;
}
/**
        \fn getVideoTrackSize
*/
uint64_t flvHeader::getVideoTrackSize(void)
{
    return videoTrack->_sizeInBytes;
}

/**
      \fn setVideoHeader
      \brief Set codec and stuff
*/
bool flvHeader::updateDimensionWithMeta(uint32_t codec)
{
    if(codec==0xFFFF) return false;
    ADM_info("We got metadata : %d x %d\n",(int)metaWidth,(int)metaHeight,(int)codec);
    if(metaFrameWidth)  metaWidth=metaFrameWidth;
    if(metaFrameHeight) metaHeight=metaFrameHeight;
    if( metaWidth && metaHeight )
    {
        switch(codec)
        {
            case FLV_CODECID_VP6A:
            case FLV_CODECID_H264:
            case FLV_CODECID_VP6:

                    _video_bih.biHeight=_mainaviheader.dwHeight=metaHeight ;
                    _video_bih.biWidth=_mainaviheader.dwWidth=metaWidth;
                    break;
            default:break;
        }
    }
    return true;
}
/**
    \fn setVideoHeader
*/
uint8_t flvHeader::setVideoHeader(uint8_t codec,uint32_t *remaining)
{
    printf("[FLV] Video Codec:%u\n",codec);
     
    _video_bih.biWidth=_mainaviheader.dwWidth=320;
    _video_bih.biHeight=_mainaviheader.dwHeight=240;
#define MKFLV(x,t) case FLV_CODECID_##x :  _videostream.fccHandler=_video_bih.biCompression=\
                            fourCC::get((uint8_t *)#t);break;
    switch(codec)
    {
        MKFLV(H264,H264);
        MKFLV(H263,FLV1);
        MKFLV(VP6,VP6F);
        MKFLV(VP6A,VP6A);
        default : _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"XXX");break;
    }
    updateDimensionWithMeta(codec);

    if(codec==FLV_CODECID_H263 && *remaining)
    {
#ifdef USE_BUFFERED_IO
      uint64_t pos=0;
      parser->getpos(&pos);
#else
      uint32_t pos=ftello(_fd);
#endif
      uint32_t len=*remaining,width,height;
      uint8_t *buffer=new uint8_t[len];
      read(len,buffer);
#ifdef USE_BUFFERED_IO
      parser->setpos(pos);
#else
      fseeko(_fd,pos,SEEK_SET);
#endif
       /* Decode header, from h263dec.c / lavcodec*/
      if(extractH263FLVInfo(buffer,len,&width,&height))
      {
         _video_bih.biHeight=_mainaviheader.dwHeight=height;
         _video_bih.biWidth=_mainaviheader.dwWidth=width;
      }
      delete [] buffer;
    }
   return 1;
}
/**
      \fn setAudioHeader
      \brief Build WavHeader from info

*/
uint8_t   flvHeader::setAudioHeader(uint32_t format,uint32_t fq,uint32_t bps,uint32_t channels)
{
  switch(fq)
  {
    case 3: wavHeader.frequency=44100;break;
    case 2: wavHeader.frequency=22050;break;
    case 1: wavHeader.frequency=11025;break;
    case 0:
          if(format==5) wavHeader.frequency=8000;
          else wavHeader.frequency=5512;
          break;
    default: printf("[FLV]Unknown frequency:%u\n",fq);
  }
  switch(format)
  {
    case 6: wavHeader.encoding=WAV_NELLYMOSER;break;
    case 2: wavHeader.encoding=WAV_MP3;break;
    case 3: wavHeader.encoding=WAV_PCM;break;
    case 0: wavHeader.encoding=WAV_LPCM;break;
    case 1: wavHeader.encoding=WAV_MSADPCM;break;
    case 10:wavHeader.encoding=WAV_AAC;break;
    default:
          printf("[FLV]Unsupported audio codec:%u\n",format);
  }
  switch(channels)
  {
    case 1: wavHeader.channels=2;break;
    case 0: wavHeader.channels=1;break;
        default:
          printf("[FLV]Unsupported channel mode :%u\n",channels);
  }
  switch(bps)
  {
    case 1: wavHeader.bitspersample=16;break;
    case 0: wavHeader.bitspersample=8;break;
        default:
          printf("[FLV]Unsupported bps mode :%u\n",bps);
  }
  wavHeader.byterate=(64000)/8; // 64 kbits default
  return 1;
}
/**
      \fn insertVideo
      \brief add a frame to the index, grow the index if needed
*/
uint8_t flvHeader::insertVideo(uint64_t pos,uint32_t size,uint32_t frameType,uint32_t dts,uint32_t pts)
{
    videoTrack->grow();
    flvIndex *x=&(videoTrack->_index[videoTrack->_nbIndex]);
    x->size=size;
    x->pos=pos;
    x->dtsUs=dts*1000LL;
    if(pts==0xffffffff) x->ptsUs=ADM_NO_PTS;
        else
      x->ptsUs=pts*1000LL;

    videoTrack->_nbIndex++;
    videoTrack->_sizeInBytes += size;

    if(videoCodec==FLV_CODECID_H264 && nalsize && spsinfo)
    {
        uint8_t *buffer=new uint8_t[size];
        if(read(size,buffer))
        { // Keep fingers crossed that we don't encounter inband SPS changing midstream.
            uint32_t flags=0;
            if(extractH264FrameType(buffer,size,nalsize,&flags,NULL,spsinfo))
            {
                if(!!(flags & AVI_KEY_FRAME) != (frameType==1))
                    ADM_warning("Container and codec disagree about frame %u: %s says keyframe.\n",
                        videoTrack->_nbIndex,(flags & AVI_KEY_FRAME)? "codec" : "container");
                if(flags & AVI_B_FRAME)
                    bFramesPresent=true;
                x->flags=flags;
                return 1;
            }
        }
    }
    if(frameType==1)
    {
        x->flags=AVI_KEY_FRAME;
    }
    else
    {
          x->flags=0;
    }
    return 1;
}
/**
      \fn enforceConstantFps
      \brief Check whether we can force CFR and update video index if necessary
*/
bool flvHeader::enforceConstantFps(uint32_t scale, uint32_t rate, uint64_t delay, bool reorder)
{
    if(!_videostream.dwRate)
        return false;

    double d=_videostream.dwScale;
    d*=1000.*1000;
    d/=_videostream.dwRate*2;
    d+=0.49;
    const int64_t threshold=(int64_t)d; // half of frame increment, as generous as possible
    const uint32_t nbFrames=videoTrack->_nbIndex;
    uint32_t i;

    for(i=0; i<nbFrames; i++)
    {
        d=i;
        d*=_videostream.dwScale;
        d*=1000.;
        d/=_videostream.dwRate;
        d*=1000.;
        d+=0.49;
        uint64_t expected=(uint64_t)d;
        flvIndex *x=&(videoTrack->_index[i]);
        if(x->dtsUs!=ADM_NO_PTS)
        {
            int64_t delta=x->dtsUs-expected;
            //printf("Frame %u dts delta: %" PRId64" us\n",i,delta);
            if(delta>threshold || -delta>threshold)
            {
                ADM_warning("Delta %" PRId64" for frame %u exceeds threshold.\n",delta,i);
                return false;
            }
        }
    }
    ADM_info("Forcing constant frame rate...\n");
    // Force constant frame rate
    for(i=0; i<nbFrames; i++)
    {
        d=i;
        d*=_videostream.dwScale;
        d*=1000.*1000;
        d/=_videostream.dwRate;
        d+=0.49;
        videoTrack->_index[i].dtsUs=(uint64_t)d;
    }
    // round up delay to a multiple of timebase
    if(delay)
    {
        uint64_t num=_videostream.dwScale;
        delay+=num-1;
        delay/=num;
        delay*=num;
    }
    if(reorder)
    {
        uint32_t i;
        std::vector <uint32_t> DisplayOrder;
        std::vector <uint64_t> sortedPts;
        for(i=0; i<nbFrames; i++)
        {
            flvIndex *x=&(videoTrack->_index[i]);
            if(x->ptsUs==ADM_NO_PTS) continue;
            sortedPts.push_back(x->ptsUs);
        }
        std::sort(sortedPts.begin(),sortedPts.end());
        for(i=0; i<nbFrames; i++)
        {
            uint32_t nbInvalid=0;
            flvIndex *x=&(videoTrack->_index[i]);
            if(x->ptsUs==ADM_NO_PTS)
            {
                DisplayOrder.push_back(i);
                nbInvalid++;
                continue;
            }
            uint32_t base=0;
            if(i>32) base=i-32; // max 16 ref * 2 fields
            for(uint32_t j=base;j<sortedPts.size();j++)
            {
                if(x->ptsUs!=sortedPts.at(j))
                    continue;
                uint32_t k=nbInvalid+j;
                ADM_assert(k<nbFrames);
                DisplayOrder.push_back(k);
                //printf("%u --> %u\n",i,k);
                break;
            }
        }
        for(i=0; i<nbFrames; i++)
        {
            if(i+1>=DisplayOrder.size())
                break;
            uint32_t j=DisplayOrder.at(i);
            flvIndex *x=&(videoTrack->_index[i]);
            if(x->ptsUs==ADM_NO_PTS) continue;
            x->ptsUs=videoTrack->_index[j].dtsUs+delay;
        }
    }else
    {
        for(i=0; i<nbFrames; i++)
        {
            flvIndex *x=&(videoTrack->_index[i]);
            x->ptsUs=x->dtsUs+delay;
        }
    }
    // verify that the delay is sufficient
    uint64_t delay2=0;
    for(i=0; i<nbFrames; i++)
    {
        flvIndex *x=&(videoTrack->_index[i]);
        if(x->ptsUs==ADM_NO_PTS || x->dtsUs==ADM_NO_PTS)
            continue;
        if(x->ptsUs+delay2<x->dtsUs)
            delay2+=x->dtsUs-x->ptsUs;
    }
    if(delay2)
    {
        ADM_warning("Original PTS delay is insufficient, adding %" PRIu64" us.\n",delay2);
        for(i=0; i<nbFrames; i++)
            videoTrack->_index[i].ptsUs+=delay2;
    }
    return true;
}

/**
      \fn checkTimeBase
      \brief Check whether all dts and pts can be expressed in the given timebase
*/
bool flvHeader::checkTimeBase(uint32_t scale, uint32_t rate)
{
    if(!scale || rate<1000)
        return false;

    uint32_t i;
    const uint32_t nbFrames=videoTrack->_nbIndex;
    scale*=1000;
    // check dts first
    for(i=0; i<nbFrames; i++)
    {
        flvIndex *x=&(videoTrack->_index[i]);
        if(x->dtsUs==ADM_NO_PTS || x->dtsUs<1000)
            continue;
        uint64_t low=x->dtsUs-1000; // -1 ms
        uint64_t high=x->dtsUs+1000; // +1 ms
        double f=low;
        f*=rate;
        f/=scale;
        f+=0.49;
        low=(uint64_t)f; // 1000 * scaled time
        f=high;
        f*=rate;
        f/=scale;
        f+=0.49;
        high=(uint64_t)f;
        if(high%1000>100 || low%1000<900)
        {
            ADM_warning("Frame %d dts is not a multiple of timebase.\n",i);
            return false;
        }
    }
    // now get pts delay
#define HIGH_POINT 0xFFFFFFF0 // an arbitrary high value
    uint64_t delay=HIGH_POINT;
    for(i=0; i<nbFrames; i++)
    {
        flvIndex *x=&(videoTrack->_index[i]);
        if(x->ptsUs==ADM_NO_PTS)
            continue;
        if(x->ptsUs<delay) delay=x->ptsUs;
        if(!delay) break;
    }
    if(delay==HIGH_POINT)
        return true; // no valid pts
    ADM_info("Probed PTS delay: %" PRIu64" us.\n",delay);
    // check pts
    for(i=0; i<nbFrames; i++)
    {
        flvIndex *x=&(videoTrack->_index[i]);
        if(x->ptsUs==ADM_NO_PTS || x->ptsUs-delay<1000)
            continue;
        uint64_t low=x->ptsUs-delay-1000; // -1 ms
        uint64_t high=x->ptsUs-delay+1000; // +1 ms
        double f=low;
        f*=rate;
        f/=scale;
        f+=0.49;
        low=(uint64_t)f; // 1000 * scaled time
        f=high;
        f*=rate;
        f/=scale;
        f+=0.49;
        high=(uint64_t)f;
        if(high%1000>100 || low%1000<900)
        {
            ADM_warning("Frame %d pts is not a multiple of timebase.\n",i);
            return false;
        }
    }
    return true;
}

/**
      \fn insertAudio
      \brief add a frame to the index, grow the index if needed
*/
uint8_t flvHeader::insertAudio(uint64_t pos,uint32_t size,uint32_t pts)
{
    audioTrack->grow();
    flvIndex *x=&(audioTrack->_index[audioTrack->_nbIndex]);
    x->size=size;
    x->pos=pos;
    x->dtsUs=pts*1000LL;
    x->flags=0;
    audioTrack->_nbIndex++;
    return 1;
}



/**
    \fn getAudioInfo
    \brief returns wav header for stream i (=0)
*/
WAVHeader *flvHeader::getAudioInfo(uint32_t i )
{
  if(_isaudiopresent)
    return &wavHeader;
  else
      return NULL;
}
/**
   \fn getAudioStream
*/

uint8_t   flvHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
  if(_isaudiopresent)
  {
    *audio=_audioStream;
    return 1;
  }
  *audio=NULL;
  return 0;
}
/**
    \fn getNbAudioStreams

*/
uint8_t   flvHeader::getNbAudioStreams(void)
{
 if(_isaudiopresent)
  {

    return 1;
  }
  return 0;
}
/*
    __________________________________________________________
*/

void flvHeader::Dump(void)
{

}
/**
    \fn close
    \brief cleanup
*/

uint8_t flvHeader::close(void)
{
  if(_filename) ADM_dealloc(_filename);
  if(videoTrack) 
  {
        if(videoTrack->extraData) delete [] videoTrack->extraData;
        delete videoTrack;
  }
  if(audioTrack) 
  {
     if(audioTrack->extraData) delete [] audioTrack->extraData;
    delete audioTrack;
  }
#ifdef USE_BUFFERED_IO
  if(parser)
  {
      delete parser;
      parser=NULL;
  }
#else
  if(_fd) fclose(_fd);
  _fd=NULL;
#endif
  if(_audioStream) delete _audioStream;
  if(_access) delete _access;
  if(spsinfo) delete spsinfo;

  _filename=NULL;
  videoTrack=NULL;
  audioTrack=NULL;
  _audioStream=NULL;
  _access=NULL;
  spsinfo=NULL;
  return 1;
}
/**
    \fn flvHeader
    \brief constructor
*/

 flvHeader::flvHeader( void ) : vidHeader()
{
    videoCodec=0xFFFF;
#ifdef USE_BUFFERED_IO
    parser=NULL;
#else
    _fd=NULL;
#endif
    _filename=NULL;
    videoTrack=NULL;
    audioTrack=NULL;
    _audioStream=NULL;
    _access=NULL;
    memset(&wavHeader,0,sizeof(wavHeader));
    metaWidth=0;
    metaHeight=0;
    metaFps1000=0;
    metaFrameWidth=0;
    metaFrameHeight=0;
    ptsInvalid=false;
    bFramesPresent=false;
    nalsize=0;
    spsinfo=NULL;
}
/**
    \fn flvHeader
    \brief destructor
*/

 flvHeader::~flvHeader(  )
{
  close();
}


/**
    \fn setFlag
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

  uint8_t  flvHeader::setFlag(uint32_t frame,uint32_t flags)
{
    if(frame>=videoTrack->_nbIndex)
    {
        printf("[FLV] Setflags out of boud %u/%u\n",frame,videoTrack->_nbIndex);
        return 0;
    }
    videoTrack->_index[frame].flags=flags;
    return 1;
}
/**
    \fn getFlags
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint32_t flvHeader::getFlags(uint32_t frame,uint32_t *flags)
{
    if(frame>=videoTrack->_nbIndex)
    {
        printf("[FLV] Getflags out of boud %u/%u\n",frame,videoTrack->_nbIndex);
        return 0;
    }

    *flags=videoTrack->_index[frame].flags;
    return  1;
}

/**
    \fn getTime
    \brief Returns timestamp in us of frame "frame" (PTS)
*/
uint64_t flvHeader::getTime(uint32_t frame)
{
     if(frame>=videoTrack->_nbIndex) return ADM_NO_PTS;
     flvIndex *idx=&(videoTrack->_index[frame]);
     return idx->ptsUs;
}
/**
        \fn getFrame
*/

uint8_t  flvHeader::getFrame(uint32_t frame,ADMCompressedImage *img)
{
     if(frame>=videoTrack->_nbIndex) return 0;
     flvIndex *idx=&(videoTrack->_index[frame]);
#ifdef USE_BUFFERED_IO
     parser->setpos(idx->pos);
     if(!read(idx->size,img->data))
         return 0;
#else
     fseeko(_fd,idx->pos,SEEK_SET);
     fread(img->data,idx->size,1,_fd);
#endif
     img->dataLength=idx->size;
     img->flags=idx->flags;
     //img->demuxerDts=ADM_COMPRESSED_NO_PTS;
    // For flash assume PTS=DTS (???)
     img->demuxerDts=idx->dtsUs;;
     img->demuxerPts=idx->ptsUs;;
     return 1;
}
/**
        \fn getExtraHeaderData
*/
uint8_t  flvHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
        if(videoTrack)
        {
                *len=videoTrack->extraDataLen;
                *data=videoTrack->extraData;
                return 1;
        }
        *len=0;
        *data=NULL;
        return 1;
}

/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t flvHeader::getFrameSize (uint32_t frame, uint32_t * size)
{
   if(frame>=videoTrack->_nbIndex)
    {
        printf("[FLV] getFrameSize out of boud %u/%u\n",frame,videoTrack->_nbIndex);
        return 0;
    }
  *size = videoTrack->_index[frame].size;
  return 1;
}
//!!

/**
    \fn getPtsDts
*/
bool    flvHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{

    if(frame>=videoTrack->_nbIndex)
    {
      printf("[MKV] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,videoTrack->_nbIndex);
      return 0;
    }

     flvIndex *idx=&(videoTrack->_index[frame]);
    
    *dts=idx->dtsUs; // FIXME
    *pts=idx->ptsUs;
    return true;
}
/**
        \fn setPtsDts
*/
bool    flvHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    if(frame>=videoTrack->_nbIndex)
    {
      printf("[MKV] Frame %" PRIu32" exceeds # of frames %" PRIu32"\n",frame,videoTrack->_nbIndex);
      return 0;
    }

     flvIndex *idx=&(videoTrack->_index[frame]);
    
    idx->dtsUs=dts; // FIXME
    idx->ptsUs=pts;
    //*pts=idx->timeCodeUs; // FIXME PTS=DTS ??
    return true;
}

//EOF

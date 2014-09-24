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
#include "ADM_videoInfoExtractor.h"

#include "ADM_flv.h"

#include <math.h>
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
// Borrowed from lavformt/flv.h
#include "libavformat/flv.h"
// Borrowed from lavformt/flv.h
uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
extern uint8_t extractH263FLVInfo(uint8_t *buffer,uint32_t len,uint32_t *w,uint32_t *h);

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
extern "C" {
double av_int2dbl(int64_t v);
}

/**
    \fn setProperties
    \brief get a couple key/value and use it if needed...
*/
void flvHeader::setProperties(const char *name,float value)
{
    if(!strcmp(name,"framerate"))
    {
        _videostream.dwRate=(uint32_t)(value*1000);
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
            aprintf("nesting = %d, at %d, : ,end=%d",nesting,ftello(_fd),endPos);
            switch(type)
            {
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
                                            Nest();printf("\n**[FLV] Array : %"PRIu32" entries**\n",len);
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
                                            val=(float)av_int2dbl(hi);
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
                                                            Skip(r);}
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
    uint32_t endPos=ftello(_fd)+remaining;
    // Check the first one is onMetaData...
    uint8_t type=read8();
    char *z;
    if(type!=AMF_DATA_TYPE_STRING) // String!
        goto endit;
    z=readFlvString();
    printf("[FlashString] %s\n",z);
    if(z && strncmp(z,"onMetaData",10)) goto endit;
    // Normally the next one is mixed array
    while(ftello(_fd)<endPos-4)
    {
        bool theend;
        printf("\n----------------------- Parse---------------------\n");
        if(false==parseOneMeta("meta",endPos,theend)) goto endit;
    }

endit:
    fseeko(_fd,endPos,SEEK_SET);
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
            ADM_info("[FLV] found some extradata %"PRIu32"\n",r);
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
  uint32_t prevLen, type, size, dts,pos=0;
  bool firstVideo=true;
  _isvideopresent=0;
  _isaudiopresent=0;
  audioTrack=NULL;
  videoTrack=NULL;
  _videostream.dwRate=0;
  _filename=ADM_strdup(name);
  _fd=ADM_fopen(name,"rb");
  if(!_fd)
  {
    printf("[FLV] Cannot open %s\n",name);
    return 0;
  }
  // Get size
  uint32_t fileSize=0;
  fseeko(_fd,0,SEEK_END);
  fileSize=ftello(_fd);
  fseeko(_fd,0,SEEK_SET);
  printf("[FLV] file size :%u bytes\n",fileSize);
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
    GUI_Info_HIG(ADM_LOG_INFO,"Warning","This FLV file says it has no video.\nI will assume it has and try to continue");
    _isvideopresent=1;
    }
  if(flags & 4) // Audio
  {
    _isaudiopresent=1;
    printf("[FLV] Audio flag\n");
  }


  // Skip header
  uint32_t skip=read32();
  fseeko(_fd,skip,SEEK_SET);
  printf("[FLV] Skipping %u header bytes\n",skip);
  pos=ftello(_fd);;
  printf("pos:%u/%u\n",pos,fileSize);
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

    pos=ftello(_fd);
    prevLen=read32();
    type=read8();
    size=read24();
    dts=read24();
    read32(); // ???
    aprintf("--------\n");
    aprintf("prevLen=%d\n",(int)prevLen);
    aprintf("type  =%d\n",(int)type);
    aprintf("size  =%d\n",(int)size);
    aprintf("dts   =%d\n",(int)dts);
    if(!size) continue;
    uint32_t remaining=size;
    //printf("[FLV] At %08x found type %x size %u pts%u\n",pos,type,size,dts);
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
            if(remaining)
                insertAudio(ftello(_fd),remaining,dts);
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
                if(true==extraHeader(videoTrack,&remaining,true,&cts)) continue;
                int64_t sum=cts+dts;
                if(sum<0) pts=0xffffffff;
                    else pts=dts+(int32_t)cts;

            }
            if(remaining)
                insertVideo(ftello(_fd),remaining,frameType,dts,pts);
          }
           break;
      default: printf("[FLV]At 0x%x, unhandled type %u\n",pos,type);
    }
    Skip(remaining);
  } // while

  // Udpate frame count etc..
  ADM_info("\n[FLV] Found %u frames\n",videoTrack->_nbIndex);
  
  if(!metaWidth && !metaHeight &&  videoCodec==FLV_CODECID_H264)
  {
      ADM_info("No width / height, trying to get them..\n");
      
      uint32_t spsLen,ppsLen;
      uint8_t *spsData,*ppsData;
      if( ADM_getH264SpsPpsFromExtraData(videoTrack->extraDataLen,videoTrack->extraData,
                                    &spsLen,&spsData,
                                    &ppsLen,&ppsData))
      {
               ADM_SPSInfo spsinfo;
                if(extractSPSInfo (spsData,spsLen,&spsinfo))
                {
                        ADM_info("W %d\n",spsinfo.width);
                        ADM_info("H %d\n",spsinfo.height);
                        if(spsinfo.width && spsinfo.height)
                        {
                            metaWidth=spsinfo.width;
                            metaHeight=spsinfo.height;
                            updateDimensionWithMeta(FLV_CODECID_H264);
                        }
                        
                }else
                {
                    ADM_warning("Cannot decode SPS\n");
                }
      }else
      {
          ADM_warning("Cannot extract pps and sps\n");
      }
          
  }
  
   _videostream.dwLength= _mainaviheader.dwTotalFrames=videoTrack->_nbIndex;
   // Compute average fps
    float f=_videostream.dwLength;
    uint64_t duration=videoTrack->_index[videoTrack->_nbIndex-1].dtsUs;

    if(duration)
          f=1000.*1000.*1000.*f/duration;
     else  f=25000;
    // If it was available from the metadata, use the one from metadata
    if(! _videostream.dwRate)
    {
        float d=searchMinimum();
        printf("[FLV] minimum delta :%d\n",(uint32_t)d);
        d=1/d;
        d*=1000*1000*1000;

        uint32_t avg=(uint32_t)floor(f);
        uint32_t max=(uint32_t)floor(d);
        if(max<2) max=2; // 500 fps max
        printf("[FLV] Avg fps :%d, max fps :%d\n",avg,max);
        _videostream.dwRate=max;
    }
    _videostream.dwScale=1000;
    _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(_videostream.dwRate);
   printf("[FLV] Duration %"PRIu64" ms\n",videoTrack->_index[videoTrack->_nbIndex-1].dtsUs/1000);

   //
    _videostream.fccType=fourCC::get((uint8_t *)"vids");
    _video_bih.biBitCount=24;
    _videostream.dwInitialFrames= 0;
    _videostream.dwStart= 0;
    videoTrack->_index[0].flags=AVI_KEY_FRAME;

    // audio track
    if(_isaudiopresent)
    {
        ADM_flvAccess *access=new ADM_flvAccess(name,audioTrack);
        _audioStream=ADM_audioCreateStream(&wavHeader,access);
    }
    else
    {
        _audioStream = NULL;
       access=NULL;
    }

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
  
      uint32_t pos=ftello(_fd);
      uint32_t len=*remaining,width,height;
      uint8_t *buffer=new uint8_t[len];
      read(len,buffer);
      fseeko(_fd,pos,SEEK_SET);
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
uint8_t flvHeader::insertVideo(uint32_t pos,uint32_t size,uint32_t frameType,uint32_t dts,uint32_t pts)
{
    videoTrack->grow();
    flvIndex *x=&(videoTrack->_index[videoTrack->_nbIndex]);
    x->size=size;
    x->pos=pos;
    x->dtsUs=dts*1000LL;
    if(pts==0xffffffff) x->ptsUs=ADM_NO_PTS;
        else
      x->ptsUs=pts*1000LL;
    if(frameType==1)
    {
        x->flags=AVI_KEY_FRAME;
    }
    else
    {
          x->flags=0;
    }
    videoTrack->_nbIndex++;
    return 1;
}
/**
      \fn insertVideo
      \brief add a frame to the index, grow the index if needed
*/
uint8_t flvHeader::insertAudio(uint32_t pos,uint32_t size,uint32_t pts)
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
  if(_fd) fclose(_fd);
  if(_audioStream) delete _audioStream;
  if(access) delete access;
  
  
  _fd=NULL;
  _filename=NULL;
  videoTrack=NULL;
  audioTrack=NULL;
  _audioStream=NULL;
  access=NULL;
  return 1;
}
/**
    \fn flvHeader
    \brief constructor
*/

 flvHeader::flvHeader( void ) : vidHeader()
{
    videoCodec=0xFFFF;
    _fd=NULL;
    _filename=NULL;
    videoTrack=NULL;
    audioTrack=NULL;
    _audioStream=NULL;
    access=NULL;
    memset(&wavHeader,0,sizeof(wavHeader));
    metaWidth=0;
    metaHeight=0;
    metaFps1000=0;
    metaFrameWidth=0;
    metaFrameHeight=0;

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
     fseeko(_fd,idx->pos,SEEK_SET);
     fread(img->data,idx->size,1,_fd);
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
      printf("[MKV] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,videoTrack->_nbIndex);
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
      printf("[MKV] Frame %"PRIu32" exceeds # of frames %"PRIu32"\n",frame,videoTrack->_nbIndex);
      return 0;
    }

     flvIndex *idx=&(videoTrack->_index[frame]);
    
    idx->dtsUs=dts; // FIXME
    idx->ptsUs=pts;
    //*pts=idx->timeCodeUs; // FIXME PTS=DTS ??
    return true;
}

//EOF

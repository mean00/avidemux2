/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr


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

#include "ADM_flv.h"

#include <math.h>

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
    fread(&r,1,1,_fd);
    return r;
}
uint32_t flvHeader::read16(void)
{
  uint8_t r[2];
    fread(r,2,1,_fd);
    return (r[0]<<8)+r[1];
}
uint32_t flvHeader::read24(void)
{
  uint8_t r[3];
    fread(r,3,1,_fd);
    return (r[0]<<16)+(r[1]<<8)+r[2];
}
uint32_t flvHeader::read32(void)
{
  uint8_t r[4];
    fread(r,4,1,_fd);
    return (r[0]<<24)+(r[1]<<16)+(r[2]<<8)+r[3];
}
/**
    \fn     readFlvString
    \brief  read pascal like string
*/
char *flvHeader::readFlvString(void)
{
static char stringz[255];
    int size=read16();
    read(size,(uint8_t *)stringz);
    stringz[size]=0;
    return stringz;
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
}
/**
    \fn parseMetaData
    \brief
*/
uint8_t flvHeader::parseMetaData(uint32_t remaining)
{
    uint32_t endPos=ftello(_fd)+remaining;
    {
        // Check the first one is onMetaData...
        uint8_t type=read8();

        if(type!=AMF_DATA_TYPE_STRING) // String!
            goto endit;
        char *z=readFlvString();
        printf("[FlashString] %s\n",z);
        if(z && strncmp(z,"onMetaData",10)) goto endit;
        // Normally the next one is mixed array
        Skip(4);
        Skip(1);
        while(ftello(_fd)<endPos-4)
        {

            char *s=readFlvString();
            printf("[FlvType] :%d String : %s",type,s);
            type=read8();
            switch(type)
            {
                case AMF_DATA_TYPE_DATE: Skip(8+2);break;
                case AMF_DATA_TYPE_NUMBER:
                                        {
                                            float val;
                                            uint64_t hi,lo;
                                            hi=read32();lo=read32();
                                            hi=(hi<<32)+lo;
                                            val=(float)av_int2dbl(hi);
                                            printf("->%f\n",val);
                                            setProperties(s,val);
                                        }
                                        ;break;
                case AMF_DATA_TYPE_STRING: {int r=read16();Skip(r);}break;
                case AMF_DATA_TYPE_BOOL: read8();break;
                case AMF_DATA_TYPE_OBJECT: goto endit; // unsupported for the moment
                default : printf("\n");ADM_assert(0);
            }
            printf("\n");

        }

        // Process them...
    }
endit:
    fseeko(_fd,endPos,SEEK_SET);
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
        if((x[1].timeCodeUs-x[0].timeCodeUs)<delta) delta=x[1].timeCodeUs-x[0].timeCodeUs;

    }
    return delta;
}

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t flvHeader::open(const char *name)
{
  uint32_t prevLen, type, size, pts,pos=0;

  _isvideopresent=0;
  _isaudiopresent=0;
  audioTrack=NULL;
  videoTrack=NULL;
  _videostream.dwRate=0;
  _filename=ADM_strdup(name);
  _fd=fopen(name,"rb");
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
    pos=ftello(_fd);
    prevLen=read32();
    type=read8();
    size=read24();
    pts=read24();
    read32(); // ???
    uint32_t remaining=size;
    //printf("[FLV] At %08x found type %x size %u pts%u\n",pos,type,size,pts);
    switch(type)
    {
      case FLV_TAG_TYPE_AUDIO:
          {
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
            insertAudio(pos+of,remaining,pts);
          }
          break;
      case FLV_TAG_TYPE_META:
                parseMetaData(remaining);
                remaining=0;
                break;
      case FLV_TAG_TYPE_VIDEO:
          {
            int of=1+4+3+3+1+4;
            uint8_t flags=read8();
            remaining--;
            int frameType=flags>>4;

            int codec=(flags)&0xf;

            if(codec==FLV_CODECID_VP6)
            {
              read8(); // 1 byte of extraData
              remaining--;
              of++;
            }
            int first=0;
            if(!videoTrack->_nbIndex) first=1;
            insertVideo(pos+of,remaining,frameType,pts);
            if(first) // first frame..
            {
                if(!setVideoHeader(codec,&remaining)) return 0;
            }

          }
           break;
      default: printf("[FLV]At 0x%x, unhandled type %u\n",pos,type);
    }
    Skip(remaining);
  } // while

  // Udpate frame count etc..
  printf("[FLV] Found %u frames\n",videoTrack->_nbIndex);
   _videostream.dwLength= _mainaviheader.dwTotalFrames=videoTrack->_nbIndex;
   // Compute average fps
    float f=_videostream.dwLength;
    uint64_t duration=videoTrack->_index[videoTrack->_nbIndex-1].timeCodeUs;

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
   printf("[FLV] Duration %"LLU" ms\n",videoTrack->_index[videoTrack->_nbIndex-1].timeCodeUs/1000);

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
     uint64_t dur=videoTrack->_index[videoTrack->_nbIndex-1].timeCodeUs;
    return dur;
}

/**
      \fn setVideoHeader
      \brief Set codec and stuff
*/
uint8_t flvHeader::setVideoHeader(uint8_t codec,uint32_t *remaining)
{
    printf("[FLV] Video Codec:%u\n",codec);
     _video_bih.biWidth=_mainaviheader.dwWidth=320;
    _video_bih.biHeight=_mainaviheader.dwHeight=240;
    switch(codec)
    {
      case FLV_CODECID_H263:            _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"FLV1");break;
      case FLV_CODECID_VP6:             _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"VP6F");break;
    //???   case FLV_CODECID_SCREEN:          _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"VP6F");break;
      default :                         _videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"XXX");break;

    }
    if(codec==FLV_CODECID_VP6)
    {
        read8();
        read8();
        *remaining-=2;

         _video_bih.biHeight=_mainaviheader.dwHeight=read8()*16;
         _video_bih.biWidth=_mainaviheader.dwWidth=read8()*16;
        *remaining-=2;
    }
    if(codec==FLV_CODECID_H263)
    {
      uint32_t len=*remaining,width,height;
      uint8_t buffer[len];
      read(len,buffer);
      *remaining=0;
       /* Decode header, from h263dec.c / lavcodec*/
      if(extractH263FLVInfo(buffer,len,&width,&height))
      {
         _video_bih.biHeight=_mainaviheader.dwHeight=height;
         _video_bih.biWidth=_mainaviheader.dwWidth=width;
      }
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
uint8_t flvHeader::insertVideo(uint32_t pos,uint32_t size,uint32_t frameType,uint32_t pts)
{
    videoTrack->grow();
    flvIndex *x=&(videoTrack->_index[videoTrack->_nbIndex]);
    x->size=size;
    x->pos=pos;
    x->timeCodeUs=pts*1000LL;
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
    x->timeCodeUs=pts*1000LL;
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
  if(videoTrack) delete videoTrack;
  if(audioTrack) delete audioTrack;
  if(_fd) fclose(_fd);
  if(_audioStream) delete _audioStream;
  if(access) delete access;
  _fd=NULL;
  _filename=NULL;
  videoTrack=NULL;
  audioTrack=NULL;
  _audioStream=NULL;
  access=NULL;
}
/**
    \fn flvHeader
    \brief constructor
*/

 flvHeader::flvHeader( void ) : vidHeader()
{

    _fd=NULL;
    _filename=NULL;
    videoTrack=NULL;
    audioTrack=NULL;
    _audioStream=NULL;
    access=NULL;
    memset(&wavHeader,0,sizeof(wavHeader));

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
     if(frame>=videoTrack->_nbIndex) return 0;
     flvIndex *idx=&(videoTrack->_index[frame]);
     return idx->timeCodeUs;
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
     img->demuxerDts=idx->timeCodeUs;;
     img->demuxerPts=idx->timeCodeUs;;
     return 1;
}
/**
        \fn getExtraHeaderData
*/
uint8_t  flvHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=0; //_tracks[0].extraDataLen;
                *data=NULL; //_tracks[0].extraData;
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
      printf("[MKV] Frame %"LU" exceeds # of frames %"LU"\n",frame,videoTrack->_nbIndex);
      return 0;
    }

     flvIndex *idx=&(videoTrack->_index[frame]);
    
    *dts=idx->timeCodeUs; // FIXME
    *pts=idx->timeCodeUs;
    return true;
}
/**
        \fn setPtsDts
*/
bool    flvHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    if(frame>=videoTrack->_nbIndex)
    {
      printf("[MKV] Frame %"LU" exceeds # of frames %"LU"\n",frame,videoTrack->_nbIndex);
      return 0;
    }

     flvIndex *idx=&(videoTrack->_index[frame]);
    
    idx->timeCodeUs=dts; // FIXME
    //*pts=idx->timeCodeUs; // FIXME PTS=DTS ??
    return true;
}

//EOF

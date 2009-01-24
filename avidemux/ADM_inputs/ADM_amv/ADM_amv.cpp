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
#include "config.h"
#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"

#include "fourcc.h"


#include "ADM_amv.h"


uint32_t ADM_UsecFromFps1000(uint32_t fps1000);

/**
    \fn Skip
    \brief Skip some bytes from the file
*/
uint8_t amvHeader::Skip(uint32_t len)
{
  fseeko(_fd,len,SEEK_CUR);
  return 1; 
}
/**
    \fn read
    \brief read the given size from file
*/

uint8_t amvHeader::read(uint32_t len, uint8_t *where)
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
uint8_t amvHeader::read8(void)
{
  uint8_t r;
    fread(&r,1,1,_fd);
    return r; 
}
uint32_t amvHeader::read16(void)
{
  uint8_t r[2];
    fread(r,2,1,_fd);
    return (r[1]<<8)+r[0]; 
}
uint32_t amvHeader::read24(void)
{
  uint8_t r[3];
    fread(r,3,1,_fd);
    return (r[2]<<16)+(r[1]<<8)+r[0]; 
}
uint32_t amvHeader::read32(void)
{
  uint8_t r[4];
    fread(r,4,1,_fd);
    return (r[3]<<24)+(r[2]<<16)+(r[1]<<8)+r[0]; 
}

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t amvHeader::open(const char *name)
{
  uint32_t prevLen, type, size, pts,pos=0;
  
  _isvideopresent=0;
  _isaudiopresent=0;
  memset(&_mainaviheader,0,sizeof(_mainaviheader));
  _filename=ADM_strdup(name);
  _fd=fopen(name,"rb");
  if(!_fd)
  {
    printf("[AMV] Cannot open %s\n",name);
    return 0; 
  }
  // Get size
  uint32_t fileSize=0;
  fseeko(_fd,0,SEEK_END);
  fileSize=ftello(_fd);
  fseeko(_fd,0,SEEK_SET);
  printf("[AMV] file size :%u bytes\n",fileSize);
  // It must begin by F L V 01
  uint32_t four=read32();
  
  if(four!=MKFCC('R','I','F','F'))
  {
    printf("[AMV] Not a riff file\n");
    return 0; 
  }
  uint32_t s=read32(); // size;
  if(s<fileSize) fileSize=s;
  
  if(read32()!=MKFCC('A','M','V',' '))
  {
    // Not an AMV file
     printf("[AMV] Not an AMV file\n");
    return 0; 
  }
  int hdr,list,nbTrack=0;
  while((list=read32())==MKFCC('L','I','S','T'))
  {
    printf("[AMV] List\n");
    read32();
    hdr=read32();
    
    switch(hdr)
    {
      case MKFCC('h','d','r','l'):
          printf("\t[AMV] Header\n");
          if(!readHeader()) return 0;
          printf("\t[AMV] Header Ok\n");
          break;
      case MKFCC('s','t','r','l'):
          printf("\t[AMV] Track\n");
          if(!readTrack(nbTrack)) return 0;
          nbTrack++;
          printf("\t[AMV] Track Ok\n");
          break;
      case MKFCC('m','o','v','i'):
            printf("\t[AMV] Indexing\n");
            if(!index()) return 0;
            printf("\t[AMV] Indexing Ok\n");
            break;
      default:
          printf("[AMV] Last :%s",fourCC::tostring(hdr));
          ADM_assert(0);
    }
  }
  if(videoTrack.index) videoTrack.index[0].flags=AVI_KEY_FRAME;
  else return 0;
  _isvideopresent=1;
  // Build a fake stream/bih bih
#define ENTRY(x,y) _videostream.x=y
  uint32_t codec=MKFCC('A','M','V',' ');
    ENTRY(	fccType,MKFCC('v','i','d','s'));
    ENTRY(	fccHandler,codec);
    ENTRY(	dwInitialFrames,0);
    ENTRY(	dwScale,1000);
    ENTRY(	dwRate,8000);		/* dwRate / dwScale == samples/second */
  // _video_bih
#undef ENTRY
#define ENTRY(x,y) _video_bih.x=y
    
     
    ENTRY(biWidth,_mainaviheader.dwWidth);
    ENTRY(biHeight,_mainaviheader.dwHeight);
    ENTRY(biCompression,codec);
    printf("[AMV] Dimension %u * %u\n",_video_bih.biWidth,_video_bih.biHeight);
    
    // If audio...
    if(audioTrack.index)
    {
      _isaudiopresent=1;
      _audio=new amvAudio(name,&audioTrack,&wavHeader);
    }
  return 1;
}
/**
    \fn    index
    \brief read hdlr
*/
uint8_t amvHeader::index(void)
{
  printf("[AMV] Dimension %u * %u\n",_mainaviheader.dwWidth,_mainaviheader.dwHeight);
  printf("[AMV] # pics  %u \n",_mainaviheader.dwTotalFrames);
  uint32_t type,size,nbPics=0,nbAudio=0;
  uint32_t org=ftello(_fd);
  
  
  while(!feof(_fd))
  {
     type=read32();
     size=read32();
     if(feof(_fd)) break;
     switch(type)
        {
        case MKFCC('0','0','d','c'):
          nbPics++;
          break;
        case MKFCC('0','1','w','b'):
          nbAudio++;
          break;
        default:
            printf("[AMV] Unknown entry :%s\n",fourCC::tostring(type));
        }
     fseeko(_fd,size,SEEK_CUR);
  }
  printf("Found %u pics\n",nbPics);
  printf("Found %u audio\n",nbAudio);
  _mainaviheader.dwTotalFrames=nbPics;
  // Allocate video index
  videoTrack.indexRoof=nbPics;
  videoTrack.index=new amvIndex[nbPics];
  videoTrack.nbIndex=0;
  // Allocate audio index
  audioTrack.indexRoof=nbAudio;
  audioTrack.index=new amvIndex[nbAudio];
  audioTrack.nbIndex=0;
  
      
  //
  fseeko(_fd,org,SEEK_SET);
  uint32_t idx=0;
  while(!feof(_fd))
  {
     type=read32();
     size=read32();
     if(feof(_fd)) break;
     switch(type)
        {
        case MKFCC('0','0','d','c'):
          videoTrack.index[idx].pos=ftello(_fd);
          videoTrack.index[idx].size=size;
          videoTrack.index[idx].flags=AVI_KEY_FRAME;
          idx++;
          break;
        case MKFCC('0','1','w','b'):
        {
          int adx=audioTrack.nbIndex;
          audioTrack.index[adx].pos=ftello(_fd);
          audioTrack.index[adx].size=size;
          audioTrack.index[adx].flags=AVI_KEY_FRAME;
          audioTrack.nbIndex++;
        }
          break;
        default:
            printf("[AMV] Unknown entry :%s\n",fourCC::tostring(type));
        }
     fseeko(_fd,size,SEEK_CUR);
  }
  return 1;
}
/**
    \fn    readHeader
    \brief read hdlr
*/
uint8_t amvHeader::readHeader(void)
{
  uint32_t tail,sz;
  if(read32()!=MKFCC('a','m','v','h'))
  {
    printf("[AMV]Wrong header (amvh)\n"); 
    return 0;
  }
  sz=read32();
  printf("\t\t[AMV] Header size : %u,sizeof :%u\n",sz,sizeof(MainAVIHeader));
  tail=sz+ftello(_fd);
  if(sz!=sizeof(MainAVIHeader))
  {
    printf("[AMV] Wrong mainheader size\n");
    return 0; 
  }
  fread(&_mainaviheader,sz,1,_fd);
  fseeko(_fd,tail,SEEK_SET);
  return 1;
}
/**
    \fn    readHeader
    \brief read one track
*/
uint8_t amvHeader::readTrack(int nb)
{
  uint32_t tail,sz;
  AVIStreamHeader stream;
  if(read32()!=MKFCC('s','t','r','h'))
  {
    printf("[AMV]Wrong header (strh)\n"); 
    return 0;
  }
  // First is strh
  sz=read32();
  printf("\t\t[AMV] strh size : %u,sizeof :%u\n",sz,sizeof(AVIStreamHeader));
  fseeko(_fd,sz,SEEK_CUR);
  
  // The strf
  if(read32()!=MKFCC('s','t','r','f'))
  {
    printf("[AMV]Wrong header (strf)\n"); 
    return 0;
  }
  sz=read32();
  uint32_t pos=ftell(_fd);
  printf("\t\t[AMV] strf size : %u,sizeof :%u\n",sz,sizeof(AVIStreamHeader));
  if(nb==1) // Second track=Audio
  {
                                   read16(); // Tag
      wavHeader. 	channels  =read16();
      wavHeader. 	frequency =read32();
      wavHeader. 	byterate  =read32();
      wavHeader. 	blockalign=read16();
      wavHeader. 	encoding  =WAV_AMV_ADPCM;       
      wavHeader. 	bitspersample=16;	 
 
  }
  fseeko(_fd,sz+pos,SEEK_SET);
  
  return 1;
}
/*
  __________________________________________________________
*/
WAVHeader *amvHeader::getAudioInfo(void )
{
  if(_audio)
    return &wavHeader;
  else
      return NULL;
}
/*
    __________________________________________________________
*/

uint8_t amvHeader::getAudioStream(AVDMGenericAudioStream **audio)
{

  if(_audio)
  {
    *audio=_audio;
    return 1;
  }
  *audio=NULL;
  return 0; 
}
/*
    __________________________________________________________
*/

void amvHeader::Dump(void)
{
 
}
/*
    __________________________________________________________
*/

uint8_t amvHeader::close(void)
{
  if(_filename) ADM_dealloc(_filename);
  if(videoTrack.index) delete [] videoTrack.index;
  if(audioTrack.index) delete [] audioTrack.index;
  videoTrack.index=NULL;
  audioTrack.index=NULL;
  
  if(_fd) fclose(_fd);
//  if(_audioStream) delete _audioStream;
  
  _fd=NULL;
  _filename=NULL;
  if(_audio) delete _audio;
  _audio=NULL;
}
/*
    __________________________________________________________
*/

 amvHeader::amvHeader( void ) : vidHeader()
{ 
    _fd=NULL;
    _filename=NULL;
    memset(&videoTrack,0,sizeof(videoTrack));
    memset(&audioTrack,0,sizeof(videoTrack));
    memset(&wavHeader,0,sizeof(wavHeader));
    _filename=NULL;
    _audio=NULL;
}
/*
    __________________________________________________________
*/

 amvHeader::~amvHeader(  )
{
  close();
}

/*
    __________________________________________________________
*/

/*
    __________________________________________________________
*/

  uint8_t  amvHeader::setFlag(uint32_t frame,uint32_t flags)
{
 if(frame>=_mainaviheader.dwTotalFrames) return 0;
 amvIndex *idx=&(videoTrack.index[frame]);
 idx->flags=flags;
 return 1;
}
/*
    __________________________________________________________
*/

uint32_t amvHeader::getFlags(uint32_t frame,uint32_t *flags)
{
 if(frame>=_mainaviheader.dwTotalFrames) return 0;
 amvIndex *idx=&(videoTrack.index[frame]);
 *flags=idx->flags;
 return 1;
}
/*
    __________________________________________________________
*/

uint8_t  amvHeader::getFrameNoAlloc(uint32_t frame,ADMCompressedImage *img)
{
 if(frame>=_mainaviheader.dwTotalFrames) return 0;
 amvIndex *idx=&(videoTrack.index[frame]);
 fseeko(_fd,idx->pos,SEEK_SET);
 fread(img->data,idx->size,1,_fd);
  img->dataLength=idx->size;
  img->flags=AVI_KEY_FRAME;
 return 1;
}
/*
    __________________________________________________________
*/

uint8_t  amvHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
                *len=0; //_tracks[0].extraDataLen;
                *data=NULL; //_tracks[0].extraData;
                return 1;            
}
/*
    __________________________________________________________
*/
uint8_t  amvHeader::changeAudioStream(uint32_t newstream)
{
    return 0;
}
/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t amvHeader::getFrameSize (uint32_t frame, uint32_t * size)
{
  if(frame>=_mainaviheader.dwTotalFrames) return 0;
  *size=videoTrack.index[frame].size;
  return 1;
}

/*
    __________________________________________________________
*/
uint32_t  amvHeader::getCurrentAudioStreamNumber(void)
{
  return 0;
}
/**
    \fn getAudioStreamsInfo
    \brief returns infos about audio streams (code,...)
    @param nbStreams (out) nb audio streams
    @param infos (out) pointer to streams info. It is up to the caller to free them.
*/
uint8_t  amvHeader::getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos)
{
    if(!_isaudiopresent)
    {}
        *nbStreams=0;
        *infos=NULL;
        return 1;
   
}
//EOF

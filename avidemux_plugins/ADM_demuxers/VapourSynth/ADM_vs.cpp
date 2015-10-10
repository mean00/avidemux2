/***************************************************************************
    \file ADM_vs.cpp
    \author (C) 2015 by mean    email                : fixounet@free.fr
    \brief VapourSynth demuxer

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

#include "ADM_vs.h"

#include <math.h>

static const VSAPI *vsapi = NULL;
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
uint32_t ADM_UsecFromFps1000(uint32_t fps1000);

/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t vsHeader::open(const char *name)
{
    ADM_info("Opening %s as VapourSynth file\n",name);
    
   
    
    if (!vsscript_init()) 
    {
          ADM_warning("Cannot initialize vsapi script_init. Check PYTHONPATH\n");
          return false;
    }
    if(!vsapi)
    {
        vsapi = vsscript_getVSApi();
        if(!vsapi)
        {
            ADM_warning("Cannot get vsAPI entry point\n");
            vsscript_finalize();
            return 0;
        }
        
    }
      
    if (vsscript_createScript(&_script))
    {
        ADM_warning("Script Init failed <%s>\n", vsscript_getError(_script));
        vsscript_freeScript(_script);
        vsscript_finalize();
        return 0;
    }
    _node = vsscript_getOutput(_script, _outputIndex);
    if (!_node) 
    {
       ADM_warning("vsscript_getOutput failed\n");
       vsscript_freeScript(_script);
       vsscript_finalize();
       return 0;
    }
    const VSVideoInfo *vi = vsapi->getVideoInfo(_node);
    if(!vi)
    {
          ADM_warning("Cannot get information on node\n");
          vsscript_freeScript(_script);
          vsscript_finalize();
          return 0;
    }
    ADM_info("Format    : %s\n",vi->format->name);
    ADM_info("FrameRate : %d / %d\n",vi->fpsNum,vi->fpsDen);
    ADM_info("Width     : %d\n",vi->width);
    ADM_info("Height    : %d\n",vi->height);
    ADM_info("Frames    : %d\n",vi->numFrames);
    ADM_info("Flags     : 0x%x\n",vi->flags);
    

#if 0
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
        ADM_vsAccess *access=new ADM_vsAccess(name,audioTrack);
        _audioStream=ADM_audioCreateStream(&wavHeader,access);
    }
    else
    {
        _audioStream = NULL;
       access=NULL;
    }

  printf("[FLV]FLV successfully read\n");
#endif
     
  return 1;
}
/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t vsHeader::getVideoDuration(void)
{
       return 25*1000*1000;
}
#if 0
/**
    \fn setVideoHeader
*/
uint8_t vsHeader::setVideoHeader(uint8_t codec,uint32_t *remaining)
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
uint8_t   vsHeader::setAudioHeader(uint32_t format,uint32_t fq,uint32_t bps,uint32_t channels)
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
#endif
uint8_t  vsHeader::setFlag(uint32_t frame,uint32_t flags)
{
    return 1;
}
uint32_t vsHeader::getFlags(uint32_t frame,uint32_t *flags)
{
    return AVI_KEY_FRAME;
}

/**
    \fn getAudioInfo
    \brief returns wav header for stream i (=0)
*/
WAVHeader *vsHeader::getAudioInfo(uint32_t i )
{
      return NULL;
}
/**
 * 
 * @param frame
 * @return 
 */
 uint64_t vsHeader::getTime(uint32_t frame)
 {
     return frame*25000;
 }
/**
   \fn getAudioStream
*/

uint8_t   vsHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
  *audio=NULL;
  return 0;
}
/**
    \fn getNbAudioStreams

*/
uint8_t   vsHeader::getNbAudioStreams(void)
{
   return 0;
}
/**
    \fn close
    \brief cleanup
*/

uint8_t vsHeader::close(void)
{
    if(_script)
    {
          vsscript_freeScript(_script);
          vsscript_finalize();
    }
    // _node ?
   return 1;
}
/**
    \fn vsHeader
    \brief constructor
*/

 vsHeader::vsHeader( void ) : vidHeader()
{
    _fd=NULL;
    _script=NULL;
    _node = NULL;
}
/**
    \fn vsHeader
    \brief destructor
*/

 vsHeader::~vsHeader(  )
{
  close();
}


/**
        \fn getFrame
*/

uint8_t  vsHeader::getFrame(uint32_t frame,ADMCompressedImage *img)
{
        return 1;
}
/**
        \fn getExtraHeaderData
*/
uint8_t  vsHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
        *len=0;
        *data=NULL;
        return 1;
}
/**
 * 
 */
 void     vsHeader::Dump(void)
 {
     return;
 }
/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t vsHeader::getFrameSize (uint32_t frame, uint32_t * size)
{
  //  *size = videoTrack->_index[frame].size;
  return 1;
}
//!!

/**
    \fn getPtsDts
*/
bool    vsHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
//    *dts=idx->dtsUs; // FIXME
    //*pts=idx->ptsUs;
    return true;
}
/**
        \fn setPtsDts
*/
bool    vsHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
       return false;
}

//EOF

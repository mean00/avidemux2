/** *************************************************************************
    \file       ADM_wtv.cpp
    \brief      WTV demuxer
    copyright            : (C) 2010 by mean
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

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_wtv.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
    \fn getAudioInfo
*/
WAVHeader *wtvHeader::getAudioInfo(uint32_t i )
{
  if(!_nbAudioTrack) return NULL;

  ADM_assert(i<_nbAudioTrack);
  if(!_audioAccess) return NULL;
  return &(_allAudioTracks[i].wavHeader);
}
/**
    \fn getAudioStream
*/
uint8_t    wtvHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
 *audio=NULL;
  if(!_nbAudioTrack) return true;
  ADM_assert(i<_nbAudioTrack);
  *audio=_audioStreams[i];
 return 1;
}
/**
    \fn getNbAudioStreams
*/
uint8_t                 wtvHeader::getNbAudioStreams(void)
{
    return _nbAudioTrack;
}

/**
    \fn Dump
*/

void wtvHeader::Dump(void)
{

  printf("*********** WTV INFO***********\n");
}

/**
    \fn close
*/

uint8_t wtvHeader::close(void)
{
	if (_fd)
		fclose(_fd);

	_fd=NULL;

  if(_videoExtraData)
  {
    delete [] _videoExtraData;
    _videoExtraData=NULL;
  }
  if(myName)
  {
    delete myName;
    myName=NULL;
  }
  if(_videoExtraData)
  {
    delete [] _videoExtraData;
    _videoExtraData=NULL;
  }



  for(int i=0;i<_nbAudioTrack;i++)
  {
    wtvAudioTrak *trk=&(_allAudioTracks[i]);
    if(trk->extraData) delete [] trk->extraData;
    trk->extraData=NULL;
    delete    _audioAccess[i];
    _audioAccess[i]=NULL;
    delete _audioStreams[i];
    _audioStreams[i]=NULL;
  }
  return 1;
}
/**
    \fn wtvHeader
*/


 wtvHeader::wtvHeader( void ) : vidHeader()
{
  _fd=NULL;
  _videoIndex=-1;
  myName=NULL;
  _videoStreamId=0;
  nbImage=0;
  memset(&(_allAudioTracks[0]),0,sizeof(_allAudioTracks));
  memset(&(_audioAccess[0]),0,sizeof(_audioAccess));
  memset(&(_audioStreams[0]),0,sizeof(_audioStreams));
  _nbAudioTrack=0;

}
/**
    \fn ~ wtvHeader
*/

 wtvHeader::~wtvHeader(  )
{
  close();
}
/**
    \fn open
*/

uint8_t wtvHeader::open(const char *name)
{
  _fd=ADM_fopen(name,"rb");
  if(!_fd)
  {
    GUI_Error_HIG("File Error.","Cannot open file\n");
    return 0;
  }
  return false;
  for(int i=0;i<_nbAudioTrack;i++)
  {
        _audioAccess[i]=new wtvAudioAccess(this,i);
        _audioStreams[i]=ADM_audioCreateStream(&(_allAudioTracks[i].wavHeader), _audioAccess[i]);
  }
  return 1;
}
/**
    \fn setFlag
*/

  uint8_t  wtvHeader::setFlag(uint32_t frame,uint32_t flags)
{
  ADM_assert(frame<nbImage);
  return 1;
}
/**
    \fn getFlags
*/

uint32_t wtvHeader::getFlags(uint32_t frame,uint32_t *flags)
{
  if(frame>=nbImage) return 0;
  *flags=AVI_KEY_FRAME;
  return 1;
}
/**
    \fn getFrameSize
*/
uint8_t     wtvHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    *size=0;
    if(frame>=nbImage) return 0;
    *size=1;
    return true;
}
/**
    \fn getTime
*/
uint64_t                   wtvHeader::getTime(uint32_t frameNum)
{
     if(frameNum>=nbImage) return 0;
     return ADM_NO_PTS;
}
/**
    \fn getTime
*/

uint64_t                   wtvHeader::getVideoDuration(void)
{
    return 01000000LL;
}
/**
    \fn getTime
*/

bool                       wtvHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>=nbImage) return false;
    *pts=ADM_NO_PTS;
    *dts=ADM_NO_PTS;
    return true;
}
/**
    \fn getTime
*/

bool                       wtvHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
     if(frame>=nbImage) return false;
    return true;
}
/**
    \fn getFrame
*/
uint8_t  wtvHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
  img->dataLength=0;
  img->flags=AVI_KEY_FRAME;
  return false;
}
/*
    __________________________________________________________
*/

//EOF

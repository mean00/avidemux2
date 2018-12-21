/** *************************************************************************
    \file       ADM_nativeAv.cpp
    \brief      Native AVISynth demuxer
    copyright            : (C) 2018 by mean
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
#include "ADM_nativeAvs.h"
#include "ADM_image.h"

/**
*/
const AVS_Linkage *AVS_linkage = 0;
IScriptEnvironment*     env=NULL;
#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

#define nbImage _videostream.dwLength 

/**
*/
bool coldInit(void)
{
	if (!env)
	{
		env = CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION);
		if (!env)
		{
			ADM_warning("Cannot open avisynth dll env\n");
			return false;
		}
		AVS_linkage = env->GetAVSLinkage();
	}
	return true;
}

/**
    \fn getAudioInfo
*/
WAVHeader *nativeAvsHeader::getAudioInfo(uint32_t i )
{
  return NULL;

}
/**
    \fn getAudioStream
*/
uint8_t    nativeAvsHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
    return 0;
}
/**
    \fn getNbAudioStreams
*/
uint8_t                 nativeAvsHeader::getNbAudioStreams(void)
{
    return 0;
}

/**
    \fn Dump
*/

void nativeAvsHeader::Dump(void)
{

  printf("*********** nativeAvs INFO***********\n");
}

/**
    \fn close
*/

uint8_t nativeAvsHeader::close(void)
{
	if (clip)
	{
		delete clip;
		clip = NULL;
	}
	if(env)
	{
		env=NULL;
	}
	return 1;
}
/**
    \fn nativeAvsHeader
*/


 nativeAvsHeader::nativeAvsHeader( void ) : vidHeader()
{
   env=NULL;
   clip = NULL;
}
/**
    \fn ~ nativeAvsHeader
*/

 nativeAvsHeader::~nativeAvsHeader(  )
{
  close();
}
/**
    \fn open
*/

uint8_t nativeAvsHeader::open(const char *name)
{
	if (!coldInit())
		return false;

try{
       
        AVSValue arg(name);
        AVSValue res = env->Invoke("Import", AVSValue(&arg, 1));
        if (!res.IsClip()) 
        {
            ADM_warning("Avisynth:%s didn t return a video clip.\n", name);
            return 0;
        }
#if 0
		if (res.GetVersion() < 5)
		{
			ADM_warning(" avisynth.dll version is too old (%d)\n", res.GetVersion());
			return 0;
		}
#endif

		clip = new PClip;
        (*clip) = res.AsClip();
        // Collect informations...
        VideoInfo inf = (*clip)->GetVideoInfo();
        //
        _isaudiopresent = 0;	
        _isvideopresent = 1;	
        _videostream.dwScale = inf.fps_numerator;
        _videostream.dwRate = inf.fps_denominator;
        _mainaviheader.dwMicroSecPerFrame = 40000;;	// 25 fps hard coded
        _videostream.fccHandler = fourCC::get((uint8_t *) "YV12");
		_video_bih.biCompression=_videostream.fccHandler;
        _videostream.dwLength = _mainaviheader.dwTotalFrames = inf.num_frames;
        _videostream.dwInitialFrames = 0;
        _videostream.dwStart = 0;
        _video_bih.biWidth = _mainaviheader.dwWidth = inf.width;
        _video_bih.biHeight = _mainaviheader.dwHeight = inf.height;
    }
    catch(AvisynthError err) 
    {
        ADM_warning( "Avisynth error:\n%s\n", err.msg);
        return false;
    }
 return true;
}
/**
    \fn setFlag
*/

uint8_t  nativeAvsHeader::setFlag(uint32_t frame,uint32_t flags)
{
  ADM_assert(frame<nbImage);
  return 1;
}
/**
    \fn getFlags
*/

uint32_t nativeAvsHeader::getFlags(uint32_t frame,uint32_t *flags)
{
  if(frame>=nbImage) return 0;
  *flags=AVI_KEY_FRAME;
  return 1;
}
/**
    \fn getFrameSize
*/
uint8_t     nativeAvsHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    *size=0;
    if(frame>=nbImage) return 0;
    *size=   (3* _video_bih.biWidth *  _video_bih.biHeight)>>1;
    return true;
}
/**
    \fn getTime
*/
uint64_t                   nativeAvsHeader::getTime(uint32_t frameNum)
{
     if(frameNum>=nbImage) return 0;
     return frameNum2PTS(frameNum);
}
/**
    \fn getTime
*/

uint64_t                   nativeAvsHeader::getVideoDuration(void)
{
    return 01000000LL;
}
/**
    \fn getTime
*/

bool                       nativeAvsHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>=nbImage) return false;
    *pts=frameNum2PTS(frame);
    *dts=frameNum2PTS(frame);
    return true;
}
/**
    \fn getTime
*/

bool                       nativeAvsHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
     if(frame>=nbImage) return false;
    return true;
}
/**
    \fn getFrame
*/
uint8_t  nativeAvsHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
  img->dataLength=0;
  img->flags=AVI_KEY_FRAME;

  PVideoFrame Aframe=NULL;

  try
  {
        Aframe=(*clip)->GetFrame(framenum,env);

  }
  catch(AvisynthError err) 
    {
        ADM_warning( "Avisynth get frame error :\n%s\n", err.msg);
        return false;
    }
  if(!Aframe)
  {
        ADM_warning("No video frame\n");
        return false;
  }
    // Pack
    int w= _video_bih.biWidth ;
    int h=  _video_bih.biHeight;
    int plane= w*h;
	uint8_t *source;
	uint8_t *dst;
	int     sourcePitch;	

    // Y
	source= (uint8_t *)Aframe->GetReadPtr(AVS_PLANAR_Y);
	sourcePitch = Aframe->GetPitch(AVS_PLANAR_Y);
    dst=img->data;
    BitBlit(dst,w,source,sourcePitch,w,h);
    // U
    source=(uint8_t *)Aframe->GetReadPtr(AVS_PLANAR_U);
    sourcePitch= Aframe->GetPitch(AVS_PLANAR_U);
    dst=img->data+ (plane * 5) / 4; 
    BitBlit(dst,w>>1,source,sourcePitch,w>>1,h>>1);
    // V
    source= (uint8_t *)Aframe->GetReadPtr(AVS_PLANAR_V);
    sourcePitch= Aframe->GetPitch(AVS_PLANAR_V);
	dst = img->data + (plane);
    BitBlit(dst,w>>1,source,sourcePitch,w>>1,h>>1);
    // misc infos 
    img->flags=AVI_KEY_FRAME;
    img->dataLength=(plane*3)>>1;
    img->demuxerFrameNo=framenum;
    img->demuxerPts=frameNum2PTS(framenum);
    img->demuxerDts=frameNum2PTS(framenum);
    // done
  Aframe=NULL;
  return true;
}
/**

*/
uint64_t              nativeAvsHeader::frameNum2PTS(int frameNumber)
{
    if(!frameNumber)
    {
        return 0;
    }
    return ADM_NO_PTS;
}

//EOF

/** *************************************************************************
    \file       ADM_nativeAv.cpp
    \brief      Native AVISynth demuxer
                contains code derived from Avs2YUV by Loren Merritt
    copyright            : (C) 2019 by mean
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
const AVS_Linkage   *AVS_linkage = 0;
static IScriptEnvironment  *env=NULL;
static bool        tried = false;
#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif
#define nbImage _videostream.dwLength 

typedef IScriptEnvironment * __stdcall DLLFUNC(int);

/**
*/
static bool tryColdInit(void)
{
    DLLFUNC *CreateScriptEnvironment = NULL;    
    ADM_info("Loading Avisynth.dll \n");
    HMODULE instance = LoadLibrary("avisynth.dll");
    if (!instance)
    {
        ADM_warning("Cannot load avisynth dll\n");
        return false;
    }
    ADM_info("Avisynth.dll loaded\n");
    CreateScriptEnvironment = (DLLFUNC *)GetProcAddress(instance, "CreateScriptEnvironment");
    if (!CreateScriptEnvironment)
    {
        ADM_warning("failed to load CreateScriptEnvironment()\n");
        return false;
    }
    try
    {
        env = CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION);
        if (!env)
        {
            ADM_warning("Create Env failed\n");
            return false;
        }
        AVS_linkage = env->GetAVSLinkage();
    }
    catch (AvisynthError err)
    {
        ADM_warning("Avisynth error:\n%s\n", err.msg);
        return false;
    }
    return true;
}
/**
*/
static bool coldInit(void)
{
    if (!tried)
    {
        tried = true;
        return tryColdInit();
    }
    return env!=NULL;
}

/**
    \fn getAudioInfo
*/
WAVHeader *nativeAvsHeader::getAudioInfo(uint32_t i )
{
	if (audioAccess)
		return &audioInfo;
  return NULL;

}
/**
    \fn getAudioStream
*/
uint8_t    nativeAvsHeader::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
	if (audioAccess && !i)
	{
		*audio = audioStream;
		return true;
	}
	return false;
}
/**
    \fn getNbAudioStreams
*/
uint8_t                 nativeAvsHeader::getNbAudioStreams(void)
{
	if (audioAccess)
		return 1;
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
		if (audioStream)
		{
			delete audioStream;
			audioStream = NULL;
		}
		if (audioAccess)
		{
			delete audioAccess;
			audioAccess = NULL;
		}
        return 1;
}
/**
    \fn nativeAvsHeader
*/


 nativeAvsHeader::nativeAvsHeader( void ) : vidHeader()
{ 
   clip = NULL;
   audioAccess = NULL;
   audioStream = NULL;
}
/**
    \fn ~ nativeAvsHeader
*/

 nativeAvsHeader::~nativeAvsHeader(  )
{
  close();
  audioAccess = NULL; // need to delete it ?
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
                if (res.AsClip()->GetVersion() < 5)
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
        _videostream.dwRate = inf.fps_numerator;
        _videostream.dwScale = inf.fps_denominator;
        _mainaviheader.dwMicroSecPerFrame = 40000;;	// 25 fps hard coded
        
        if(inf.IsYV12())
        {
            pixfrmt=ADM_PIXFRMT_YV12;
            _videostream.fccHandler = fourCC::get((uint8_t *) "YV12");
        }else
            if(inf.IsYUY2())
            {
                pixfrmt=ADM_PIXFRMT_YUV422;
                _videostream.fccHandler = fourCC::get((uint8_t *) "YUY2");
            }
            else
                return 0;
        
        
        _video_bih.biCompression=_videostream.fccHandler;
        _videostream.dwLength = _mainaviheader.dwTotalFrames = inf.num_frames;
        _videostream.dwInitialFrames = 0;
        _videostream.dwStart = 0;
        _video_bih.biWidth = _mainaviheader.dwWidth = inf.width;
        _video_bih.biHeight = _mainaviheader.dwHeight = inf.height;
        
        // Do we have audio ?
        if(inf.SamplesPerSecond()) // 0 means no audio
        {
			audioInfo.frequency=inf.SamplesPerSecond(),
			audioInfo.channels=inf.AudioChannels();
			
			audioInfo.byterate = audioInfo.channels*audioInfo.frequency;
			if(audioInfo.frequency)
			{
                int sampleType = inf.SampleType();
				if(sampleType == SAMPLE_INT16 || sampleType == SAMPLE_FLOAT)
                        {
                                switch (sampleType)
                                {
                                    case SAMPLE_INT16: audioInfo.encoding = WAV_PCM; audioInfo.bitspersample = 16; break;
                                    case SAMPLE_FLOAT: audioInfo.encoding = WAV_PCM_FLOAT; audioInfo.bitspersample = 32; break;
                                    default:
                                        return false;
                                }
								audioAccess=new nativeAvsAudio(this,&audioInfo, sampleType,getVideoDuration());
								audioStream = ADM_audioCreateStream(&audioInfo, audioAccess);
								if (audioStream)
								{
									ADM_info("Created audio stream\n");
									_isaudiopresent = 1;
								}
                        }
                        else
						{
							ADM_warning("Only int16 for audio and not %d!\n",(int)inf.SampleType());
						}
			}
        }
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
        return frameNum2PTS(_mainaviheader.dwTotalFrames+1);
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
    switch(pixfrmt)
   {
    
    case ADM_PIXFRMT_YV12:
        {
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
            img->dataLength=(plane*3)>>1;
            
        }
        break;
    case ADM_PIXFRMT_YUV422:
        {
            source= (uint8_t *)Aframe->GetReadPtr();
            sourcePitch = Aframe->GetPitch();
            dst=img->data;
            BitBlit(dst,w*2,source,sourcePitch,w*2,h);
            img->dataLength=(plane*2);
        }
        break;
    default:
        Aframe=NULL;
        return false;
        break;
      
  }
    // done
    img->flags=AVI_KEY_FRAME;
    img->demuxerFrameNo=framenum;
    img->demuxerPts=frameNum2PTS(framenum);
    img->demuxerDts=frameNum2PTS(framenum);
    Aframe=NULL;
    return true;
}
/**

*/
uint64_t              nativeAvsHeader::frameNum2PTS(int frameNumber)
{
    if(!frameNumber)
        return 0;
        double d = _videostream.dwScale ;
        d *= (double)frameNumber;
        d /= (double)_videostream.dwRate;
        d *= 1000000.;
    return (uint64_t)d;
}
/**
*/
bool  nativeAvsHeader::getAudioPacket(uint64_t sample,uint8_t *buffer, uint32_t size)
{
    try
    {
        (*clip)->GetAudio(buffer, sample, size, env);
        return true;
    }
    catch (AvisynthError err)
    {
        ADM_warning("Avisynth Audio error:\n%s\n", err.msg);
        return false;
    }
}
//EOF

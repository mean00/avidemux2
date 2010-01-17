/***************************************************************************
    \file ADM_avsproxy.cpp
    \author (C) 2007-2010 by mean  fixounet@free.fr

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

#include "fourcc.h"
#include "ADM_avsproxy.h"
#include "ADM_avsproxy_internal.h"


/**
    \fn avsHeader
*/
avsHeader::avsHeader()
{
    haveAudio=false;
    memset(&wavHeader,0,sizeof(wavHeader));
    audioStream=NULL;
    audioAccess=NULL;
}
/**
    \fn avsHeader
*/

 avsHeader::~avsHeader(  )
{
    close();   
    
}
/**
    \fn close
*/
uint8_t avsHeader::close()
{
    if(audioStream) delete audioStream;
    if(audioAccess) delete audioAccess;
    audioStream=NULL;
    audioAccess=NULL;
    network.close();
    return 1;
}
/**
    \fn open
*/
uint8_t avsHeader::open(const char *name)
{
   
    if(!network.bindMe(9999))
    {
        printf("[avsProxy]Open failed\n");
        return 0;
    }
    // now time to grab some info
    avsInfo info;
    avsNetPacket in,out;
    out.buffer=(uint8_t *)&info;
    out.sizeMax=sizeof(info);
    out.size=0;
    typedef struct 
    {
        uint32_t ver;
        uint32_t api;
    }version;
    version v={AVSHEADER_API_VERSION,6};
    in.buffer=(uint8_t *)&v;
    in.size=sizeof(v);
    in.sizeMax=sizeof(v);

    if(!network.command(AvsCmd_GetInfo,0,&in,&out))
    {
        printf("Get info failed\n");
        return 0;   
    }
    // Dump some info
#define PINFO(x) printf(#x":%d\n",info.x);
    PINFO( version);
	PINFO( width);
	PINFO( height);
	PINFO( fps1000);
	PINFO( nbFrames);
	PINFO( frequency);
	PINFO( channels);
    // Build header..
    _isaudiopresent = 0;	// Remove audio ATM
    _isvideopresent = 1;	// Remove audio ATM

#define CLR(x)              memset(& x,0,sizeof(  x));

    CLR(_videostream);
    CLR(_mainaviheader);

    _videostream.dwScale = 1000;
    _videostream.dwRate = info.fps1000;
    _mainaviheader.dwMicroSecPerFrame = 40000;;	// 25 fps hard coded
    _videostream.fccType = fourCC::get((uint8_t *) "YV12");

    _video_bih.biBitCount = 24;

    _videostream.dwLength = _mainaviheader.dwTotalFrames = info.nbFrames;
    _videostream.dwInitialFrames = 0;
    _videostream.dwStart = 0;
    //
    //_video_bih.biCompression= 24;
    //
    _video_bih.biWidth = _mainaviheader.dwWidth = info.width;
    _video_bih.biHeight = _mainaviheader.dwHeight = info.height;
    _video_bih.biCompression = _videostream.fccHandler =  fourCC::get((uint8_t *) "YV12");
   
    //
    if(info.frequency)
    {
        wavHeader.frequency=info.frequency;
        wavHeader.channels=info.channels;
        wavHeader.bitspersample=16;
        wavHeader.blockalign=info.channels*2;
        wavHeader.byterate=info.frequency*2*info.channels;
        wavHeader.encoding=WAV_PCM;
        audioAccess=new ADM_avsAccess(&network,&wavHeader,10000);
        _isaudiopresent=1;
        if(audioAccess)
            audioStream=ADM_audioCreateStream(&wavHeader,audioAccess);
        if(audioStream)
        {
            ADM_info("Created audio stream\n");
            haveAudio=true;
        }
        else
            ADM_warning("Error when creating audio stream\n");
    }
    printf("Connection to avsproxy succeed\n");
    return 1;
}
/**
    \fn frameToTime 
    \brief convert a give frame into time
*/
uint64_t    avsHeader::frameToTime(uint32_t frame)
{
    float f=frame;
    f*=1000000000; // Want us
    f/=_videostream.dwRate;
    uint64_t t=(uint64_t)f;
    //printf("%"LU" -> %"LLU"\n",frame,t);
    return t;
}
/**
    \fn getFrame
*/
uint8_t  avsHeader::getFrame(uint32_t framenum,ADMCompressedImage *img)
{
    uint32_t page=(_mainaviheader.dwWidth*_mainaviheader.dwHeight*3)>>1;
    
    if(framenum>=_mainaviheader.dwTotalFrames)
    {
        ADM_warning("Avisynth proxy out of bound %u / %u\n",framenum,_mainaviheader.dwTotalFrames);
        return 0;
    }
    
    avsNetPacket out;
    out.buffer=img->data;
    out.sizeMax=page;
    out.size=0;
    //printf("Asking for frame %d\n",framenum);
    if(!network.command(AvsCmd_GetFrame,framenum,NULL,&out))
    {
        ADM_error("Get frame failed for frame %u\n",framenum);
        return 0;   
    }
    ADM_assert(out.size==page);
    img->dataLength=page;
    img->demuxerDts=frameToTime(framenum);
    img->demuxerPts=img->demuxerDts;
    //printf("Frame :%"LU" Time=%"LLU"\n",framenum,img->demuxerPts);
    return 1;
}
/**
    \fn getFrame
*/

  uint8_t  avsHeader::setFlag(uint32_t frame,uint32_t flags)
{
    return 0; // All keyframes
}
/**
    \fn getFrame
*/

  uint32_t avsHeader::getFlags(uint32_t frame,uint32_t *flags)
{
    *flags=AVI_KEY_FRAME;
    if(frame>=_mainaviheader.dwTotalFrames)
    {
        ADM_warning("Avisynth proxy out of bound %u / %u\n",frame,_mainaviheader.dwTotalFrames);
        return 0;
    }
    return 1;
}
/**
    \fn getTime
*/

  uint64_t avsHeader::getTime(uint32_t frame)
{
    if(frame>=_mainaviheader.dwTotalFrames)
    {
        ADM_warning("Avisynth proxy out of bound %u / %u\n",frame,_mainaviheader.dwTotalFrames);
        return 0;
    }
    return frameToTime(frame);
}
/**
    \fn getExtraHeaderData
*/

         uint8_t  avsHeader::getExtraHeaderData(uint32_t *len, uint8_t **data)
{
    *len=0; 
    *data=NULL;
    return 1;
}
/**
    \fn getVideoDuration
*/

  uint64_t avsHeader::getVideoDuration(void)
{
    return frameToTime(_mainaviheader.dwTotalFrames);
}
/**
    \fn getPtsDts
*/

bool   avsHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    *pts=frameToTime(frame);
    *dts=*pts;
    return true;
}
/**
    \fn setPtsDts
*/

bool   avsHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    return false;
}
/**
    \fn getFrameSize
*/
uint8_t                 avsHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    if(frame>=_mainaviheader.dwTotalFrames)
    {
        ADM_warning("Avisynth proxy out of bound %u / %u\n",frame,_mainaviheader.dwTotalFrames);
        return 0;
    } 
    *size=(_mainaviheader.dwWidth*_mainaviheader.dwHeight*3)>>1;
    return true;
}

//EOF


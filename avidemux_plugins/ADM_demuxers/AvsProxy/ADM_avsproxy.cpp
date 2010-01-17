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
    mySocket=0;
}
/**
    \fn avsHeader
*/

 avsHeader::~avsHeader(  )
{
    close();   
}

/**
    \fn open
*/
uint8_t avsHeader::open(const char *name)
{
    mySocket=0;
    if(!bindMe(9999))
    {
        printf("[avsProxy]Open failed\n");
        return 0;
    }
    // now time to grab some info
    avsInfo info;
    if(!askFor(AvsCmd_GetInfo,0,sizeof(info),(uint8_t*)&info))
    {
        printf("Get info failed\n");
        return 0;   
    }
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
    
    printf("Connection to avsproxy succeed\n");
    return 1;
}
/**

*/
uint64_t    avsHeader::frameToTime(uint32_t frame)
{


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
    if(!askFor(AvsCmd_GetFrame,framenum,page,img->data))
    {
        printf("Get frame failed for frame %u\n",framenum);
        return 0;   
    }
    img->dataLength=page;
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
    return 1;
}
/**
    \fn getPtsDts
*/

bool   avsHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    float f=frame;
    
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


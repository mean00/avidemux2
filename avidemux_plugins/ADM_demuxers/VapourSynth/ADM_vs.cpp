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
#include "ADM_vs.h"
#include "ADM_vsInternal.h"
static const VSAPI *vsapi = NULL;
#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
uint32_t ADM_UsecFromFps1000(uint32_t fps1000);
extern vsDynaLoader dynaLoader;
/**
      \fn open
      \brief open the flv file, gather infos and build index(es).
*/

uint8_t vsHeader::open(const char *name)
{
    ADM_info("Opening %s as VapourSynth file\n",name);
    inited+=!!dynaLoader.init();
    if(!inited)
    {
        ADM_warning("Cannot initialize vsapi script_init. Check PYTHONPATH\n");
        return false;
    }
    if(!vsapi)
    {
        vsapi = dynaLoader.getVSApi();
        if(!vsapi)
        {
            ADM_warning("Cannot get vsAPI entry point\n");
            close();
            return 0;
        }
    }
    ADM_info("VapourSynth init ok, opening file..\n");
    if (dynaLoader.evaluateFile(&_script, name, 0)) 
    {
        ADM_warning("Evaluate script failed <%s>\n", dynaLoader.getError(_script));
        close();
        return 0;
    }
    _node = dynaLoader.getOutput(_script, 0);
    if (!_node) 
    {
        ADM_warning("vsscript_getOutputNode failed\n");
        close();
        return 0;
    }  

    const VSVideoInfo *vi = vsapi->getVideoInfo(_node);
    if(!vi)
    {
        ADM_warning("Cannot get information on node\n");
        close();
        return 0;
    }
    ADM_info("Format    : %s\n",vi->format->name);
    ADM_info("FrameRate : %d / %d\n",vi->fpsNum,vi->fpsDen);
    ADM_info("Width     : %d\n",vi->width);
    ADM_info("Height    : %d\n",vi->height);
    ADM_info("Frames    : %d\n",vi->numFrames);
    ADM_info("Flags     : 0x%x\n",vi->flags);
    
    double fps1000;
    if(vi->fpsDen)
    {
        fps1000=1000.*((double)vi->fpsNum /(double)vi-> fpsDen);
    }else
    {
        
        fps1000=25000;
    }
    //--
    _videostream.dwRate=vi->fpsNum;
    _videostream.dwScale=vi-> fpsDen;
    
    ADM_info("Fps1000=%d\n",(int)fps1000);
    
    _mainaviheader.dwMicroSecPerFrame=ADM_UsecFromFps1000(fps1000);
    _video_bih.biBitCount=24;
    _videostream.dwInitialFrames= 0;
    _videostream.dwStart= 0;
    _video_bih.biHeight=_mainaviheader.dwHeight=vi->height  ;
    _video_bih.biWidth=_mainaviheader.dwWidth=vi->width;
    _isvideopresent=true;
    _isaudiopresent=false;
    _nbFrames=vi->numFrames;
    _videostream.dwLength=_mainaviheader.dwTotalFrames=_nbFrames;
    _videostream.fccType=_videostream.fccHandler=_video_bih.biCompression=fourCC::get((uint8_t *)"YV12");
    return true;
}
/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t vsHeader::getVideoDuration(void)
{
    uint64_t d=_mainaviheader.dwMicroSecPerFrame;
    if(_nbFrames)
        d+=getTimeForFrame(_nbFrames-1);
    return d;
}
/**
 * 
 * @param frame
 * @param flags
 * @return 
 */
uint8_t  vsHeader::setFlag(uint32_t frame,uint32_t flags)
{
    return 0;
}
uint32_t vsHeader::getFlags(uint32_t frame,uint32_t *flags)
{
    *flags=AVI_KEY_FRAME;
    if(frame>=_mainaviheader.dwTotalFrames)
    {
        ADM_warning("Frame out of bounds: %u / %u\n",frame,_mainaviheader.dwTotalFrames);
        return 0;
    }
    return 1;
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
    return getTimeForFrame(frame);
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
    if(vsapi && _node)
    {
        vsapi->freeNode(_node);
        _node=NULL;
    }
    if(_script)
    {
        dynaLoader.freeScript(_script);
        _script=NULL;
    }
    while(inited)
    {
        inited--;
        dynaLoader.finalize();
    }
    vsapi=NULL;
    return 1;
}
/**
    \fn vsHeader
    \brief constructor
*/

 vsHeader::vsHeader( void ) : vidHeader()
{
    inited=0;
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
    if(frame>=_nbFrames) return 0;

    char errMsg[1024];
    const int mapp[3]={0,2,1};

    const VSFrameRef *vsframe = vsapi->getFrame(frame, _node, errMsg, sizeof(errMsg));
    if (!vsframe) 
    { 
        ADM_error("Error getting frame %d\n",frame);
        return 0;
    }   
    img->flags=AVI_KEY_FRAME;
    img->dataLength=(_mainaviheader.dwHeight*_mainaviheader.dwWidth*3)>>1;
    img->demuxerPts=getTimeForFrame(frame);
    img->demuxerDts=img->demuxerPts;
    img->demuxerFrameNo=frame; // not sure
    uint8_t *target=img->data;
#if 0    
    const VSVideoInfo *vi = vsapi->getVideoInfo(_node);
    if(!vi)
    {
        ADM_error("Error getting getVideoInfo for frame %d\n",frame);
        return false;
    }
#endif
    for (int plane = 0; plane < 3; plane++) 
    {
        int p=mapp[plane];
        int stride = vsapi->getStride(vsframe, p);
        const uint8_t *readPtr = vsapi->getReadPtr(vsframe, p);
       
        if(!readPtr)
        {
            ADM_error("Cannot get pointer for frame %p\n",p);
            return 0;
        }
        
        int rowSize = _mainaviheader.dwWidth;
        int height  = _mainaviheader.dwHeight;
        if(p)
         {
             rowSize>>=1;height>>=1;
         }
         for (int y = 0; y < height; y++) 
         {

             memcpy(target,readPtr,rowSize);
             target  += rowSize; // useless memcpy...
             readPtr += stride;
         }
    }
    vsapi->freeFrame(vsframe);
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
    if(frame>=_nbFrames) return 0;
    *size = (_video_bih.biHeight*_video_bih.biWidth*3)>>1;
    return 1;
}

/**
 * \fn getTimeForFrame
 * @param frame
 * @return 
 */
uint64_t vsHeader::getTimeForFrame(int frame)
{
    double d=1000000.;
    d*=(double)_videostream.dwScale;
    d/=(double)_videostream.dwRate;
    d*=frame;
    return (uint64_t)d;
}
/**
    \fn getPtsDts
*/
bool    vsHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    *dts=getTimeForFrame(frame);
    *pts=*dts;
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

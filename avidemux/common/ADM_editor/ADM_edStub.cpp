/***************************************************************************
                          ADM_edStub.cpp  -  description
                             -------------------
    begin                : Sat Mar 2 2002
    copyright            : (C) 2002 by mean
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
#include <math.h>


#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

#define STUBB _segments.getRefVideo(0)->_aviheader
#define ADM_TRANSLATE(func,frame) \
uint32_t ref,refOffset;\
    if(false== _segments.getRefFromFrame( frame,&ref,&refOffset))  \
    { \
        ADM_warning(#func " cannot translate fame %"LD"\n",frame); \
        return false; \
    }
/**
    \fn getSpecificMpeg4Info
	Propagate this hack to the underlying decoder
*/
uint32_t ADM_Composer::getSpecificMpeg4Info( void )
{
	if(_segments.getNbSegments())
    {
        _VIDEOS *vid=_segments.getRefVideo(0);
		if(vid->decoder)
	 		return vid->decoder->getSpecificMpeg4Info();
    }
	return 0;

}


/**
    \fn getFrame
    \brief returns the raw frame from the demuxer with len pts & dts
*/
uint8_t   ADM_Composer::getFrame (uint32_t framenum, ADMCompressedImage *img, uint8_t *isSeq)
{
  static uint32_t lastRef = 0, lastframe = 0;
  ADM_TRANSLATE(getFrame,framenum)

  if (ref)
    {
      if ((lastRef == ref) && ((lastframe + 1) == refOffset))
	{
	  *isSeq = 1;
	}
      else
	*isSeq = 0;
    }
  lastRef = ref;
  lastframe = refOffset;
  return _segments.getRefVideo(ref)->_aviheader->getFrame (refOffset,img);
}


/**
    \fn getTime
    \brief return or estimate the pts of frame fn
*/
uint64_t ADM_Composer::getTime (uint32_t fn)
{
    
    uint64_t t= STUBB->getTime(fn);
    uint32_t org=fn;
    if(t!=ADM_COMPRESSED_NO_PTS) return t;
    if(!fn) return 0;

    // Try to guess what is the time...
    while(1)
    {
        fn--;
        if(STUBB->getTime(fn)!=ADM_COMPRESSED_NO_PTS)
        {
            t=STUBB->getTime(fn);
            t+= _segments.getRefVideo(0)->timeIncrementInUs*(org-fn);
            return t;
        }
    }
    ADM_warning("[ADM_Composer::getTime] Cannot estimate time for frame %u\n",org);
    return 0;
}
/**
    \fn getFlags
*/
uint32_t ADM_Composer::getFlags (uint32_t frame, uint32_t * flags)
{
    ADM_TRANSLATE(getFlags,frame);
    return _segments.getRefVideo(ref)->_aviheader->getFlags (refOffset, flags);
}
/**
    \fn getFlagsAndSeg
*/
uint32_t ADM_Composer::getFlagsAndSeg (uint32_t frame, uint32_t * flags,uint32_t *segs)
{
 ADM_TRANSLATE(getFlagsAndSeg,frame);
*segs=0;
#warning fixme
 return _segments.getRefVideo(ref)->_aviheader->getFlags (refOffset, flags);
}
/**
    \fn getFrameSize
*/
uint8_t ADM_Composer::getFrameSize (uint32_t frame, uint32_t * size)
{
 
  ADM_TRANSLATE(getframeSize,frame);
  return _segments.getRefVideo(ref)->_aviheader->getFrameSize (refOffset, size);
}

/**
    \fn setFlag
*/
uint8_t ADM_Composer::setFlag (uint32_t frame, uint32_t flags)
{
    ADM_TRANSLATE(setFlag,frame);
    return _segments.getRefVideo(ref)->_aviheader->setFlag (refOffset, flags);
}

//
//
uint8_t ADM_Composer::updateVideoInfo (aviInfo * info)
{

  info->nb_frames = _segments.getNbFrames();
  if (info->fps1000)
    {
      uint32_t 	r,	s,	d;
      // we got 1000 * image /s
      // we want rate, scale and duration
      //
      s = 1000;
      r = info->fps1000;
      double	u;
      u = (double) info->fps1000;
      if (u < 10.)
            u = 10.;
      u = 1000000. / u;
      d = (uint32_t) floor (u);;	// 25 fps hard coded
      AVIStreamHeader *ily =	_segments.getRefVideo(0)->_aviheader->	getVideoStreamHeader ();
      ily->dwRate = r;
      ily->dwScale = s;
      rebuildDuration();
    }
  return 1;
}
/**
    \fn getVideoInfo
*/
uint8_t ADM_Composer::getVideoInfo (aviInfo * info)
{
  uint8_t    ret;
  ret = STUBB->getVideoInfo (info);
  if (ret)
    {
      info->nb_frames = _segments.getNbFrames();
    }
  return ret;
}

//______________________________
//    Info etc... to be removed later
//______________________________
AVIStreamHeader *ADM_Composer::getVideoStreamHeader (void)
{
  return STUBB->getVideoStreamHeader ();
};
MainAVIHeader *ADM_Composer::getMainHeader (void)
{
  return STUBB->getMainHeader ();
}

ADM_BITMAPINFOHEADER *ADM_Composer::getBIH (void)
{
  return STUBB->getBIH ();
};


/**
    \fn getVideoDuration
    \brief returns duration of the video track

*/
uint64_t ADM_Composer::getVideoDuration(void)
{
    return _segments.getTotalDuration();
}

//EOF

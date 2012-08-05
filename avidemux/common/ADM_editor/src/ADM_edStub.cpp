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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include <math.h>


#include "fourcc.h"
#include "ADM_edit.hxx"

#define STUBB _segments.getRefVideo(0)->_aviheader
#define ADM_TRANSLATE(func,frame) \
uint32_t ref,refOffset;\
    if(false== _segments.getRefFromFrame( frame,&ref,&refOffset))  \
    { \
        ADM_warning(#func " cannot translate fame %"PRIi32"\n",frame); \
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
 //       _VIDEOS *vid=_segments.getRefVideo(0);
//		if(vid->decoder)
//	 		return vid->decoder->getSpecificMpeg4Info();
    }
	return 0;

}

/**
    \fn updateVideoInfo

*/
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

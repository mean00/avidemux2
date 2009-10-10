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
/*
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
    printf("[ADM_Composer::getTime] Cannot estimate time for frame %u\n",org);
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
#if 0
  uint32_t    relframe;
  uint32_t    seg;
  if (!convFrame2Seg (frame, &seg, &relframe))
    return 0;
  uint32_t    ref =   _segments[seg]._reference;

    *segs=seg;
#endif
    return _segments.getRefVideo(0)->_aviheader->getFlags (frame, flags);
}
/**
    \fn getFrameSize
*/
uint8_t ADM_Composer::getFrameSize (uint32_t frame, uint32_t * size)
{
 
  ADM_TRANSLATE(getframeSize,frame);
  return _segments.getRefVideo(ref)->_aviheader->getFrameSize (refOffset, size);


}


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
      info->nb_frames = _total_frames;
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
    \fn sanityCheckRef
//	Do a sanity check for copy mode
//	Check that B frames did not loose there backward/forward ref frame
// 	It is brute force as we only need to check begin/end of each segment
//	But it should be fast anyway
*/
uint8_t		ADM_Composer::sanityCheckRef(uint32_t start, uint32_t end,uint32_t *fatal)
{
#if 0
uint32_t flags,seg;
uint32_t lastnonb=0,segnonB=0xffff;
uint32_t forward=0,forwardseg=0xffff;

uint8_t ok=0;
uint32_t i=0;
	*fatal=0;
	// If it is not in PTS, no need to bother
	if(!isReordered(start))
	{
		printf("Not reordered or no B frame, nothing to check\n");
		return 1;
	}
	// If the last frames are B frames it is fatal
	if(!getFlagsAndSeg (end-1, &flags,&seg))
	{
				printf("Cannot get flags for frame %"LU"\n",end-1);
				goto _abt;
	}
	if(flags & AVI_B_FRAME)
	{
		printf("Ending B frame -> abort (%"LU")\n",end-1);
		*fatal=1;
		return 0;
	}
	if(!getFlagsAndSeg (start, &flags,&seg))
	{
				printf("Cannot get flags for frame %"LU"\n",start);
				goto _abt;
	}
	if(flags & AVI_B_FRAME)
	{
		printf("Starting B frame -> abort\n");
		*fatal=1;
		return 0;
	}
	for( i=start;i<end;i++)
	{
			//printf("%08lu/%08lu\r",i,end-start);
			if(!getFlagsAndSeg (i, &flags,&seg))
			{
				printf("Cannot get flags for frame %"LU"\n",i);
				goto _abt;
			}
			if(flags & AVI_B_FRAME)
			{ 	// search if we have to send a I/P frame in adance
				if(segnonB!=seg)
				{
					printf("bw failed! (%"LU"/%"LU")\n",seg,segnonB);
					 goto _abt;
				}
;

				forwardseg=searchForwardSeg(i);
				if(seg!=forwardseg)
				{
					printf("Fw failed! (%"LU"/%"LU")\n",seg,forwardseg);
					 goto _abt;
				}
			}
			else // it is not a B frame and we have nothing on hold, sent it..
			{
				lastnonb=i;
				segnonB=seg;
			}
	}
	ok=1;
_abt:
	if(!ok)
	{
		printf("Frame %d has lost its fw/bw reference frame (%"LU"/%"LU")\n",i,start,end);
	}
	return ok;
#endif
    return true;
}

/**
    \fn getVideoDuration
    \brief returns duration of the video track

*/
uint64_t ADM_Composer::getVideoDuration(void)
{
    return _segments.getTotalDuration();
}

//EOF

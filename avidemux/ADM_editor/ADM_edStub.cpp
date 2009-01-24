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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_assert.h"
#include <math.h>

#include "config.h"
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

#define STUBB _videos[0]._aviheader

/*
	Propagate this hack to the underlying decoder
*/
uint32_t ADM_Composer::getSpecificMpeg4Info( void )
{
	if(_nb_segment)
		if(_videos[0].decoder)
	 		return _videos[0].decoder->getSpecificMpeg4Info();
	return 0;

}
/**
  Convert a frame as seen by the gui to a segment and offset
  in the segment
*/
uint8_t
  ADM_Composer::convFrame2Seg (uint32_t framenum,
			       uint32_t * seg, uint32_t * relframe)
{
  uint32_t curseg = 0, taq = 0;

  while (curseg < _nb_segment)
    {

      if ((framenum >= taq)
	  && (framenum < (taq + _segments[curseg]._nb_frames)))
	{
	  // gotcha
	  *seg = curseg;
	  *relframe = _segments[curseg]._start_frame + framenum - taq;
	  return 1;

	}
      taq += _segments[curseg]._nb_frames;
      curseg++;

    };
  printf("\n Frame 2 seg failed! (%"LU")\n",framenum);
  dumpSeg();
  return 0;


}

/**
  Convert a frame inside a segment as a frame as seen by GUI
  */
uint8_t
  ADM_Composer::convSeg2Frame (uint32_t * framenum,
			       uint32_t seg, uint32_t relframe)
{
  uint32_t curseg = 0;
  ADM_assert (seg < _nb_segment);
  *framenum = 0;
  while (curseg < seg)
    {

      *framenum += _segments[curseg]._nb_frames;

      curseg++;
    };
#if 0
  printf ("\n Seg: %lu Sum : %lu, relframe :%lu, start:%lu", seg,
	  *framenum, relframe, _segments[seg]._start_frame);
#endif
  *framenum += relframe - _segments[seg]._start_frame;
#if 0
  printf ("--> %lu\n", *framenum);
#endif
  return 1;


}

decoders 		*ADM_Composer::rawGetDecoder(uint32_t frame)
{
 uint32_t relframe;
  uint32_t seg;
  uint32_t ref;

  // convert frame to block, relative frame
  if (!convFrame2Seg (frame, &seg, &relframe))
    return 0;
   ref = _segments[seg]._reference;
    return _videos[ref].decoder;
}
/**
    \fn getFrame
    \brief returns the raw frame from the demuxer with len pts & dts
*/
uint8_t   ADM_Composer::getFrame (uint32_t framenum, ADMCompressedImage *img, uint8_t *isSeq)
{
  uint32_t relframe;
  uint32_t seg;
  static uint32_t lastseg = 0, lastframe = 0;
  uint32_t ref;

  // convert frame to block, relative frame
  if (!convFrame2Seg (framenum, &seg, &relframe))
    return 0;
#if 0
  printf ("\n %"LU" --> %"LU",%"LU"\n", framenum, seg, relframe);
#endif
  if (seg)
    {
      if ((lastseg == seg) && ((lastframe + 1) == relframe))
	{
	  *isSeq = 1;
	}
      else
	*isSeq = 0;
    }
  lastseg = seg;
  lastframe = relframe;
  ref = _segments[seg]._reference;
  return _videos[ref]._aviheader->getFrame (relframe,img);
}
//
// Check that the 2 frames are sequential with just B frames in between
// B > A!
// Return 1 if they are in sequence
// 0 if not
uint8_t ADM_Composer::sequentialFramesB(uint32_t frameA,uint32_t frameB)
{
	uint32_t relframeA,segA;
	uint32_t relframeB,segB,ref;
	uint32_t flags;

	ADM_assert(frameB>frameA);

	if (!convFrame2Seg (frameA, &segA, &relframeA))
  	{
  		printf("Editor: seq : convFrame2seg failed!\n");
    		return 0;
  	}
  	if (!convFrame2Seg (frameB, &segB, &relframeB))
  	{
  		printf("Editor: seq : convFrame2seg failed!\n");
    		return 0;
  	}
  	if(segA!=segB)
	{
	// printf("%"LU" %"LU" -> seg differs: %"LU",%"LU"\n",frameA,frameB,segA,segB);
	 return 0;
	}

	 ref = _segments[segA]._reference;

	for(uint32_t i=relframeA+1;i<relframeB;i++)
	{
		_videos[ref]._aviheader->getFlags(i,&flags);
		if(!(flags&AVI_B_FRAME))
		{
	//		printf("Start: %"LU" ko:%"LU" end:%"LU"\n",relframeA+1,i,relframeB);
			return 0;		// There is not only B frame between A & B
		}
	}

	return 1;

}
//_________________________________________________________________
uint8_t   ADM_Composer::isSequential (uint32_t framenum)
{
  uint32_t relframe;
  uint32_t seg;
  static uint32_t lastseg = 0, lastframe = 0;
  uint32_t ref;

  // convert frame to block, relative frame
  if (!convFrame2Seg (framenum, &seg, &relframe))
  {
  	printf("Editor: seq : convFrame2seg failed!\n");
    return 0;
  }

	if ((lastseg == seg) && ((lastframe + 1) == relframe))
	  return 1;
	else
	return 0;
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
            t+= _videos[0].timeIncrementInUs*(org-fn);
            return t;
        }
    }
    printf("[ADM_Composer::getTime] Cannot estimate time for frame %u\n",org);
    return 0;
}

uint32_t ADM_Composer::getFlags (uint32_t frame, uint32_t * flags)
{
  uint32_t
    relframe;
  uint32_t
    seg;
  if (!convFrame2Seg (frame, &seg, &relframe))
  {
        printf("Error for frame %u\n",frame);
    return 0;
  }
  uint32_t
    ref =
    _segments[seg].
    _reference;
  return _videos[ref]._aviheader->getFlags (relframe, flags);
}
uint32_t ADM_Composer::getFlagsAndSeg (uint32_t frame, uint32_t * flags,uint32_t *segs)
{
  uint32_t
    relframe;
  uint32_t
    seg;
  if (!convFrame2Seg (frame, &seg, &relframe))
    return 0;
  uint32_t    ref =   _segments[seg]._reference;

    *segs=seg;
    return _videos[ref]._aviheader->getFlags (relframe, flags);
}
/**
    \fn getFrameSize
*/
uint8_t ADM_Composer::getFrameSize (uint32_t frame, uint32_t * size)
{
  uint32_t    relframe;
  uint32_t    seg,    ref;
  if (!convFrame2Seg (frame, &seg, &relframe))
    return 0;

  ref = _segments[seg]._reference;
  return _videos[ref]._aviheader->getFrameSize (relframe, size);


}


uint8_t ADM_Composer::setFlag (uint32_t frame, uint32_t flags)
{
  uint32_t
    relframe;
  uint32_t
    seg;
  if (!convFrame2Seg (frame, &seg, &relframe))
    return 0;

  uint32_t
    ref =
    _segments[seg].
    _reference;
  return _videos[ref]._aviheader->setFlag (relframe, flags);
}

//
//
uint8_t ADM_Composer::updateVideoInfo (aviInfo * info)
{

  info->nb_frames = _total_frames;
  if (info->fps1000)
    {
      uint32_t
	r,
	s,
	d;
      // we got 1000 * image /s
      // we want rate, scale and duration
      //
      s = 1000;
      r = info->fps1000;
      double
	u;
      u = (double) info->fps1000;
      if (u < 10.)
	u = 10.;
      u = 1000000. / u;
      d = (uint32_t) floor (u);;	// 25 fps hard coded

      //getVideoStreamHeader

      AVIStreamHeader *ily =	_videos[0]._aviheader->	getVideoStreamHeader ();
      ily->dwRate = r;
      ily->dwScale = s;
      rebuildDuration();
    }
  return 1;
}

uint8_t ADM_Composer::getVideoInfo (aviInfo * info)
{
  uint8_t
    ret;
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
AVIStreamHeader *
ADM_Composer::getVideoStreamHeader (void)
{
  return STUBB->getVideoStreamHeader ();
};
MainAVIHeader *
ADM_Composer::getMainHeader (void)
{
  return STUBB->getMainHeader ();
}

ADM_BITMAPINFOHEADER *ADM_Composer::getBIH (void)
{
  return STUBB->getBIH ();
};
//
//	Do a sanity check for copy mode
//	Check that B frames did not loose there backward/forward ref frame
// 	It is brute force as we only need to check begin/end of each segment
//	But it should be fast anyway
uint8_t		ADM_Composer::sanityCheckRef(uint32_t start, uint32_t end,uint32_t *fatal)
{
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
}
// return the segment holding the next reference frame
//
uint32_t ADM_Composer::searchForwardSeg(uint32_t startframe)
{
	uint32_t fw=startframe;
	uint32_t flags,seg;
	uint8_t r;

			while(1)
			{
				fw++;
				r=getFlagsAndSeg (fw, &flags,&seg);
				if(!(flags & AVI_B_FRAME))
				{
					return seg;

				}

				if(!r)
				{
					seg=0xffff;
					return seg;
				}

			}
	return 1;
}
/**
    \fn getVideoDuration
    \brief returns duration of the video track

*/
uint64_t ADM_Composer::getVideoDuration(void)
{
  if(_nb_segment)
    return _videos[0]._aviheader->getVideoDuration();
  return 0;
}

//EOF

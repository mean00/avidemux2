/***************************************************************************
                          ADM_edSearch.cpp  -  description
                             -------------------
    begin                : Sat Apr 13 2002
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
#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

/**
    \fn searchNextKeyFrame
 //             Input : frame <b>as seen by GUI  </b>
 //     Output, segment & relframe previous keyframe
 //
*/
uint8_t   ADM_Composer::searchNextKeyFrame (uint32_t in, uint32_t * oseg,
				    uint32_t * orel)
{
#if 0
  uint32_t relframe;
  uint32_t seg, flags;
  uint8_t switched = 0;
  uint32_t ref;


  if (in >= (_total_frames - 1))
    {
      printf ("\nNKF  out of bounds\n");
      return 0;;
    }
  // convert frame to block, relative frame
  if (!convFrame2Seg (in, &seg, &relframe))
    {
      printf ("\n Conversion failed !\n");
      return 0;
    }

  while (seg < _segments.size())
    {
      ref = _segments[seg]._reference;
      // Search next kf in seg
      if (!switched)
	flags = 0;
      else
	_videos[ref]._aviheader->getFlags (relframe, &flags);


      while (!(flags & AVI_KEY_FRAME) && (relframe != 0xffffffff))
	{
	  relframe++;
	  if (!_videos[ref]._aviheader->getFlags (relframe, &flags))
	    relframe = 0xffffffff;
	}
      // verify it is within the segment

      if (relframe == 0xffffffff)
	printf ("\n not found in current seg\n");
      if (relframe <
	  (_segments[seg]._start_frame + _segments[seg]._nb_frames))
	{			// It is ...
	  *oseg = seg;
	  *orel = relframe;
	  return 1;
	}
      // Not in segment
      printf ("\n trying next seg...\n");
      seg++;
      relframe = _segments[seg]._start_frame;
      switched = 1;
    }
#endif
#warning : Obsolete!
  return 0;

}

//_________________________________________
uint8_t
  ADM_Composer::searchPreviousKeyFrame (uint32_t in, uint32_t * oseg,
					uint32_t * orel)
{
#if 0
  uint32_t relframe;
  uint32_t seg, flags;
  uint32_t ref;


  if (in == 0)
    {
      printf ("\n PKF out of bounds\n");
      return 0;;
    }
  // convert frame to block, relative frame
  if (!convFrame2Seg (in, &seg, &relframe))
    {
      printf ("\n Conversion failed !\n");
      return 0;
    }

  if (relframe == 0)		//switch to last frame of prev seg
    {
      ADM_assert (seg);
      seg--;
      relframe = _segments[seg]._nb_frames - 1;

    }

  do
    {
      ref = _segments[seg]._reference;
      // Search next kf in seg
      flags = 0;
      while (!(flags & AVI_KEY_FRAME) && (relframe > 0))
	{
	  relframe--;
	  if (!_videos[ref]._aviheader->getFlags (relframe, &flags))
          {
	    relframe = 0xffffffff;
            break;
          }
	}
      // verify it is within the segment

      if ((relframe <
	   (_segments[seg]._start_frame +
	    _segments[seg]._nb_frames)) &&
	  (relframe >= _segments[seg]._start_frame))

	{			// It is ...
	  *oseg = seg;
	  *orel = relframe;
	  return 1;
	}
      // Not in segment
      printf ("\n trying previous seg...\n");
      if (seg == 0)
	{
	  printf ("\n failed..\n");
	  return 0;
	}
      seg--;
      relframe = _segments[seg]._start_frame + _segments[seg]._nb_frames - 1;
    }
  while (1);
  printf ("\n failed pkf..\n");
#endif
#warning obsolete
  return 0;

}

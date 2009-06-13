/***************************************************************************
                          ADM_Video.cpp  -  description
                             -------------------
    begin                : Fri Apr 19 2002
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
#include <math.h>
#include "ADM_default.h"
#include "ADM_Video.h"
vidHeader::~vidHeader ()
{
  if (_name)
    delete[]_name;
  _name = NULL;
  if (_videoExtraLen)
    {
      delete[]_videoExtraData;
      _videoExtraData = NULL;
      _videoExtraLen = 0;
    }
  _name = NULL;

}

uint8_t vidHeader::getExtraHeaderData (uint32_t * len, uint8_t ** data)
{
  *len = _videoExtraLen;
  if (*len)
    {
      *data = _videoExtraData;
    }
  else
    *data = NULL;
  return 1;
}

vidHeader::vidHeader (void)
{
  _name = NULL;
  _videoExtraData = NULL;
  _videoExtraLen = 0;
}

//_______________________________________
// Get simplified information for gui
//
//_____________________________________
uint8_t vidHeader::getVideoInfo (aviInfo * info)
{
  double
    u,
    d;
  if (!_isvideopresent)
    return 0;

  info->width = _video_bih.biWidth;
  info->height = _video_bih.biHeight;
  info->nb_frames = _mainaviheader.dwTotalFrames;
  info->fcc = _videostream.fccHandler;
  info->bpp = _video_bih.biBitCount;
  u = _videostream.dwRate;
  u *= 1000.F;
  d = _videostream.dwScale;
  if (_videostream.dwScale)
    info->fps1000 = (uint32_t) floor (u / d);
  else
    info->fps1000 = 0;



  return 1;
}

uint8_t vidHeader::setMyName (const char *name)
{
  _name = new char[strlen (name) + 1];
  ADM_assert (_name);
  strcpy (_name, name);
  return 1;

}

char *
vidHeader::getMyName (void)
{
  return _name;
}

uint8_t vidHeader::getFrameSize (uint32_t frame, uint32_t * size)
{
  UNUSED_ARG (frame);
  UNUSED_ARG (size);
  return 0;
}
/**
    \fn estimatePts
    \brief Returns Pts of given frame or estimate if unknown

*/
uint64_t vidHeader::estimatePts(uint32_t frame)
{
    uint64_t pts=getTime(frame);
    if(pts!=ADM_NO_PTS) return pts; // The demuxer can provide the PTS
    // Else guesstimate...
    uint32_t count=0;
    while(frame && getTime(frame)==ADM_NO_PTS)
    {
        count++;
        frame--;
    }
    float f=_videostream.dwScale;
    f*=1000*1000;
    f/=_videostream.dwRate;
    f*=count;
    pts=getTime(frame)+count*(uint32_t)f;
}
//

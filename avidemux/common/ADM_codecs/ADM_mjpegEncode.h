/***************************************************************************
                          ADM_mjpegEncode.h  -  description
                             -------------------
    begin                : Tue Jul 23 2002
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
#ifndef __MJPEG_ENC__
#define   __MJPEG_ENC__

class mjpegEncoder:public encoder
{
protected:uint8_t _qual;
  uint8_t _swap;
public:  mjpegEncoder (uint32_t width, uint32_t height):encoder (width,
							    height)
  {
    _qual = 75;
    _swap = 0;

  };
  uint8_t stopEncoder (void);
  virtual uint8_t init (uint32_t val, uint32_t fps1000);
  uint8_t init (uint32_t val, uint32_t fps1000, uint8_t sw);
  virtual uint8_t encode (uint8_t * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags);

};

#endif

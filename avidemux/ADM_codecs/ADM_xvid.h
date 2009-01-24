/***************************************************************************
                          ADM_xvid.h.h  -  description
                             -------------------
    begin                : Fri Jul 12 2002
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

#ifdef USE_XX_XVID



#include "ADM_gui/GUI_xvidparam.h"

class xvidEncoder
{
protected:uint32_t _w, _h;
  uint8_t _init;
  void *_handle;
  uint32_t encode_flags;
  uint32_t motion_flags;

  void dumpConf (void);
  void checkFlags (xvidEncParam * extend = NULL);

public:void *getXvidStat (void);
    xvidEncoder (uint32_t width, uint32_t height)
    // : divxEncoder(width,height)
  {
    _w = width;
    _h = height;
    _init = 0;
  };
  uint8_t stopEncoder (void);
  virtual uint8_t init (uint32_t val, uint32_t fps1000) = 0;
  virtual uint8_t init (uint32_t val, uint32_t fps1000, uint32_t extra) = 0;
  virtual uint8_t initExtented (uint32_t val, xvidEncParam * extend = NULL) =
    0;
  virtual uint8_t encode (ADMImage * in,
			  uint8_t * out,
			  uint32_t * len, uint32_t * flags) = 0;


};
class xvidEncoderCBR:public xvidEncoder
{
protected:uint32_t _br;
public:xvidEncoderCBR (uint32_t width, uint32_t height):xvidEncoder (width,
								height)
  {
  };
  virtual uint8_t initExtented (uint32_t val, xvidEncParam * extend = NULL);
  virtual uint8_t init (uint32_t val, uint32_t fps1000);
  virtual uint8_t init (uint32_t val, uint32_t fps1000, uint32_t extra);
  virtual uint8_t encode (ADMImage * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags);
};
class xvidEncoderCQ:public xvidEncoder
{
protected:uint32_t _q;


public:xvidEncoderCQ (uint32_t width, uint32_t height):xvidEncoder (width,
							       height)
  {

  };
  virtual uint8_t initExtented (uint32_t val, xvidEncParam * extend = NULL);
  virtual uint8_t init (uint32_t val, uint32_t fps1000);
  virtual uint8_t init (uint32_t val, uint32_t fps1000, uint32_t extra);
  virtual uint8_t encode (ADMImage * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags);
};
class xvidEncoderVBR:public xvidEncoderCQ
{
protected: public:xvidEncoderVBR (uint32_t width,
		  uint32_t height):xvidEncoderCQ (width,
						  height)
  {

  };

  virtual uint8_t encode (ADMImage * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags);
  uint8_t encodeVBR (ADMImage * in,
		     uint8_t * out,
		     uint32_t * len,
		     uint32_t * flags, uint16_t nq, uint8_t forcekey);

};


#endif

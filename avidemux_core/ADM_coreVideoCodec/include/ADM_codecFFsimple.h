/***************************************************************************
                          ADM_ffmp43.h  -  description
                             -------------------
                             
	Mpeg4 ****decoder******** using ffmpeg
	                              
    begin                : Wed Sep 25 2002
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
#ifndef ADM_CODEC_FF_SIMPLE
#define ADM_CODEC_FF_SIMPLE
#include "ADM_default.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"

/**
    \class decoderFFSimple
*/
class decoderFFSimple:public decoderFF
{
protected:
    bool hasBFrame;

public:
  decoderFFSimple (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
  virtual const char *getDecoderName(void)  {return "lavcodec";}
  virtual bool bFramePossible (void)        {return hasBFrame; }
  
};

decoders *admCreateFFSimple(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);

#endif
// EOF


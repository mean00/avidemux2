/***************************************************************************
         \fn ADM_codec.h
         \brief Base class for all decoders
         \author mean, fixounet@free.fr (C) 2002-2010
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __ADM_CODECS__
#define __ADM_CODECS__

#include "ADM_image.h"
#include "ADM_frameType.h"
#include "ADM_codecType.h"


/*
        Bitrate in configuration will always be in **kBITS**

*/

#include "ADM_bitstream.h"
#include "ADM_compressedImage.h"
/**
    \class decoders
    \brief base class for video decoders
*/
class decoders
{
protected:
  uint32_t _w;
  uint32_t _h;
  uint8_t _lastQ;
public:
    decoders (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
  {
    _w = w;
    _h = h;
    _lastQ = 0;
  }
  virtual ~ decoders ()
  {
  };
  virtual uint8_t getPARWidth (void)
  {
    return 1;
  };
  virtual uint8_t getPARHeight (void)
  {
    return 1;
  };
  virtual bool setParam (void)
  {
        return false;
  };
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out)=0;

  // does this codec *possibly* can have b-frame ?
  virtual bool dontcopy (void)
  {
    return false;
  }				// if 1 means the decoder will return reference
  // no need to copy the datas to ADMimage
  virtual bool bFramePossible (void)
  {
    return false;
  }
  virtual bool decodeHeaderOnly (void)
  {
    return false;
  };
  virtual bool decodeFull (void)
  {
    return false;
  }
  virtual bool flush(void)
    {
        return true;
    }
};
/* This function is to be implemented by the application, it is just here for reference */
decoders *ADM_getDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, uint8_t * extraData,uint32_t bpp=0);
/* This function is implemented in coreVideoCodec, it will return a codec if it can find a suitable one, NULL if not */
decoders *ADM_coreCodecGetDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, uint8_t * extraData,uint32_t bpp=0);


#endif

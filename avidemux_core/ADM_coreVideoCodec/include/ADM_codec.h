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

#include "ADM_coreVideoCodec6_export.h"
#include "ADM_image.h"
#include "ADM_frameType.h"
#include "ADM_codecType.h"
#include "ADM_bitstream.h"
#include "ADM_compressedImage.h"
#include "ADM_confCouple.h"

/*
        Bitrate in configuration will always be in **kBITS**

*/

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

  virtual ~decoders() { }
  virtual bool initializedOk(void) { return true; }
  virtual uint8_t getPARWidth (void) { return 1; }
  virtual uint8_t getPARHeight (void) { return 1; }
  virtual bool setParam (void) { return false; }
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out) = 0;

  virtual bool getConfiguration(CONFcouple **conf) { *conf = NULL; return true; }
  virtual bool resetConfiguration() { return true; }
  virtual bool setConfiguration(CONFcouple * conf) { return true; }

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
  virtual const char *getDecoderName(void) {return "????";}
};
/* This function is to be implemented by the application, it is just here for reference */
decoders *ADM_getDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, uint8_t * extraData,uint32_t bpp=0);
/* This function is implemented in coreVideoCodec, it will return a codec if it can find a suitable one, NULL if not */
ADM_COREVIDEOCODEC6_EXPORT decoders *ADM_coreCodecGetDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, uint8_t * extraData,uint32_t bpp=0);
/**

*/
typedef enum
{
    ADM_CORE_CODEC_FEATURE_VDPAU=1,
    ADM_CORE_CODEC_FEATURE_XVBA=2,
    ADM_CORE_CODEC_FEATURE_LIBVA=3,
    ADM_CORE_CODEC_FEATURE_DXVA2=4,
}ADM_CORE_CODEC_FEATURE;

ADM_COREVIDEOCODEC6_EXPORT bool admCoreCodecSupports(ADM_CORE_CODEC_FEATURE feat);


#endif

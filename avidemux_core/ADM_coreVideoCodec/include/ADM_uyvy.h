//
// C++ Interface: ADM_uyvy
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADM_YUYV_H
#define ADM_YUYV_H
#include "ADM_codec.h"
class decoderUYVY:decoders
{
protected:

public:
  decoderUYVY (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
                :decoders (  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
  {
  };
  virtual ~ decoderUYVY ()
  {
  };
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
};
class decoderYUY2:decoders
{
protected:

public:
  decoderYUY2 (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
                :decoders (  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
  {
  };
  virtual ~ decoderYUY2 ()
  {
  };
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
};
#endif

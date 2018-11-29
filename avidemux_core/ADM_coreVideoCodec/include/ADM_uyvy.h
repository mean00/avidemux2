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
#pragma once
#include "ADM_codec.h"
/**
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
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
  bool dontcopy (void) { return 1; }
  
   virtual const char *getDecoderName(void)
  {
    return "UYVY";
  }
};
/**
 * 
 * @param w
 * @param h
 * @param fcc
 * @param extraDataLen
 * @param extraData
 * @param bpp
 */
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
  virtual const char *getDecoderName(void)
  {
    return "YUY2";
  }
  bool dontcopy (void) { return 1; }
};
// EOF

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
class decoderUYVY:decoders
{
protected:

public:
  decoderUYVY (uint32_t w, uint32_t h):decoders (w, h)
  {
  };
  virtual ~ decoderUYVY ()
  {
  };
  virtual uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
};
class decoderYUY2:decoders
{
protected:

public:
  decoderYUY2 (uint32_t w, uint32_t h):decoders (w, h)
  {
  };
  virtual ~ decoderYUY2 ()
  {
  };
  virtual uint8_t uncompress (ADMCompressedImage * in, ADMImage * out);
};

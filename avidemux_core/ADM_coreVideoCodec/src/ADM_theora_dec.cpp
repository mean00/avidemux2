/***************************************************************************
                          ADM_theora_dec.cpp  -  description
                             -------------------
    begin                : Thu Sep 26 2002
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

#include "config.h"
#include "ADM_default.h"

#ifdef USE_THEORA
#include "ADM_colorspace/colorspace.h"

#include "ADM_codecs/ADM_codec.h"

#include "ADM_codecs/ADM_theora_dec.h"
//________________________________________________
void
decoderTheora::setParam (void)
{
  return;			// no param for ffmpeg
}
//-------------------------------
decoderTheora::decoderTheora (uint32_t w, uint32_t h):decoders (w, h)
{
  memset (&_tinfo, 0, sizeof (_tinfo));
  memset (&_tstate, 0, sizeof (_tstate));

  _tinfo.width = _w;
  _tinfo.height = _h;
  _tinfo.fps_numerator = 25;
  _tinfo.fps_denominator = 1;
  _tinfo.aspect_numerator = 4;
  _tinfo.aspect_denominator = 3;
  _tinfo.quality = 9;

  _tinfo.version_major = 3;
  _tinfo.version_minor = 1;


  theora_decode_init (&_tstate, &_tinfo);
  printf ("\n Theora initialized\n");
}


//-------------------------------

uint8_t
  decoderTheora::uncompress (uint8_t * in, uint8_t * out, uint32_t len,
			     uint32_t * flagz)
{
  int got_picture = 0;

  if (len == 0)			// Null frame, silently skip
    {
      if (flagz)
	*flagz = 0;
      return 1;
    }
  ogg_packet ogg;

  memset (&ogg, 0, sizeof (ogg));
  ogg.packet = in;
  ogg.bytes = len;
  /*
     typedef struct {
     unsigned char *packet;
     long  bytes;
     long  b_o_s;
     long  e_o_s;

     ogg_int64_t  granulepos;

     ogg_int64_t  packetno;      sequence number for decode; the framing
     knows where there's a hole in the data,
     but we need coupling so that the codec
     (which is in a seperate abstraction
     layer) also knows about the gap 
     } ogg_packet; */
  if (theora_decode_packetin (&_tstate, &ogg))
    {
      printf ("\n error decoding theora ..\n");
      return 0;
    }
  yuv_buffer yuv;
  theora_decode_YUVout (&_tstate, &yuv);
  memcpy (out, yuv.y, _w * _h);
  memset (out + _w * _h, 128, (_w * _h) >> 1);





  return 1;
}

//_____________________________________________________

decoderTheora::~decoderTheora ()
{

  printf ("Theora destroyed\n");
  theora_clear (&_tstate);
}

#endif

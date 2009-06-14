/***************************************************************************
                          ADM_codecs.cpp  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr

    see here : http://www.webartz.com/fourcc/

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


#ifdef USE_FFMPEG
extern "C"
{
#include "ADM_lavcodec.h"
};
#endif
#include "ADM_default.h"
#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif

//#include "ADM_colorspace/colorspace.h"

#ifdef USE_XX_XVID
#include "xvid.h"
#endif

#include "ADM_codecs/ADM_codec.h"
#include "ADM_codecs/ADM_mjpeg.h"
#include "ADM_codecs/ADM_codecNull.h"
#include "ADM_codecs/ADM_rgb16.h"
#include "ADM_codecs/ADM_uyvy.h"
#include "avidemutils.h"
#include "fourcc.h"


#include "ADM_codecs/ADM_ffmp43.h"


#ifdef USE_THEORA
#include "ADM_codecs/ADM_theora_dec.h"
#endif

#include "ADM_codecs/ADM_png.h"

#include "ADM_assert.h"
#include "prefs.h"

extern uint8_t GUI_Question (char *);
extern uint8_t use_fast_ffmpeg;
extern bool vdpauUsable(void);

uint8_t
  decoders::uncompress (ADMCompressedImage * in, ADMImage * out)
{
  UNUSED_ARG (in);
  UNUSED_ARG (out);
  return 0;
}
decoders *
getDecoderVopPacked (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen,
		     uint8_t * extraData)
{
  UNUSED_ARG (fcc);
  UNUSED_ARG (extraLen);
  UNUSED_ARG (extraData);
  return (decoders *) (new decoderFFMpeg4VopPacked (w, h));

}
decoders *getDecoderH264noLogic (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen,
		     uint8_t * extraData)
{
  UNUSED_ARG (fcc);
  UNUSED_ARG (extraLen);
  UNUSED_ARG (extraData);
  return (decoders *) (new decoderFFH264 (w, h, extraLen, extraData,0));

}
/**
    \fn getDecoder
    \brief returns the correct decoder for a stream
*/
decoders *getDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen,
	    uint8_t * extraData,uint32_t bpp)
{
  printf("\nSearching decoder (%d x %d, extradataSize:%d)...\n",w,h,extraLen);
  if (isMSMpeg4Compatible (fcc) == 1)
    {
      // For div3, no problem we take ffmpeg

      return (decoders *) (new decoderFFDiv3 (w, h));
    }

#ifdef USE_FFMPEG
  if (isDVCompatible(fcc))//"CDVC"))
    {

      return (decoders *) (new decoderFFDV (w, h, extraLen, extraData));
    }
#endif
#ifdef USE_FFMPEG
  if (fourCC::check (fcc, (uint8_t *) "MP42"))
    {

      return (decoders *) (new decoderFFMP42 (w, h));
    }
#endif
    if (fourCC::check (fcc, (uint8_t *) "FLV1"))
    {
      return (decoders *) (new decoderFFFLV1 (w, h,extraLen, extraData));
    }


#ifdef USE_FFMPEG
  if (fourCC::check (fcc, (uint8_t *) "H263"))
    {

      return (decoders *) (new decoderFFH263 (w, h));
    }
  if (fourCC::check (fcc, (uint8_t *) "HFYU"))
    {

      return (decoders *) (new decoderFFhuff (w, h, extraLen, extraData,bpp));
    }
#ifdef USE_PNG
  if (fourCC::check (fcc, (uint8_t *) "PNG "))
    {

      return (decoders *) (new decoderPng (w, h));
    }
#endif
 if (fourCC::check (fcc, (uint8_t *) "cvid"))
    {

      return (decoders *) (new decoderFFCinepak (w, h, extraLen, extraData));
    }
  if (fourCC::check (fcc, (uint8_t *) "FFVH"))
    {

      return (decoders *) (new decoderFF_ffhuff (w, h, extraLen, extraData,bpp));
    }
  if (fourCC::check (fcc, (uint8_t *) "SVQ3"))
    {

      return (decoders *) (new decoderFFSVQ3 (w, h, extraLen, extraData));
    }
  if (fourCC::check (fcc, (uint8_t *) "tscc"))
    {

      return (decoders *) (new decoderCamtasia (w, h, bpp));
    }

     if (fourCC::check (fcc, (uint8_t *) "CRAM"))
    {

      return (decoders *) (new decoderFFCRAM (w, h, extraLen, extraData));
    }
  if (fourCC::check (fcc, (uint8_t *) "WMV2"))
    {

      return (decoders *) (new decoderFFWMV2 (w, h, extraLen, extraData));
    }
    if (fourCC::check (fcc, (uint8_t *) "WMV1"))
    {

      return (decoders *) (new decoderFFWMV1 (w, h, extraLen, extraData));
    }

  if (fourCC::check (fcc, (uint8_t *) "WMV3"))
    {

      return (decoders *) (new decoderFFWMV3 (w, h, extraLen, extraData));
    }

    if (fourCC::check (fcc, (uint8_t *) "WVC1"))
    {

      return (decoders *) (new decoderFFVC1 (w, h, extraLen, extraData));
    }

if (fourCC::check (fcc, (uint8_t *) "FFV1"))
    {

      return (decoders *) (new decoderFFV1 (w, h));
    }
  if (fourCC::check (fcc, (uint8_t *) "SNOW"))
    {

      return (decoders *) (new decoderSnow (w, h));
    }
  if (isH264Compatible (fcc))
    {
#ifdef USE_VDPAU
        if(vdpauUsable()==true)
            return (decoders *) (new decoderFFVDPAU (w, h, extraLen, extraData));
        else
#endif
            return (decoders *) (new decoderFFH264 (w, h, extraLen, extraData,1));
    }
#endif

/*
	Could be either divx5 packed crap or xvid or ffmpeg
	For now we return FFmpeg and later will switch to divx5 if available
		(ugly hack for ugly hack....)
*/

  if (isMpeg4Compatible (fcc) == 1)
    {
      return (decoders *) (new decoderFFMpeg4 (w, h, fcc,extraLen, extraData));
      //    return(decoders *)( new decoderXvid(w,h));
      //    return(decoders *)( new decoderDIVX(w,h));
    }

  if (fourCC::check (fcc, (uint8_t *) "MJPB"))
    {
      printf ("\n using FF mjpeg codec\n");
      return (decoders *) (new decoderFFMjpegB (w, h,extraLen,extraData));
    }
if (fourCC::check (fcc, (uint8_t *) "MJPG")
      || fourCC::check (fcc, (uint8_t *) "mjpa"))
    {
#if  0
      //#ifdef USE_MJPEG
      printf ("\n using mjpeg codec\n");
      return (decoders *) (new decoderMjpeg (w, h));
#else
      printf ("\n using FF mjpeg codec\n");
      return (decoders *) (new decoderFFMJPEG (w, h));
#endif

    }
  if (fourCC::check (fcc, (uint8_t *) "YV12")
      || fourCC::check (fcc, (uint8_t *) "yv12")
      || fourCC::check (fcc, (uint8_t *) "I420"))
    {
      printf ("\n using null codec\n");
      return (decoders *) (new decoderNull (w, h));
    }
  if (fourCC::check (fcc, (uint8_t *) "UYVY"))
    {
      printf ("\n using uyvy codec\n");
      return (decoders *) (new decoderUYVY (w, h));
    }
  if (fourCC::check (fcc, (uint8_t *) "YUY2"))
    {
      printf ("\n using YUY2 codec\n");
      return (decoders *) (new decoderYUY2 (w, h));
    }
  if (fourCC::check (fcc, (uint8_t *) "AMV "))
    {
      printf ("\n using AMV codec\n");
      return (decoders *) (new decoderFFAMV (w, h,extraLen,extraData));
    }


  if (isVP6Compatible(fcc))
    {
      printf ("\n using VP6F codec\n");
      return (decoders *) (new decoderFFVP6F (w, h,extraLen,extraData));
    }


  if ((fcc == 0) || fourCC::check (fcc, (uint8_t *) "RGB "))
    {
      // RGB 16 Codecs
      printf ("\n using RGB codec\n");
      return (decoders *) (new decoderRGB16 (w, h, 1, bpp));

    }
 if ((fcc == 0) || fourCC::check (fcc, (uint8_t *) "DIB "))
    {
      // RGB 16 Codecs
      printf ("\n using DIB codec\n");
      return (decoders *) (new decoderRGB16 (w, h, 0, bpp));

    }
  if (isMpeg12Compatible (fcc))
	  return (decoders *) (new decoderFFMpeg12 (w, h, extraLen, extraData));

  // default : null decoder
  printf ("\n using invalid codec for \n");
  fourCC::print (fcc);

  return (decoders *) (new decoderEmpty (w, h));
}

uint8_t coders::compress (ADMImage * in, ADMBitstream * out)
{
  UNUSED_ARG (in);
  UNUSED_ARG (out);

  return 0;
}

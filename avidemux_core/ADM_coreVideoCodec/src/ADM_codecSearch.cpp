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

#include "ADM_default.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif

#include "ADM_codec.h"
#include "ADM_codecNull.h"
#include "ADM_rgb16.h"
#include "ADM_uyvy.h"
#include "ADM_codecEmpty.h"
#include "ADM_ffmp43.h"
#include "ADM_codecFFVP9.h"
#include "ADM_codecFFsimple.h"
#include "fourcc.h"


/**
    \fn getDecoder
    \brief returns the correct decoder for a stream w,h,fcc,extraLen,extraData,bpp
*/
decoders *ADM_coreCodecGetDecoder (uint32_t fcc, uint32_t w, uint32_t h, uint32_t extraLen, uint8_t * extraData,uint32_t bpp)
{
    ADM_info("Searching decoder in coreVideoCodec(%d x %d, extradataSize:%d)...\n",w,h,extraLen);
    decoderFF *ffdec = NULL;
    if (isMSMpeg4Compatible (fcc))
        ffdec = new decoderFFDiv3 (w,h,fcc,extraLen,extraData,bpp);
    else if (isDVCompatible(fcc))//"CDVC"))
        ffdec = new decoderFFDV (w,h,fcc,extraLen,extraData,bpp);
    else if (fourCC::check (fcc, (uint8_t *) "HFYU"))
        ffdec = new decoderFFhuff (w,h,fcc,extraLen,extraData,bpp);
    else if (fourCC::check (fcc, (uint8_t *) "PNG "))
        ffdec = new decoderFFPng (w,h,fcc,extraLen,extraData,bpp);
    else if (fourCC::check (fcc, (uint8_t *) "FFVH"))
        ffdec = new decoderFF_ffhuff (w,h,fcc,extraLen,extraData,bpp);
    else if (fourCC::check (fcc, (uint8_t *) "FICV"))
        ffdec = new decoderFFficv (w,h,fcc,extraLen,extraData,bpp);
    else if (isH264Compatible (fcc))
    {
#if defined(USE_VDPAU) && 0
        if(vdpauUsable()==true)
            return (decoders *) (new decoderFFVDPAU (w,h,fcc,extraLen,extraData,bpp));
        else
#endif
        ffdec = new decoderFFH264 (w,h,fcc,extraLen,extraData,bpp);
    }
    else if (isH265Compatible (fcc))
        ffdec = new decoderFFH265 (w,h,fcc,extraLen,extraData,bpp);

/*
	Could be either divx5 packed crap or xvid or ffmpeg
	For now we return FFmpeg and later will switch to divx5 if available
		(ugly hack for ugly hack....)
*/

    else if (isMpeg4Compatible (fcc))
        ffdec = new decoderFFMpeg4 (w,h,fcc,extraLen,extraData,bpp);
    else if (isMpeg12Compatible (fcc))
    {
        if (fourCC::check (fcc, (uint8_t *) "mpg1"))
            ffdec = new decoderFFMpg1 (w,h,fcc,extraLen,extraData,bpp);
        else
            ffdec = new decoderFFMpeg12 (w,h,fcc,extraLen,extraData,bpp);
    }
    else if(isVP9Compatible(fcc))
        ffdec = new decoderFFVP9 (w,h,fcc,extraLen,extraData,bpp);

    bool ffailed=false;
    if(ffdec)
    {
        if(ffdec->initialized())
            return (decoders *)ffdec;
        delete ffdec;
        ffdec=NULL;
        ffailed=true;
    }

  if (fourCC::check (fcc, (uint8_t *) "YV12")
      || fourCC::check (fcc, (uint8_t *) "yv12")
      || fourCC::check (fcc, (uint8_t *) "I420"))
    {
      printf ("\n using null codec\n");
      return (decoders *) (new decoderNull (w,h,fcc,extraLen,extraData,bpp));
    }
  if (fourCC::check (fcc, (uint8_t *) "UYVY"))
    {
      printf ("\n using uyvy codec\n");
      return (decoders *) (new decoderUYVY (w,h,fcc,extraLen,extraData,bpp));
    }
  if (fourCC::check (fcc, (uint8_t *) "YUY2"))
    {
      printf ("\n using YUY2 codec\n");
      return (decoders *) (new decoderYUY2 (w,h,fcc,extraLen,extraData,bpp));
    } 
  if ((fcc == 0) || fourCC::check (fcc, (uint8_t *) "RGB "))
    {
      // RGB 16 Codecs
      printf ("\n using RGB codec\n");
      return (decoders *) (new decoderRGB16 (w,h,fcc,extraLen,extraData,bpp)); //1

    }
 if ((fcc == 0) || fourCC::check (fcc, (uint8_t *) "DIB "))
    {
      // RGB 16 Codecs
      if(bpp!=96)
          printf ("\n using DIB codec (%d bpp%s)\n",(int)bpp,(bpp==32)? " BGRx" : "");
      else
          printf ("\n using DIB codec (%d bpp xBGR)\n",32);
      return (decoders *) (new decoderRGB16 (w,h,fcc,extraLen,extraData,bpp));  //0

    }
    if(!ffailed)
    {
        // Search ffsimple
        decoders *dec=admCreateFFSimple(w,h,fcc,extraLen,extraData,bpp);
        if(dec)
        {
            printf("using ffSimple\n");
            return dec;
        }
    }
  // default : null decoder
  printf ("\n using invalid codec for ");
  fourCC::print (fcc);
  printf ("\n");

  return (decoders *) (new decoderEmpty(w,h,fcc,extraLen,extraData,bpp));
}
//EOF


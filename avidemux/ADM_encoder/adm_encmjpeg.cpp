/***************************************************************************
                          adm_encmjpeg.cpp  -  description
                             -------------------
    begin                : Tue Jul 23 2002
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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>


#include <time.h>
#include <sys/time.h>

#ifdef USE_FFMPEG
#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "fourcc.h"
#include "avi_vars.h"

#include "ADM_assert.h"
#include "ADM_encoder/ADM_vidEncode.hxx"


#include "ADM_videoFilter.h"
#include "ADM_codecs/ADM_ffmpeg.h"

#include "ADM_encoder/adm_encoder.h"



#include "ADM_codecs/ADM_mjpegEncode.h"


#include "ADM_encoder/adm_encmjpeg.h"

extern int getMjpegCompressParams (int *qual, int *swap);

/*_________________________________________________*/
EncoderMjpeg::EncoderMjpeg (COMPRES_PARAMS * conf)
{
  _codec = NULL;
  fd = NULL;
  entries = NULL;
  strcpy (_logname, "");
  _frametogo = 0;
  MJPEGConfig *cf = (MJPEGConfig *) conf->extraSettings;
  ADM_assert (sizeof (MJPEGConfig) == conf->extraSettingsLen);
  _q = cf->qual;
  _swapped = cf->swapped;
};
//--------------------------------
uint8_t
EncoderMjpeg::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADV_Info *info;
//         int q,s;

  ADM_assert (instream);
  _in = instream;

  info = instream->getInfo ();
  _w = info->width;
  _h = info->height;


//              _vbuffer=new uint8_t[_w*_h*3];
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);

  //   _codec=new mjpegEncoder(_w,_h);
  _codec = new ffmpegEncoderFFMjpeg (_w, _h, FF_MJPEG);

  _codec->init (_q, info->fps1000, 0);

  return 1;
}





//______________________________
uint8_t
EncoderMjpeg::encode (uint32_t frame, ADMBitstream *out)
{
  uint32_t l, f;


  ADM_assert (_codec);
  ADM_assert (_in);

  if (!_in->getFrameNumberNoAlloc (frame, &l, _vbuffer, &f))
    {
      printf ("\n Error : Cannot read incoming frame !");
      return 0;
    }
  return _codec->encode (_vbuffer, out);
}

//_______________________________
uint8_t
EncoderMjpeg::stop (void)
{
  delete _codec;
  _codec = 0;
  return 1;

}

uint8_t
EncoderMjpeg::setLogFile (const char *p, uint32_t fr)
{				// for dual pass only

  UNUSED_ARG (p);
  UNUSED_ARG (fr);
  return 1;
}
#endif

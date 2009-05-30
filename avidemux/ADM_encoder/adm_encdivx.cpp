/***************************************************************************
                          adm_encdivx.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

#ifdef USE_DIVX

#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_toolkit/toolkit.hxx"

#include "ADM_encoder/ADM_vidEncode.hxx"

//#include "ADM_video/ADM_vidEncode.hxx"

#include "ADM_codecs/ADM_divxInc.h"
#include "ADM_video/ADM_genvideo.hxx"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_codecs/ADM_divxEncode.h"
#include "ADM_encoder/adm_encdivx.h"

extern int getCompressParams (COMPRESSION_MODE * mode, uint32_t * qz,
			      uint32_t * br, uint32_t * fs);

/*_________________________________________________*/
EncoderDivx::EncoderDivx (DIVXConfig * config)
{
  _codec = NULL;
  fd = NULL;
  entries = NULL;
  strcpy (_logname, "");
  _frametogo = 0;
  memcpy (&_param, &(config->generic), sizeof (_param));
};
//--------------------------------
uint8_t
EncoderDivx::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  //  COMPRES_PARAMS par;

  ADM_assert (instream);
  ADV_Info *info;

  info = instream->getInfo ();
  _w = info->width;
  _h = info->height;
  _vbuffer = new uint8_t[_w * _h * 3];
  ADM_assert (_vbuffer);


  switch (_param.mode)
    {
    case COMPRESS_CQ:
      _state = enc_CQ;
      _codec = new divxEncoderCQ (_w, _h);
      _codec->init (_param.qz, info->fps1000);
      break;
    case COMPRESS_CBR:
      _state = enc_CBR;
      _codec = new divxEncoderCBR (_w, _h);
      _codec->init (_param.bitrate, info->fps1000);	//qz,bitrate,finalsize;
      break;
    case COMPRESS_2PASS:
      _state = enc_Pass1;
      _codec = new divxEncoderCQ (_w, _h);
      _codec->init (2, info->fps1000);
      break;
    default:
      ADM_assert (0);

    }
  _in = instream;
  printf ("\n Divx Encoder , w: %lu h:%lu mode:%d", _w, _h, _state);
  return 1;

}



uint8_t
EncoderDivx::startPass1 (void)
{
  ADM_assert (_state == enc_Pass1);
  _frametogo = 0;
  printf ("\n Starting pass 1\n");
  printf (" Creating logfile :%s\n", _logname);
  fd = fopen (_logname, "wt");
  if (!fd)
    {
      printf ("\n cannot create logfile !\n");
      return 0;
    }
  return 1;
}


uint8_t
EncoderDivx::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
    {
      return 1;
    }
  return 0;

}
uint8_t
EncoderDivx::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  return 1;

}

//______________________________
uint8_t
EncoderDivx::encode (uint32_t frame, uint32_t * len, uint8_t * out,
		     uint32_t * flags)
{
  uint32_t l, f;
  myENC_RESULT enc;

  ADM_assert (_codec);
  ADM_assert (_in);

  if (!_in->getFrameNumberNoAlloc (frame, &l, _vbuffer, &f))
    {
      printf ("\n Error : Cannot read incoming frame !");
      return 0;
    }

  switch (_state)
    {
    case enc_CBR:
    case enc_CQ:
      return _codec->encode (_vbuffer, out, len, flags);
      break;
    case enc_Pass1:

      break;

    default:
      ADM_assert (0);
    }
  return 0;
}

//_______________________________
uint8_t
EncoderDivx::stop (void)
{
  delete _codec;
  _codec = 0;
  return 1;

}

uint8_t
EncoderDivx::startPass2 (void)
{

  return 0;
}

#endif

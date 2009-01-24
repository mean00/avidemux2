/***************************************************************************
                          adm_encxvid.cpp  -  description
                             -------------------
			     Encoder for Xvid 1.0x (dev-api4)
    begin                : Sun Jul 14 2002
    copyright            : (C) 2002/2003 by mean
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


#ifdef USE_XVID_4
#include "xvid.h"

#include "fourcc.h"
#include "avi_vars.h"

#include "ADM_default.h"

#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_videoFilter.h"
#include "ADM_codecs/ADM_xvid4.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encXvid4.h"


#define aprintf printf



/*_________________________________________________*/
EncoderXvid4::EncoderXvid4 (COMPRES_PARAMS * codecconfig)
{

  _codec = NULL;
  strcpy (_logname, "");
  _frametogo = 0;
  _pass1Done = 0;

  memcpy (&_param, codecconfig, sizeof (_param));
  ADM_assert (codecconfig->extraSettingsLen == sizeof (encparam));
  memcpy (&encparam, codecconfig->extraSettings, sizeof (encparam));


};
EncoderXvid4::~EncoderXvid4 ()
{

  stop ();

};
//--------------------------------
uint8_t
EncoderXvid4::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;

  //uint32_t flag1,flag2,flag3;
  if(encparam.par_as_input)
  {
    encparam.par_width=instream->getPARWidth();
    encparam.par_height=instream->getPARHeight();
  }else
  {
      printf("[xvid] Using %u x %u aspect ratio\n",encparam.par_width,encparam.par_height);
  }
  
  //
  info = instream->getInfo ();
  _w = info->width;
  _h = info->height;
//  _vbuffer = new uint8_t[_w * _h * 2];
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;
  _fps1000 = info->fps1000;
  switch (_param.mode)
    {
    case COMPRESS_SAME:
      printf ("[xvid] Follow quant mode\n");
      _state = enc_Same;
      _codec = new xvid4EncoderVBRExternal (_w, _h);
      if (!_codec->init (2, info->fps1000, &encparam))
	{
	  printf ("[xvid] Error init Follow mode\n");
	  return 0;
	}
      break;
    case COMPRESS_CQ:
      printf ("[xvid] CQ mode: %ld\n", _param.qz);
      _state = enc_CQ;
      _codec = new xvid4EncoderCQ (_w, _h);

      if (!_codec->init (_param.qz, info->fps1000, &encparam))
	{
	  printf ("[xvid] Error init CQ mode\n");
	  return 0;
	}
      break;
    case COMPRESS_CBR:
      printf ("[xvid] CBR mode: %lu\n", _param.bitrate);
      _state = enc_CBR;

      _codec = new xvid4EncoderCBR (_w, _h);
      if (!_codec->init (_param.bitrate, info->fps1000, &encparam))
	{
	  printf ("[xvid] Error init CBR mode\n");
	  return 0;
	}
      break;
    case COMPRESS_2PASS:
    case COMPRESS_2PASS_BITRATE:
      if(_param.mode==COMPRESS_2PASS)
          printf ("[xvid] Dual size: %lu (%s)\n", _param.finalsize, _logname);
      else
          printf ("[xvid] Dual avg br: %u kb/s (%s)\n", _param.avg_bitrate, _logname);
      _state = enc_Pass1;
      _codec = new xvid4EncoderPass1 (_w, _h);
      strcpy (encparam.logName, _logname);
      printf ("[xvid] Using %s as stat file\n", encparam.logName);

      break;
    default:
      ADM_assert (0);
    }
  printf ("[xvid] Encoder ready, w: %lu h: %lu mode: %d\n", _w, _h, _state);
  return 1;

}



uint8_t EncoderXvid4::startPass1 (void)
{
  ADV_Info *
    info;
  ADM_assert (_state == enc_Pass1);
  info = _in->getInfo ();
  if (!_codec->init (_param.bitrate, info->fps1000, &encparam))
    {
      printf ("[xvid] Error init pass 1 mode\n");
      return 0;
    }
  return 1;
}



uint8_t EncoderXvid4::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
    {
      return 1;
    }
  return 0;

}

uint8_t EncoderXvid4::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  _totalframe = nbframe;
  return 1;

}

//______________________________
uint8_t
  EncoderXvid4::encode (uint32_t frame, ADMBitstream *out)
{
  uint32_t l, f, q;
  uint8_t r;
  //ENC_RESULT enc;

  ADM_assert (_codec);
  ADM_assert (_in);
  if (!_in->getFrameNumberNoAlloc (frame, &l, _vbuffer, &f))
    {
      printf ("[xvid] Error: Cannot read incoming frame!\n");
      return 0;
    }

  switch (_state)
    {
    case enc_Same:
      
      out->flags=0;
      if (frame < (encparam.bframes + 1))
	{
	  out->flags = AVI_KEY_FRAME;
	  printf ("[xvid] Forcing keyframe for B frame\n");
	}
      q = _vbuffer->_Qp;
      if (q < 2 || q > 31)
	{
	  printf ("[xvid] Out of bound incoming q: %d\n", q);
	  if (q < 2)
	    q = 2;
	  if (q > 31)
	    q = 31;
	}
        out->in_quantizer =q;

    case enc_CBR:
    case enc_CQ:
    case enc_Pass1:
    case enc_Pass2:
      r = _codec->encode (_vbuffer, out);
      return r;
      break;

    default:
      ADM_assert (0);
    }

  return 1;

}

//_______________________________
uint8_t EncoderXvid4::stop (void)
{
  if (_codec)
    delete
      _codec;
  _codec = NULL;


  return 1;

}
extern uint32_t ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);
uint8_t EncoderXvid4::startPass2 (void)
{
  uint32_t     finalSize;
  uint32_t         br;

  ADM_assert (_state == enc_Pass1);
  printf ("[xvid] Starting pass 2 (%d x %d)\n", _w, _h);

  if(_param.mode==COMPRESS_2PASS)
  {
    
    br=ADM_computeBitrate( _fps1000,_totalframe,_param.finalsize);
    printf("[xvid] Final Size: %u MB, avg bitrate %u kb/s\n",_param.finalsize,br/1000);
  }else if(_param.mode==COMPRESS_2PASS_BITRATE)
  {
    br=_param.avg_bitrate*1000;
    printf("[xvid] 2 pass avg bitrate %u kb/s\n",br/1000);
  }else ADM_assert(0);

  _state = enc_Pass2;
  // Delete codec and start new one
  if (_codec)
    {
      delete
	_codec;
      _codec = NULL;
    }

  _codec = new xvid4EncoderPass2 (_w, _h);
  strcpy (encparam.logName, _logname);
  printf ("[xvid] Using %s as stat file, average bitrate %d kbps\n", _logname,	  br / 1000);
  if (!_codec->init (br, _fps1000, &encparam))
    {
      printf ("[xvid] Error initializing pass1 mode\n");
      return 0;
    }
  _frametogo = 0;
  return 1;
}

#endif

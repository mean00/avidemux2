/***************************************************************************
                          adm_encffmpeg.cpp  -  description
                             -------------------
    begin                : Tue Sep 10 2002
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

#include <unistd.h>

#include <time.h>
#include <sys/time.h>


#ifdef USE_FFMPEG
#include "ADM_lavcodec.h"

#include "fourcc.h"
#include "avi_vars.h"
#include "DIA_coreToolkit.h"
#include "ADM_assert.h"

#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_videoFilter.h"


//#include "ADM_codecs/ADM_divxEncode.h"
#include "ADM_codecs/ADM_ffmpeg.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encffmpeg.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_ENCODER
#include "ADM_osSupport/ADM_debug.h"

#include "adm_encffmatrix.h"

#define FIRST_PASS_QUANTIZER 2


#include "ADM_libraries/ADM_xvidratectl/ADM_ratecontrol.h"

static ADM_newXvidRcVBV *_xrc = NULL;

/*_________________________________________________*/
EncoderFFMPEGMpeg1::EncoderFFMPEGMpeg1 (FF_CODEC_ID id, COMPRES_PARAMS * config):EncoderFFMPEG (id,
	       config)
{
  _frametogo = 0;
  _pass1Done = 0;
  _lastQz = 0;
  _lastBitrate = 0;
  _totalframe = 0;


  memcpy (&_param, config, sizeof (_param));
  ADM_assert (config->extraSettingsLen == sizeof (_settings));
  memcpy (&_settings, config->extraSettings, sizeof (_settings));
  _use_xvid_ratecontrol = 0;

};
uint8_t     EncoderFFMPEGMpeg1::encode (uint32_t frame, ADMBitstream *out)
{
  uint32_t l, f;
  ADM_assert (_codec);
  ADM_assert (_in);
  ADM_rframe rf;

  if (!_in->getFrameNumberNoAlloc (frame, &l, _vbuffer, &f))
    {
      printf ("\n Error : Cannot read incoming frame !");
      return 0;
    }
  switch (_state)
    {
    case enc_CQ:
    case enc_CBR:
      return _codec->encode (_vbuffer, out );
      break;
    case enc_Pass1:
      // collect result
      if (!_codec->encode (_vbuffer, out))
	{
	  printf ("\n codec error on 1st pass !");
	  return 0;
	}
      if (_use_xvid_ratecontrol)
	{
	  if (!out->len)
	    {
	      printf ("Skipping delay\n");
	      return 1;
	    }
	  switch (out->flags)
	    {
	    case AVI_KEY_FRAME:
	      rf = RF_I;
	      break;
	    case AVI_B_FRAME:
	      rf = RF_B;
	      break;
	    case 0:
	      rf = RF_P;
	      break;
	    default:
	      ADM_assert (0);

	    }
	  _xrc->logPass1 (out->out_quantizer, rf, out->len);

	}
      _frametogo++;
      return 1;
      break;
    case enc_Pass2:
      if (!_use_xvid_ratecontrol)
	{
	  if (!_codec->encode (_vbuffer, out))
	    return 0;
	  return 1;
	}

      uint32_t nq;
      uint32_t nf;
      ADM_rframe f;

      _xrc->getQz (&nq, &f);

      switch (f)
	{
	case RF_I:
	  nf = AVI_KEY_FRAME;
	  break;
	default:
	  nf = 0;
	}

#define MPEG1_MIN_Q 2
#define MPEG1_MAX_Q 28
      if (nq < MPEG1_MIN_Q)
	nq = MPEG1_MIN_Q;
      if (nq > MPEG1_MAX_Q)
	nq = MPEG1_MAX_Q;

      //printf("asked :%d ",nq);
      out->in_quantizer=nq;
      out->flags=nf;
      if (!_codec->encode (_vbuffer, out))
	return 0;
      if (!out->len)
	{
	  printf ("Skipping delay\n");
	  return 1;
	}
      {
	switch (out->flags)
	  {
	  case AVI_KEY_FRAME:
	    f = RF_I;
	    break;
	  case AVI_B_FRAME:
	    f = RF_B;
	    break;
	  case 0:
	    f = RF_P;
	    break;
	  default:
	    ADM_assert (0);

	  }
	_xrc->logPass2 (out->out_quantizer, f, out->len);
      }

      return 1;

      break;
    default:
      ADM_assert (0);
      break;
    }
  ADM_assert (0);
  return 0;

}
EncoderFFMPEGMpeg1::~EncoderFFMPEGMpeg1 ()
{
  stop ();
};



//--------------------------------
uint8_t
EncoderFFMPEGMpeg1::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{

  ADM_assert (instream);
  ADV_Info *info;
  fd = NULL;

  uint32_t flag1, flag2, flag3;
  flag1 = flag2 = flag3 = 0;

  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;
  //_vbuffer = new uint8_t[_w * _h * 3];
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;
  _use_xvid_ratecontrol = _settings.use_xvid_ratecontrol;

  switch (_param.mode)
    {
    case COMPRESS_CQ:
      printf ("ffmpeg mpeg1 cq mode: %"LU"\n", _param.qz);
      _state = enc_CQ;
      setMatrix ();		//_settings.user_matrix,_settings.gop_size);
      _codec = new ffmpegEncoderCQ (_w, _h, _id);
      _settings.override_ratecontrol = 0;
      _codec->setConfig (&_settings);
      _codec->init (_param.qz, _fps, 0);
      break;
    case COMPRESS_CBR:
      printf ("ffmpeg mpeg1 cbr mode: %"LU"\n", _param.bitrate);
      _state = enc_CBR;
      setMatrix ();
      _codec = new ffmpegEncoderCBR (_w, _h, _id);
      _settings.override_ratecontrol = 0;
      _codec->setConfig (&_settings);
      flag1 = 1;
      _codec->init (_param.bitrate * 1000, _fps, flag1);

      break;
    case COMPRESS_2PASS:
    case COMPRESS_2PASS_BITRATE:
      {
	_settings.override_ratecontrol = 1;
	ffmpegEncoderCQ *cdec = NULL;
	if (_use_xvid_ratecontrol)
	  {
	    {
	      char dummy[strlen (_logname) + 10];
	      strcpy (dummy, _logname);
	      strcat (dummy, ".fake");

	      cdec->setLogFile (dummy);
	    }
	    _xrc = new ADM_newXvidRcVBV (_fps, _logname);

	    _state = enc_Pass1;
	    printf ("\n ffmpeg dual size: %"LU" MB, using xvid rc",
		    _param.finalsize >> 10);
	    setMatrix ();
	    cdec = new ffmpegEncoderCQ (_w, _h, _id);	// Pass1

	    cdec->setConfig (&_settings);

	    cdec->init (FIRST_PASS_QUANTIZER, _fps, 1);
	    _codec = cdec;
	  }
	else
	  {
	    _state = enc_Pass1;
	    printf ("\n ffmpeg dual size: %"LU, _param.finalsize);
	    setMatrix ();
	    cdec = new ffmpegEncoderCQ (_w, _h, _id);	// Pass1

	    cdec->setConfig (&_settings);
	    cdec->setLogFile (_logname);
	    cdec->init (FIRST_PASS_QUANTIZER, _fps, 1);
	    _codec = cdec;
	  }
      }
      break;
    default:
      ADM_assert (0);
      break;

    }
  _in = instream;
  printf ("\n ffmpeg Encoder , w: %"LU" h:%"LU" mode:%d", _w, _h, _state);
  return 1;

}
uint8_t EncoderFFMPEGMpeg1::verifyLog(const char *file,uint32_t nbFrame)
{

  if(_use_xvid_ratecontrol)
  {
      return ADM_newXvidRcVBV::verifyLog(file,nbFrame);
  }else
  {
    return 0; // For now assume it is corrupted
  }

}


uint8_t
EncoderFFMPEGMpeg1::startPass1 (void)
{
  ADM_assert (_state == enc_Pass1);
  _frametogo = 0;
  printf ("\n Starting pass 1\n");
  printf (" Creating logfile :%s\n", _logname);
  _pass1Done = 1;
  if (_use_xvid_ratecontrol)
    {
      printf ("Using Xvid 2 pass rate control (%s)\n", _logname);
      _xrc->startPass1 ();

    }
  return 1;
}


uint8_t
EncoderFFMPEGMpeg1::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
    {
      return 1;
    }
  return 0;
}

uint8_t
EncoderFFMPEGMpeg1::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  _totalframe = nbframe;
  return 1;

}

//_______________________________
uint8_t
EncoderFFMPEGMpeg1::stop (void)
{
  printf ("Stopping encoder\n");
  if (_codec)
    delete _codec;
  _codec = NULL;
  if (_use_xvid_ratecontrol)
    {
      if (_state == enc_Pass1 || _state == enc_Pass2)
	{
	  if (_xrc)
	    {
	      delete _xrc;
	      _xrc = NULL;
	    }

	}
    }

  return 1;
}
//_______________________________
extern uint32_t ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);
uint8_t
EncoderFFMPEGMpeg1::startPass2 (void)
{
  uint32_t br,avg_bitrate;
  double d;

  ADM_assert (_state == enc_Pass1);
  printf ("\n-------* Starting pass 2*-------------\n");

 if(_param.mode==COMPRESS_2PASS)
  {
    br=ADM_computeBitrate( _fps,_totalframe,_param.finalsize);
    printf("[FFmpeg Mpeg1/2] Final Size: %u MB, avg bitrate %u kb/s \n",_param.finalsize,br/1000);
  }else if(_param.mode==COMPRESS_2PASS_BITRATE)
  {
    br=_param.avg_bitrate*1000;
    printf("[FFmpeg Mpeg1/2] 2pass avg bitrate %u kb/s\n",br/1000);
  }else ADM_assert(0);

  printf("[FFmpeg Mpeg1/2] Max bitrate :%u\n", (_settings.maxBitrate));
  avg_bitrate = br;
  if(_param.mode==COMPRESS_2PASS)
      printf ("\n ** Total size     : %"LU" MBytes \n", _param.finalsize);
  printf (" ** Total frame    : %"LU"  \n", _totalframe);

  printf ("\n VBR parameters computed\n");
  _state = enc_Pass2;
  // Delete codec and start new one
  if (_codec)
    {
      delete _codec;
      _codec = NULL;
    }

  if (!_use_xvid_ratecontrol)
    {
      _codec = new ffmpegEncoderVBR (_w, _h, 0, _id);	//0 -> external 1 -> internal
      _settings.override_ratecontrol = 0;
      _codec->setConfig (&_settings);

      setMatrix ();
      _codec->setLogFile (_logname);
      //_codec->setLogFile("/tmp/dummylog.txt");
      if (_settings.maxBitrate)
	if (avg_bitrate > _settings.maxBitrate*1000)
	  avg_bitrate = _settings.maxBitrate*1000;
      _codec->init (avg_bitrate, _fps);
      printf ("\n FF:ready to encode in 2pass (%s)\n", _logname);
      _frametogo = 0;
      return 1;
    }
  // ******************************************
  // If we use Xvid...
  // ******************************************
    uint32_t f;


     f=_param.finalsize;
  // Checking against max bitrate
    uint32_t maxb=_settings.maxBitrate*1000;

    printf("[FFmpeg1/2] : %u kbps average, %u max\n",br/1000,maxb/1000);
    if(br>maxb)
    {
        printf("[FFmpeg1/2] Max bitrate exceeded, clipping\n");
        br=maxb;
    }

      d=_totalframe;
      d*=1000.;
      d/=_fps;            // D is a duration in second
      d*=br;              // * bitrate = total bits
      d/=8;               // Byte
      d/=1024*1024;       // MB

      f=(uint32_t)d;


  _xrc->setVBVInfo (_settings.maxBitrate, _settings.minBitrate,
		    _settings.bufferSize);
  printf("Average bitrate :%u finale size %u\n",br,f);
  _xrc->startPass2 (f, _totalframe);


  _codec = new ffmpegEncoderVBRExternal (_w, _h, _id);	//0 -> external 1 -> internal (_w, _h);

  _settings.override_ratecontrol = 1;
  _codec->setConfig (&_settings);

  setMatrix ();
  _codec->setLogFile ("/tmp/dummylog.txt");
  _codec->init (_param.qz, _fps);
  printf ("\n XV:ready to encode in 2pass(%s)\n", _logname);
  _frametogo = 0;
  return 1;


}

//
//      Allow the user to set a custom matrix
//     The size parameter is not used
//
// *intra_matrix;
//      uint16_t *inter_matrix;
uint8_t
EncoderFFMPEGMpeg1::setMatrix (void)
{
  switch (_settings.user_matrix)
    {
    case ADM_MATRIX_DEFAULT:
      break;
    case ADM_MATRIX_TMP:
      printf ("\n using custom matrix : Tmpg\n");
      _settings.intra_matrix = tmpgenc_intra;
      _settings.inter_matrix = tmpgenc_inter;
      break;
    case ADM_MATRIX_ANIME:
      printf ("\n using custom matrix : anim\n");
      _settings.intra_matrix = anime_intra;
      _settings.inter_matrix = anime_inter;

      break;
    case ADM_MATRIX_KVCD:
      printf ("\n using custom matrix : kvcd\n");
      _settings.intra_matrix = kvcd_intra;
      _settings.inter_matrix = kvcd_inter;
      break;
    case ADM_MATRIX_HRTMP:
      printf ("\n using custom matrix : HR-Tmpgenc\n");
      _settings.intra_matrix = hrtmpgenc_intra;
      _settings.inter_matrix = hrtmpgenc_inter;
      break;
    }
  return 1;
}

#endif

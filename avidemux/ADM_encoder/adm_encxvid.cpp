/***************************************************************************
                          adm_encxvid.cpp  -  description
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
#include "config.h"
#ifdef USE_XX_XVID
#include "xvid.h"

#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_toolkit/toolkit.hxx"

#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_video/ADM_genvideo.hxx"
#include "ADM_codecs/ADM_xvid.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encxvid.h"
#include "ADM_assert.h"
extern "C"
{
#include "ADM_encoder/xvid_vbr.h"
};
#include "ADM_gui/GUI_xvidparam.h"

static vbr_control_t vbrstate;
#define aprintf printf
//static char *XvidInternal2pass_statfile=(const char *)"/tmp/xvid_int2pass.txt";

#define USE_XVID_2PASS 1

/*_________________________________________________*/
EncoderXvid::EncoderXvid (XVIDconfig * codecconfig)
{

  _codec = NULL;
  fd = NULL;
  //entries = NULL;       
  strcpy (_logname, "");
  _frametogo = 0;
  _pass1Done = 0;
  memset (&encparam, 0, sizeof (encparam));
  memset (&vbrstate, 0, sizeof (vbrstate));
  // codecconfig is of type XvidCodecConfig
  memcpy (&_param, &(codecconfig->generic), sizeof (_param));
  memcpy (&encparam, &(codecconfig->specific), sizeof (encparam));
//  _logFile=codecconfig->specific.logName;//XvidInternal2pass_statfile;

};
//--------------------------------
uint8_t EncoderXvid::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *
    info;

  //uint32_t flag1,flag2,flag3;

  info = instream->getInfo ();
  _w = info->width;
  _h = info->height;
  printf ("Configuting xvif encoder (%dx%d)\n", _w, _h);
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;



  switch (_param.mode)
    {
    case COMPRESS_CQ:
      printf ("\n Xvid cq mode: %ld", _param.qz);
      _state = enc_CQ;
      _codec = new xvidEncoderCQ (_w, _h);
      encparam.lumi = 0;
      _codec->initExtented (_param.qz, &encparam);
      break;
    case COMPRESS_CBR:
      printf ("\n Xvid cbr mode: %lu", _param.bitrate);
      _state = enc_CBR;
      encparam.lumi = 0;
      _codec = new xvidEncoderCBR (_w, _h);
      _codec->initExtented (_param.bitrate, &encparam);	//qz,bitrate,finalsize;
      break;
    case COMPRESS_2PASS:

      // initialize Xvid internal 2 pass mode

      if (0 > vbrSetDefaults (&vbrstate))
	return 0;
      vbrstate.fps = info->fps1000 / 1000.;
      // our stuff


      printf ("\n Xvid dual size: %lu", _param.finalsize);
      _state = enc_Pass1;
      _codec = new xvidEncoderCQ (_w, _h);
      _codec->initExtented (2, &encparam);

      break;
    default:
      ADM_assert (0);

    }
  _in = instream;
  printf ("\n Xvid Encoder , w: %lu h:%lu mode:%d", _w, _h, _state);
  return 1;

}



uint8_t EncoderXvid::startPass1 (void)
{
  ADM_assert (_state == enc_Pass1);
  _frametogo = 0;
  printf ("\n Starting pass 1\n");
  printf (" Creating logfile :%s\n", _logname);
  _pass1Done = 1;

  vbrstate.mode = VBR_MODE_2PASS_1;
  vbrstate.desired_size = _param.finalsize * 1024 * 1024;
  vbrstate.debug = 0;
  vbrstate.filename = _logname;	//XvidInternal2pass_statfile;


  setAdvancedOptions ();

  if (0 > vbrInit (&vbrstate))
    return 0;
  fd = fopen (_logname, "wt");
  if (!fd)
    {
      printf ("\n cannot create logfile !\n");
      return 0;
    }
  return 1;
}



uint8_t EncoderXvid::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
    {
      return 1;
    }
  return 0;

}

uint8_t EncoderXvid::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  _totalframe = nbframe;
  return 1;

}

//______________________________
uint8_t
  EncoderXvid::encode (uint32_t frame, uint32_t * len, uint8_t * out,
		       uint32_t * flags)
{
  uint32_t l, f;
  //ENC_RESULT enc;

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


      ADM_assert (fd);
      if (!_codec->encode (_vbuffer, out, len, flags))
	{
	  printf ("\n codec error on 1st pass !");
	  return 0;
	}
      myENC_RESULT enc;
      // Grab result
      _codec->getResult ((void *) &enc);

      fprintf (fd,
	       "Frame %ld: intra %d, quant %d, texture %d, motion %d, total %d\n",
	       frame, enc.is_key_frame, enc.quantizer, enc.texture_bits,
	       enc.motion_bits, enc.total_bits);

      updateStats (*len);
      _frametogo++;
      return 1;
      break;
    case enc_Pass2:

      uint16_t nq;
      uint8_t nf;

      nq = vbrGetQuant (&vbrstate);
      nf = vbrGetIntra (&vbrstate);
      //aprintf("Key : %d\n",nf);

      // Encode it !
      *flags = (nq << 8) + nf;	// ugly but help to keep interface
      if (!_codec->encode (_vbuffer, out, len, flags))
	return 0;
      updateStats (*len);
      old_bits = enc.total_bits;
      return 1;
      break;
      break;
    default:
      ADM_assert (0);
    }
  return 0;
}

//_______________________________
uint8_t EncoderXvid::stop (void)
{
  if (_codec)
    delete
      _codec;
  _codec = NULL;
  if (_state == enc_Pass1 || _state == enc_Pass2)
    {
      if (_state == enc_Pass1 && _pass1Done)
	vbrFinish (&vbrstate);
      _state = enc_Invalid;
    }

  return 1;

}

uint8_t EncoderXvid::startPass2 (void)
{

  ADM_assert (_state == enc_Pass1);
  printf ("\n Starting pass 2\n");

  // update xvid internal 2 pass engine
  if (_pass1Done)
    {
      vbrFinish (&vbrstate);	// ???
      if (fd)
	{
	  fclose (fd);
	  fd = NULL;
	}
    }
  encparam.lumi = 0;		// we reset it after first pass                    
  // switch to pass 2
  vbrstate.mode = VBR_MODE_2PASS_2;
  vbrstate.desired_size = _param.finalsize * 1024 * 1024;

  float
    br;

  br = vbrstate.desired_size * 8;
  br = br / _totalframe;	// bit / frame
  br = br * vbrstate.fps;

  vbrstate.desired_bitrate = (int) floor (br);
  vbrstate.debug = 0;
  vbrstate.filename = _logname;	//XvidInternal2pass_statfile;
  setAdvancedOptions ();

  vbrInit (&vbrstate);

  printf ("\n start 2 extra paramaters\n");

  // now read and compute
  _frametogo = _totalframe;
/*  if (!computeParameters ())
    return 0;
*/
  printf ("\n VBR paramaters computed\n");
  _state = enc_Pass2;
  old_bits = 0;
  // Delete codec and start new one
  if (_codec)
    {
      delete _codec;
      _codec = NULL;
    }
  _codec = new xvidEncoderVBR (_w, _h);
  printf ("\n ready to encode in 2pass\n");
  _codec->initExtented (2, &encparam);
  _frametogo = 0;
  return 1;

}

EncoderXvid::~EncoderXvid ()
{
  stop ();

};
	 // can be called twice if needed ..
uint8_t
EncoderXvid::updateStats (uint32_t len)
{

  myENC_RESULT enc;
  XVID_ENC_STATS *stat;
  // Update bits
  _codec->getResult (&enc);

  // update Xvid                                                                                                                  
  stat = (XVID_ENC_STATS *) _codec->getXvidStat ();
  vbrUpdate (&vbrstate,
	     stat->quant,
	     enc.is_key_frame,
	     stat->hlength, len, stat->kblks, stat->mblks, stat->ublks);

  return 1;
}
/*
  	Set Xvid 2 pass advanced options
    __________________________________
*/
void
EncoderXvid::setAdvancedOptions (void)
{

  // disable cred
  //***************************
  //***************************
  //***************************          
  //encparam.cred_rate=0;
  //***************************
  //***************************
  //***************************

  vbrstate.min_iquant = encparam.imin;
  vbrstate.max_iquant = encparam.imax;
  vbrstate.min_pquant = encparam.pmin;
  vbrstate.max_pquant = encparam.pmax;
  vbrstate.keyframe_boost = encparam.kfboost;

  printf (" I : %d/%d P : %d/%d\n",
	  vbrstate.min_iquant,
	  vbrstate.max_iquant, vbrstate.min_pquant, vbrstate.max_pquant);

  // allow specific credit handling 
  if (encparam.cred_rate)
    {
      if (encparam.startcred_end)
	{
	  vbrstate.credits_start_begin = encparam.startcred_start;
	  vbrstate.credits_start_end = encparam.startcred_end;

	  vbrstate.credits_quant_ratio = encparam.cred_rate;

	  vbrstate.credits_mode = VBR_CREDITS_MODE_RATE;
	  printf ("\n Start credit activated : From %d to %d rate %d\n",
		  encparam.startcred_start,
		  encparam.startcred_end, encparam.cred_rate);
	  vbrstate.credits_start = 1;
	}

      if (encparam.endcred_end)
	{
	  vbrstate.credits_end_begin = encparam.endcred_start;
	  vbrstate.credits_end_end = encparam.endcred_end;

	  vbrstate.credits_quant_ratio = encparam.cred_rate;

	  vbrstate.credits_mode = VBR_CREDITS_MODE_RATE;
	  printf ("\n End credit activated : From %d to %d rate %d\n",
		  encparam.endcred_start,
		  encparam.endcred_end, encparam.cred_rate);
	  vbrstate.credits_end = 1;

	}
    }

#define SETIDEM(x) {vbrstate.x=encparam.x; printf( #x": %d\n",vbrstate.x);}
  SETIDEM (min_key_interval);
  SETIDEM (bitrate_payback_delay);
  SETIDEM (curve_compression_high);
  SETIDEM (curve_compression_low);
  SETIDEM (use_alt_curve);
  SETIDEM (alt_curve_low_dist);
  SETIDEM (alt_curve_high_dist);
  SETIDEM (alt_curve_auto_str);
  SETIDEM (alt_curve_type);
  SETIDEM (bitrate_payback_method);
#undef SETIDEM
  vbrstate.alt_curve_use_auto = 1;
  vbrstate.alt_curve_use_auto_bonus_bias = 1;
  vbrstate.alt_curve_bonus_bias = 50;


}
#endif

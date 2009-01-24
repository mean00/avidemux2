/***************************************************************************
                          ADM_xvid.cpp  -  description
                             -------------------

	Use dlopen to open xvid and use it as a separate encoder
 	Decoding is still done by divx

    begin                : Fri Jul 12 2002
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

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "ADM_default.h"

#include "ADM_codecs/ADM_codec.h"
//#include "ADM_gui/GUI_xvidparam.h"

#include "ADM_assert.h"

//static myENC_RESULT xvid_res;


#ifdef USE_XX_XVID
#include "ADM_codecs/ADM_xvid.h"
#include "xvid.h"

void *dllHandle = NULL;

// api v2.0
/*
typedef int(*XVID_PROTO) (void *handle, int opt,  void *param1, void *param2);

XVID_PROTO xx_xvid_init=NULL;
XVID_PROTO xx_xvid_encore=NULL;
XVID_PROTO xx_xvid_decore=NULL;
*/
//
static XVID_ENC_FRAME fr;
static XVID_ENC_PARAM xparam;
static XVID_ENC_STATS xstats;





/*
		Returns stat in Xvid Format 
*/
void *
xvidEncoder::getXvidStat (void)
{

  return (void *) &xstats;

}
//____________________________________________
//
//  Initialize the compressor
//
uint8_t xvidEncoder::stopEncoder (void)
{
  int
    ret;

  ADM_assert (_init);

  ret = xvid_encore (_handle, XVID_ENC_DESTROY, 0, 0);
  _init = 0;
  _handle = NULL;
  if (ret == XVID_ERR_OK)
    return 1;
  return 0;
}
// Take from Xvid VFW code
static int const
  divx4_motion_presets[7] = {
  0,
  PMV_EARLYSTOP16,
  PMV_EARLYSTOP16 | PMV_ADVANCEDDIAMOND16,
  PMV_EARLYSTOP16 | PMV_HALFPELREFINE16,
  PMV_EARLYSTOP16 | PMV_HALFPELREFINE16 | PMV_EARLYSTOP8 | PMV_HALFPELREFINE8,
  PMV_EARLYSTOP16 | PMV_HALFPELREFINE16 | PMV_EARLYSTOP8 | PMV_HALFPELREFINE8,
  PMV_EARLYSTOP16 | PMV_HALFPELREFINE16 | PMV_EXTSEARCH16 | PMV_EARLYSTOP8 |
    PMV_HALFPELREFINE8
};


/* Divx4 quality to general encoder flag presets */
static int const
  divx4_general_presets[7] = {
  0,
  0,
  0,
  0 | XVID_HALFPEL,
  0 | XVID_INTER4V | XVID_HALFPEL,
  0 | XVID_INTER4V | XVID_HALFPEL,
  0 | XVID_INTER4V | XVID_HALFPEL
};



void
xvidEncoder::checkFlags (xvidEncParam * extend)
{

  if (!extend)
    {
      // default : HPel & H263Quant
      encode_flags = 0;
      encode_flags |= XVID_HALFPEL;
      encode_flags |= XVID_H263QUANT;
      motion_flags = divx4_motion_presets[4];
      return;
    }
  //
  encode_flags = 0;
  encode_flags |= extend->quantizer;
  encode_flags |= divx4_general_presets[extend->gui_option];
  motion_flags = divx4_motion_presets[extend->gui_option];
  if (extend->interlaced)
    encode_flags |= XVID_INTERLACING;
}

void
xvidEncoder::dumpConf (void)
{

#define CHK(x) if( encode_flags & XVID_##x) printf("\n"#x" is set");

  CHK (CUSTOM_QMATRIX);
  CHK (H263QUANT);
  CHK (MPEGQUANT);
  CHK (HALFPEL);
  CHK (ADAPTIVEQUANT);
  CHK (LUMIMASKING);
//CHK(LATEINTRA                   );

  CHK (INTERLACING);
  CHK (TOPFIELDFIRST);
  CHK (ALTERNATESCAN);
  CHK (HINTEDME_GET);
  CHK (HINTEDME_SET);
  CHK (INTER4V);
  CHK (ME_ZERO);
  CHK (ME_LOGARITHMIC);
  CHK (ME_FULLSEARCH);
  CHK (ME_PMVFAST);
  CHK (ME_EPZS);

#undef CHK
#define CHK(x) if( motion_flags & PMV_##x) printf("\n"#x" is set");
  CHK (EARLYSTOP16);
  CHK (ADVANCEDDIAMOND16);
  CHK (HALFPELREFINE16);
  CHK (EARLYSTOP8);
  CHK (HALFPELREFINE8);
  CHK (EXTSEARCH16);

  printf ("Max key interval : %d\n", xparam.max_key_interval);
}

// *************************************************
// *************************************************
//                                                                      CBR
// *************************************************
// *************************************************
uint8_t xvidEncoderCBR::init (uint32_t br, uint32_t fps1000)
{
  UNUSED_ARG (fps1000);
  return initExtented (br, NULL);
}
uint8_t xvidEncoderCBR::init (uint32_t br, uint32_t fps1000, uint32_t extra)
{
  UNUSED_ARG (fps1000);
  UNUSED_ARG (extra);
  return initExtented (br, NULL);
}

uint8_t xvidEncoderCBR::initExtented (uint32_t br, xvidEncParam * extend)
{
  XVID_INIT_PARAM
    xinit;
  int
    xerr;
  encode_flags = 0;
  checkFlags (extend);


  memset (&xinit, 0, sizeof (xinit));
  memset (&fr, 0, sizeof (fr));
  //---
  _br = br;
  printf (" Xvid : Compressing %lu x %lu video in CBR %lu\n", _w, _h, _br);
  ADM_assert (0 == _init);

  xinit.cpu_flags = XVID_CPU_MMX;
  xvid_init (NULL, 0, &xinit, NULL);

  xparam.width = _w;
  xparam.height = _h;
  xparam.fincr = 1;
  xparam.fbase = 25;

  xparam.rc_bitrate = _br;
  xparam.rc_reaction_delay_factor = 16;
  xparam.rc_averaging_period = 100;
  xparam.rc_buffer = 100;

  xparam.min_quantizer = 2;
  xparam.max_quantizer = 31;
  xparam.max_key_interval = extend->max_key_interval;
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xparam, NULL);
  if (XVID_ERR_OK != xerr)
    {
      printf ("\n Error initializing xvid !!!");
      return 0;
    }

  _handle = xparam.handle;
  dumpConf ();
  return 1;
}


//
//      Encode a single frame
//
uint8_t
  xvidEncoderCBR::encode (ADMImage * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags)
{

  XVID_ENC_FRAME xframe;

  int xerr;

  // general features

  xframe.general = encode_flags;
  xframe.motion = motion_flags;

  xframe.bitstream = out;
  xframe.length = -1;		// this is written by the routine

  xframe.image = in->data;
  xframe.colorspace = XVID_CSP_YV12;	// defined in <xvid.h>

  xframe.intra = -1;		// let the codec decide between I-frame (1) and P-frame (0)
  xframe.quant = 0;
//        xframe.quant = QUANTI;        // is quant != 0, use a fixed quant (and ignore bitrate)


  xerr = xvid_encore (_handle, XVID_ENC_ENCODE, &xframe, &xstats);

/*	        enc_result->is_key_frame = xframe.intra;
	        enc_result->quantizer = xframe.quant;
	        enc_result->total_bits = xframe.length * 8;
	        enc_result->motion_bits = xstats.hlength * 8;
	        enc_result->texture_bits = enc_result->total_bits - enc_result->motion_bits;
*/

  xvid_res.is_key_frame = xframe.intra;
  xvid_res.out_quantizer = xstats.quant;
  xvid_res.quantizer = xframe.quant;
  xvid_res.total_bits = xframe.length * 8;
  xvid_res.motion_bits = xstats.hlength * 8;
  xvid_res.texture_bits = xvid_res.total_bits - xvid_res.motion_bits;
/*  This is statictical data, e.g. for 2-pass.
    If you are not interested in any of this, you can use NULL instead of &xstats
*/

  *len = xframe.length;

  *flags = 0;
  if (xerr != XVID_ERR_OK)
    return 0;
  if (xframe.intra)
    {
      *flags = AVI_KEY_FRAME;
    }

  return 1;
}

// *************************************************
// *************************************************
//                                                                      CQ
// *************************************************
// *************************************************

uint8_t xvidEncoderCQ::init (uint32_t q, uint32_t fps1000)
{
  UNUSED_ARG (fps1000);
  return initExtented (q, NULL);
}
uint8_t xvidEncoderCQ::init (uint32_t q, uint32_t fps1000, uint32_t extra)
{
  UNUSED_ARG (fps1000);
  UNUSED_ARG (extra);
  return initExtented (q, NULL);
}

uint8_t xvidEncoderCQ::initExtented (uint32_t q, xvidEncParam * extend)
{
  XVID_INIT_PARAM
    xinit;
  int
    xerr;

  checkFlags (extend);
  memset (&xinit, 0, sizeof (xinit));
  memset (&fr, 0, sizeof (fr));
  //xinit.cpu_flags=XVID_CPU_FORCE;
  xinit.cpu_flags = XVID_CPU_MMX;
  //---
  _q = q;
  printf (" Xvid : Compressing %lu x %lu video in CQ %lu\n", _w, _h, _q);
  ADM_assert (0 == _init);


  xvid_init (NULL, 0, &xinit, NULL);

  xparam.width = _w;
  xparam.height = _h;
  xparam.fincr = 1;
  xparam.fbase = 25;

  xparam.rc_bitrate = 1000;
  xparam.rc_reaction_delay_factor = 16;
  xparam.rc_averaging_period = 100;
  xparam.rc_buffer = 100;


  xparam.min_quantizer = _q;
  xparam.max_quantizer = _q;
  xparam.max_key_interval = extend->max_key_interval;

  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xparam, NULL);
  if (XVID_ERR_OK != xerr)
    {
      printf ("\n Error initializing xvid !!!");
      return 0;
    }
  _handle = xparam.handle;
  dumpConf ();
  return 1;
}

//

uint8_t
  xvidEncoderCQ::encode (ADMImage * in,
			 uint8_t * out, uint32_t * len, uint32_t * flags)
{

  XVID_ENC_FRAME xframe;

  int xerr;

  memset (&xframe, 0, sizeof (xframe));

  xframe.general = encode_flags;
  xframe.motion = motion_flags;

  xframe.bitstream = out;
  xframe.length = -1;		// this is written by the routine

  xframe.image = in->data;
  xframe.colorspace = XVID_CSP_YV12;	// defined in <xvid.h>

  xframe.intra = -1;		// let the codec decide between I-frame (1) and P-frame (0)
  xframe.quant = _q;
//        xframe.quant = QUANTI;        // is quant != 0, use a fixed quant (and ignore bitrate)


  xerr = xvid_encore (_handle, XVID_ENC_ENCODE, &xframe, &xstats);

/*	        enc_result->is_key_frame = xframe.intra;
	        enc_result->quantizer = xframe.quant;
	        enc_result->total_bits = xframe.length * 8;
	        enc_result->motion_bits = xstats.hlength * 8;
	        enc_result->texture_bits = enc_result->total_bits - enc_result->motion_bits;
*/
  xvid_res.is_key_frame = xframe.intra;
  xvid_res.quantizer = xframe.quant;
  xvid_res.total_bits = xframe.length * 8;
  xvid_res.motion_bits = xstats.hlength * 8;
  xvid_res.texture_bits = xvid_res.total_bits - xvid_res.motion_bits;
  xvid_res.out_quantizer = xstats.quant;
/*  This is statictical data, e.g. for 2-pass.
    If you are not interested in any of this, you can use NULL instead of &xstats
*/

  *len = xframe.length;

  *flags = 0;
  if (xerr != XVID_ERR_OK)
    return 0;
  if (xframe.intra)
    {
      *flags = AVI_KEY_FRAME;
    }

  return 1;

}
// *************************************************
// *************************************************
//                                                                      CQ
// *************************************************
// *************************************************

uint8_t
  xvidEncoderVBR::encode (ADMImage * in,
			  uint8_t * out, uint32_t * len, uint32_t * flags)
{
  uint16_t q;
  uint8_t kf;

  q = (*flags) >> 8;
  kf = (*flags) & 1;
  return encodeVBR (in, out, len, flags, q, kf);


}
uint8_t
  xvidEncoderVBR::encodeVBR (ADMImage * in,
			     uint8_t * out,
			     uint32_t * len,
			     uint32_t * flags, uint16_t nq, uint8_t forcekey)
{

  XVID_ENC_FRAME xframe;

  int xerr;

  memset (&xframe, 0, sizeof (xframe));

  xframe.general = encode_flags;
  xframe.motion = motion_flags;

  xframe.bitstream = out;
  xframe.length = -1;		// this is written by the routine

  xframe.image = in->data;
  xframe.colorspace = XVID_CSP_YV12;	// defined in <xvid.h>

  if (forcekey)
    xframe.intra = 1;		// let the codec decide between I-frame (1) and P-frame (0)
  else
    xframe.intra = 0;		// let the codec decide between I-frame (1) and P-frame (0)

  xframe.quant = nq;
  xerr = xvid_encore (_handle, XVID_ENC_ENCODE, &xframe, &xstats);

  xvid_res.is_key_frame = xframe.intra;
  xvid_res.quantizer = xframe.quant;
  xvid_res.out_quantizer = xstats.quant;
  xvid_res.total_bits = xframe.length * 8;
  xvid_res.motion_bits = xstats.hlength * 8;
  xvid_res.texture_bits = xvid_res.total_bits - xvid_res.motion_bits;

  *len = xframe.length;

  *flags = 0;
  if (xerr != XVID_ERR_OK)
    return 0;
  if (xframe.intra)
    {
      *flags = AVI_KEY_FRAME;
    }

  return 1;

}

#endif

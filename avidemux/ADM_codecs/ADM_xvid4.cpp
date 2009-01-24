/***************************************************************************
                          ADM_xvid4.cpp  -  description
                             -------------------

	Xvid4 api

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
#ifdef USE_XVID_4

#include "ADM_codecs/ADM_xvid4param.h"
#include "ADM_codecs/ADM_xvid4.h"
#include "xvid.h"
#include "ADM_assert.h"
#include "prefs.h"

#include "ADM_editor/ADM_edit.hxx"
extern ADM_Composer *video_body;

static const int motion_presets[] = {
  0,
  0,
  0,
  0,
  XVID_ME_HALFPELREFINE16,
  XVID_ME_HALFPELREFINE16 | XVID_ME_ADVANCEDDIAMOND16,
  XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 |
    XVID_ME_HALFPELREFINE8 | XVID_ME_USESQUARES16
};

#define ME_ELEMENTS (sizeof(motion_presets)/sizeof(motion_presets[0]))


#define VOP_ELEMENTS (sizeof(vop_presets)/sizeof(vop_presets[0]))


static xvid_gbl_init_t xvid_gbl_init2;
static xvid_enc_create_t xvid_enc_create;
static xvid_plugin_single_t single;
static xvid_plugin_2pass1_t pass1;
static xvid_plugin_2pass2_t pass2;
static xvid_enc_plugin_t plugins[7];
static xvid_enc_frame_t xvid_enc_frame;
static xvid_enc_stats_t xvid_enc_stats;
static xvid_gbl_info_t xvid_gbl_info;
static uint32_t xvid_framenum;
#define MMSET(x) memset(&x,0,sizeof(x))

/******/
static int
adm_hook (void *handle, int opt, void *param1, void *param2)
{
  xvid_plg_data_t *data = (xvid_plg_data_t *) param1;
  // printf("Pass %d value %d\n",opt,data->frame_num);
  if (opt == XVID_PLG_FRAME)
    {

      xvid_framenum = data->frame_num;
      //printf("Before called with %u\n",xvid_framenum);
    }


  return 0;
}
uint8_t xvid4Encoder::encode(ADMImage * in,ADMBitstream * out)
{
    int  ret=0;
    /* Encode the frame */
    xvid_enc_frame.bitstream = out->data;
    preAmble (in->data);
    ret = xvid_encore (_handle, XVID_ENC_ENCODE, &xvid_enc_frame,
                       &xvid_enc_stats);
    if (ret < 0)
    {
        printf ("[xvid] Error calling xvid4 %d\n", ret);
        return 0;

    }
    postAmble (out);

    out->len = ret;
    if (!ret)
        printf ("[xvid] Skipped\n");
    return 1;

}


void xvid4_init (void);
/*
	System wide init, do it once for all
*/
void
xvid4_init (void)
{
  MMSET (xvid_gbl_init2);
  MMSET (xvid_gbl_info);

  printf ("[xvid] Initializing global Xvid 4\n");
  xvid_gbl_init2.version = XVID_VERSION;
  xvid_global (NULL, XVID_GBL_INIT, &xvid_gbl_init2, NULL);
  xvid_gbl_info.version = XVID_VERSION;
  xvid_global (NULL, XVID_GBL_INFO, &xvid_gbl_info, NULL);

  if (xvid_gbl_info.build)
      printf ("[xvid] Build: %s\n", xvid_gbl_info.build);

  printf ("[xvid] SIMD supported: (%x)\n", xvid_gbl_info.cpu_flags);
#define CPUF(x) if(xvid_gbl_info.cpu_flags  & XVID_CPU_##x) printf("\t\t"#x"\n");
#ifdef ADM_CPU_X86
  CPUF (MMX);
  CPUF (MMXEXT);
  CPUF (SSE);
  CPUF (SSE2);
  CPUF (3DNOW);
  CPUF (3DNOWEXT);
#endif
#ifdef ADM_CPU_ALTIVEC
  CPUF (ALTIVEC);
#endif

}
xvid4Encoder::~xvid4Encoder ()
{
  stopEncoder ();
}
//--------------------------------------------------

uint8_t
  xvid4Encoder::init (uint32_t q, uint32_t fps1000, xvid4EncParam * param)
{
  return 0;
}


void
xvid4Encoder::createUpdate (void)
{
  uint32_t threads = 0;

  prefs->get(FEATURE_THREADING_XVID, &threads);

  if (threads == 0)
	  xvid_enc_create.num_threads = xvid_gbl_info.num_threads;
  else if (threads > 1)
	  xvid_enc_create.num_threads = threads;
  else
      xvid_enc_create.num_threads = 0;

  if (xvid_enc_create.num_threads)
      printf ("[xvid] using %u threads\n", xvid_enc_create.num_threads);

  xvid_enc_create.max_bframes = _param.bframes;
  xvid_enc_create.max_key_interval = _param.max_key_interval;
  xvid_enc_create.bquant_ratio = _param.bquant_ratio;
  xvid_enc_create.bquant_offset = _param.bquant_offset;

  if (_param.packed)
    xvid_enc_create.global |=XVID_GLOBAL_PACKED;

  if (_param.closed_gop)
    xvid_enc_create.global |=XVID_GLOBAL_CLOSED_GOP;

  for (uint32_t i = 0; i < 3; i++)
    {
      xvid_enc_create.min_quant[i] = _param.qmin[i];
      xvid_enc_create.max_quant[i] = _param.qmax[i];
    }

  preAmble (NULL);
#define C_VOP(x) if(xvid_enc_frame.vop_flags& XVID_VOP_##x) printf("[xvid] Vop: "#x" is set\n");
#define C_VOL(x) if(xvid_enc_frame.vol_flags& XVID_VOL_##x) printf("[xvid] Vol: "#x" is set\n");
#define C_ME(x) if(xvid_enc_frame.motion& XVID_ME_##x) printf("[xvid] ME: "#x" is set\n");

  C_VOP (DEBUG);
  C_VOP (CHROMAOPT);
  C_VOP (HALFPEL);
  C_VOP (INTER4V);
  C_VOP (TRELLISQUANT);
  C_VOP (CHROMAOPT);
  C_VOP (CARTOON);
  C_VOP (GREYSCALE);
  C_VOP (HQACPRED);
  C_VOP (MODEDECISION_RD);

  C_VOL (MPEGQUANT);
  C_VOL (EXTRASTATS);
  C_VOL (GMC);
  C_VOL (QUARTERPEL);
  C_VOL (INTERLACING);

  C_ME (ADVANCEDDIAMOND16);
  C_ME (ADVANCEDDIAMOND8);

  C_ME (USESQUARES16);
  C_ME (USESQUARES8);

  C_ME (HALFPELREFINE16);
  C_ME (HALFPELREFINE8);

  // Turbo Mode
  C_ME (FASTREFINE16);
  C_ME (FASTREFINE8);
  C_ME (SKIP_DELTASEARCH);
  C_ME (FAST_MODEINTERPOLATE);
  C_ME (BFRAME_EARLYSTOP);

  C_ME (GME_REFINE);
  C_ME (QUARTERPELREFINE8);
  C_ME (QUARTERPELREFINE16);
  C_ME (EXTSEARCH16);
  C_ME (EXTSEARCH8);
  C_ME (CHROMA_BVOP);
  C_ME (CHROMA_PVOP);
  MMSET (xvid_enc_frame);


}

void
xvid4Encoder::dump (void)
{
  printf ("[xvid] Configuration:\n");
  if (_param.mpegQuantizer)
    printf ("\t\tMPEGQUANT\n");
  if (_param.interlaced)
    printf ("\t\tINTERLACED\n");


#define ST(x,y) \
	if(_param.x) printf("\t\t"#y"\n");


  ST (inter4mv, INTER4V);
  ST (trellis, TRELLISQUANT);
  ST (cartoon, CARTOON);
  ST (greyscale, GREYSCALE);
#undef ST

}
//____________________________________________
//
//  Initialize the compressor
//
uint8_t
xvid4Encoder::stopEncoder (void)
{
  if (_handle)
    {
      xvid_encore (_handle, XVID_ENC_DESTROY, NULL, NULL);
      printf ("[xvid] Destroying\n");
      _handle = NULL;
    }
  return 1;
}
uint8_t
xvid4Encoder::preAmble (uint8_t * in)
{
/* Version for the frame and the stats */
  //MMSET (xvid_enc_frame);
  MMSET (xvid_enc_stats);

  xvid_enc_frame.version = XVID_VERSION;
  xvid_enc_stats.version = XVID_VERSION;

  /* Bind output buffer */

  xvid_enc_frame.length = 0;
  if (_param.mpegQuantizer || _param.useCustomIntra || _param.useCustomInter)
    xvid_enc_frame.vol_flags |= XVID_VOL_MPEGQUANT;
  if (_param.interlaced)
    xvid_enc_frame.vol_flags |= XVID_VOL_INTERLACING;


#define SVOP(x,y) if(_param.x) xvid_enc_frame.vop_flags|=XVID_VOP_##y


  xvid_enc_frame.motion = motion_presets[_param.guiLevel];

  SVOP (inter4mv, INTER4V);
  SVOP (trellis, TRELLISQUANT);
  SVOP (hqac, HQACPRED);
#if  XVID_API==XVID_MAKE_API(4, 1)
  SVOP (bvhq, RD_BVOP);
#endif

  SVOP (greyscale, GREYSCALE);
  SVOP (cartoon, CARTOON);
  SVOP (chroma_opt, CHROMAOPT);
  //SVOP(  1      ,HALFPEL);
  xvid_enc_frame.vop_flags |= XVID_VOP_HALFPEL;
  // ME
  if (_param.chroma_me)
    {
      xvid_enc_frame.motion |= XVID_ME_CHROMA_BVOP;
      xvid_enc_frame.motion |= XVID_ME_CHROMA_PVOP;
    }
  if (_param.cartoon)
    {
      xvid_enc_frame.motion |= XVID_ME_DETECT_STATIC_MOTION;
    }
  //printf("Vhq mode : %d\n",_param.vhqmode);
#define VHME(x) xvid_enc_frame.motion|=	XVID_ME_##x
  switch (_param.vhqmode)
    {
    case 4:
      VHME (EXTSEARCH_RD);
    case 3:
      VHME (HALFPELREFINE8_RD);
      VHME (QUARTERPELREFINE8_RD);
      VHME (CHECKPREDICTION_RD);

    case 2:
      VHME (HALFPELREFINE16_RD);
      VHME (QUARTERPELREFINE16_RD);
    case 1:
      SVOP (vhqmode, MODEDECISION_RD);
    case 0:
      break;
    default:
      ADM_assert (0);
    }
  if (_param.qpel)
    {
      xvid_enc_frame.vol_flags |= XVID_VOL_QUARTERPEL;

      xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16;
      xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE8;
    }

  if (_param.gmc)
    {
      xvid_enc_frame.vol_flags |= XVID_VOL_GMC;
      xvid_enc_frame.motion |= XVID_ME_GME_REFINE;
    }

  if (_param.turbo)
    {
      xvid_enc_frame.motion |= XVID_ME_FASTREFINE16;
      xvid_enc_frame.motion |= XVID_ME_FASTREFINE8;
      xvid_enc_frame.motion |= XVID_ME_SKIP_DELTASEARCH;
      xvid_enc_frame.motion |= XVID_ME_FAST_MODEINTERPOLATE;
      xvid_enc_frame.motion |= XVID_ME_BFRAME_EARLYSTOP;
    }

  xvid_enc_frame.bframe_threshold = _param.bframe_threshold;

  xvid_enc_frame.input.csp = XVID_CSP_YV12;
  xvid_enc_frame.input.stride[0] = _w;
  xvid_enc_frame.input.stride[1] = _w >> 1;
  xvid_enc_frame.input.stride[2] = _w >> 1;
  xvid_enc_frame.type = XVID_TYPE_AUTO;


  /* Set up motion estimation flags */
  xvid_enc_frame.input.plane[0] = in;
  xvid_enc_frame.input.plane[1] = in + (_w * _h);
  xvid_enc_frame.input.plane[2] = in + ((_w * _h * 5) >> 2);

  xvid_enc_frame.par_width = _param.par_width;
  xvid_enc_frame.par_height = _param.par_height;
  //printf("Using AR : %u x %u\n",xvid_enc_frame.par_width,xvid_enc_frame.par_height );
  if (xvid_enc_frame.par_width != xvid_enc_frame.par_height)
      xvid_enc_frame.par = XVID_PAR_EXT;
  else
      xvid_enc_frame.par = XVID_PAR_11_VGA;

  /* Custome matrices */
  if(_param.useCustomIntra)
  {
  if(!xvid_enc_frame.quant_intra_matrix)
      printf("[xvid] Using custom intra matrix\n");
      xvid_enc_frame.quant_intra_matrix=_param.intraMatrix;
  }
  if(_param.useCustomInter)
  {
    if(!xvid_enc_frame.quant_inter_matrix)
      printf("[xvid] Using custom inter matrix\n");
     xvid_enc_frame.quant_inter_matrix=_param.interMatrix;
  }
  return 1;
}

uint8_t
xvid4Encoder::postAmble (ADMBitstream * out)
{
  out->flags = 0;
  if (xvid_enc_frame.out_flags & XVID_KEYFRAME)
    {
      out->flags = AVI_KEY_FRAME;
    }
  else if (xvid_enc_stats.type == XVID_TYPE_BVOP)
    {
      out->flags = AVI_B_FRAME;

    }
  out->out_quantizer = xvid_enc_stats.quant;
  out->ptsFrame= xvid_framenum;
  return 1;
}

// *************************************************
// *************************************************
//                                                                      CQ
// *************************************************
// *************************************************
uint8_t
  xvid4EncoderCQ::init (uint32_t q, uint32_t fps1000, xvid4EncParam * param)
{
  int xerr;
  printf ("[xvid] CQ mode = %lu (%d x %d)\n", q, _w, _h);
  _q = q;
  _fps1000 = fps1000;
  memcpy (&_param, param, sizeof (_param));
  dump ();
  // Here we go...
  MMSET (xvid_enc_create);
  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = _w;
  xvid_enc_create.height = _h;
  MMSET (single);


  plugins[0].func = xvid_plugin_single;
  plugins[0].param = &single;

  plugins[1].func = adm_hook;
  plugins[1].param = NULL;


  single.version = XVID_VERSION;
  single.bitrate = 1500;

  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 2;

  //Framerate
  xvid_enc_create.fincr = 1000;
  xvid_enc_create.fbase = _fps1000;
  //
  createUpdate ();
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
  if (xerr < 0)
    {
      printf ("[xvid] init error: %d\n", xerr);
      return 0;

    }

  _handle = xvid_enc_create.handle;

  printf ("[xvid] CQ init Ok\n");

  return 1;
}


uint8_t
xvid4EncoderCQ::encode (ADMImage * in, ADMBitstream * out)
{
  int ret;

  xvid_enc_frame.quant = _q;
  return xvid4Encoder::encode(in,out);

}
// *************************************************
// *************************************************
//              CQ but Q is controlled externally
// *************************************************
// *************************************************

uint8_t
  xvid4EncoderVBRExternal::init (uint32_t q, uint32_t fps1000,
				 xvid4EncParam * param)
{
  int xerr;
  printf ("[xvid] CQ mode = %lu (%d x %d)\n", q, _w, _h);
  _q = q;
  _fps1000 = fps1000;
  memcpy (&_param, param, sizeof (_param));
  dump ();
  // Here we go...
  MMSET (xvid_enc_create);
  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = _w;
  xvid_enc_create.height = _h;
  MMSET (single);


  plugins[0].func = xvid_plugin_single;
  plugins[0].param = &single;

  plugins[1].func = adm_hook;
  plugins[1].param = NULL;

  single.version = XVID_VERSION;
  single.bitrate = 1500;

  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 2;

  //Framerate
  xvid_enc_create.fincr = 1000;
  xvid_enc_create.fbase = _fps1000;
  //
  createUpdate ();
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
  if (xerr < 0)
    {
      printf ("[xvid] init error: %d\n", xerr);
      return 0;

    }

  _handle = xvid_enc_create.handle;

  printf ("[xvid] CQ init Ok\n");

  return 1;
}




uint8_t
  xvid4EncoderVBRExternal::encode (ADMImage * in,
				      ADMBitstream * out)
{
  xvid_enc_frame.quant = out->in_quantizer;
  if (out->flags & AVI_KEY_FRAME)
    {
      xvid_enc_frame.type = XVID_TYPE_IVOP;
    }
  else if (out->flags & AVI_B_FRAME)
    {
      xvid_enc_frame.type = XVID_TYPE_BVOP;
    }
  else
    {				//P
      xvid_enc_frame.type = XVID_TYPE_PVOP;
    }
    return xvid4Encoder::encode(in,out);
}
// *************************************************
// *************************************************
//              CBR
// *************************************************
// *************************************************
uint8_t
  xvid4EncoderCBR::init (uint32_t br, uint32_t fps1000, xvid4EncParam * param)
{
  int xerr;
  printf ("[xvid] CBR mode = %lu kbps\n", br);
  _bitrate = br;
  _fps1000 = fps1000;
  memcpy (&_param, param, sizeof (_param));
  dump ();
  // Here we go...
  MMSET (single);
  MMSET (xvid_enc_create);

  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = _w;
  xvid_enc_create.height = _h;


  plugins[0].func = xvid_plugin_single;
  plugins[0].param = &single;

  plugins[1].func = adm_hook;
  plugins[1].param = NULL;

  single.version = XVID_VERSION;
  single.bitrate = _bitrate * 1000;	// Kb->bit

  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 2;

  //Framerate
  xvid_enc_create.fincr = 1000;
  xvid_enc_create.fbase = _fps1000;
  //
  createUpdate ();
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
  if (xerr < 0)
    {
      printf ("[xvid] Init error: %d\n", xerr);
      return 0;

    }

  _handle = xvid_enc_create.handle;

  printf ("[xvid] CBR init Ok\n");

  return 1;
}


uint8_t
xvid4EncoderCBR::encode (ADMImage * in, ADMBitstream * out)
{
  xvid_enc_frame.quant = 0;
  return xvid4Encoder::encode(in,out);
}
// *************************************************
// *************************************************
//              Pass1
// *************************************************
// *************************************************
uint8_t
  xvid4EncoderPass1::init (uint32_t br, uint32_t fps1000,
			   xvid4EncParam * param)
{
  int xerr;
  printf ("[xvid] Pass 1 of 2 (log: %s)\n",
	  (char *) param->logName);

  _fps1000 = fps1000;
  memcpy (&_param, param, sizeof (_param));
  dump ();
  // Here we go...
  MMSET (xvid_enc_create);
  MMSET (pass1);

  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = _w;
  xvid_enc_create.height = _h;



  plugins[0].func = xvid_plugin_2pass1;
  plugins[0].param = &pass1;


  pass1.version = XVID_VERSION;
  pass1.filename = (char *) _param.logName;

  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 1;


  //Framerate
  xvid_enc_create.fincr = 1000;
  xvid_enc_create.fbase = _fps1000;
  //
  createUpdate ();
  // Force Qz=2 !
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
  if (xerr < 0)
    {
      printf ("[xvid] Init error: %d\n", xerr);
      return 0;

    }

  _handle = xvid_enc_create.handle;

  printf ("[xvid] Pass 1 init Ok\n");

  return 1;
}



// *************************************************
// *************************************************
//              Pass2
// *************************************************
// *************************************************
uint8_t
  xvid4EncoderPass2::init (uint32_t br, uint32_t fps1000,
			   xvid4EncParam * param)
{
  int xerr;
  printf
    ("[xvid] Pass 2 of 2, average bitrate = %lu\n",
     br);
  _bitrate = br;
  _fps1000 = fps1000;
  memcpy (&_param, param, sizeof (_param));
  dump ();
  // Here we go...
  MMSET (xvid_enc_create);
  MMSET (pass2);

  xvid_enc_create.version = XVID_VERSION;
  xvid_enc_create.width = _w;
  xvid_enc_create.height = _h;



  plugins[0].func = xvid_plugin_2pass2;
  plugins[0].param = &pass2;
  plugins[1].func = adm_hook;
  plugins[1].param = NULL;

  pass2.version = XVID_VERSION;
  pass2.filename = (char *) _param.logName;

  pass2.bitrate = br;		// Average bitrate
#define CPY(x) pass2.x=_param.x;printf("[xvid] "#x"=%d\n",pass2.x);

  CPY (keyframe_boost);
  CPY (curve_compression_high);
  CPY (curve_compression_low);
  CPY (overflow_control_strength);
  CPY (max_overflow_improvement);
  CPY (max_overflow_degradation);
  CPY (kfreduction);
  CPY (kfthreshold);

  CPY (container_frame_overhead);



  xvid_enc_create.plugins = plugins;
  xvid_enc_create.num_plugins = 2;

  //Framerate
  xvid_enc_create.fincr = 1000;
  xvid_enc_create.fbase = _fps1000;
  //
  createUpdate ();
  xerr = xvid_encore (NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
  if (xerr < 0)
    {
      printf ("[xvid] Init error: %d\n", xerr);
      return 0;

    }

  _handle = xvid_enc_create.handle;

  printf ("[xvid] Init Ok\n");

  return 1;
}


#endif

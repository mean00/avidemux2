/***************************************************************************
                          ADM_ffmpeg.cpp  -  description
                             -------------------

	Encoder for ffmpeg
	Reverse enginereed from mplayer & transcode


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



#define __STDC_CONSTANT_MACROS // Lavcodec crap

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_lavcodec.h"
#include "avi_vars.h"
#include "DIA_coreToolkit.h"
#include "ADM_assert.h"
#include "ADM_codecs/ADM_ffmpeg.h"

#include "prefs.h"

#include "ADM_video/ADM_vidMisc.h"
//#define TEST_NOB 1

extern int ADM_cpu_num_processors(void);
static char LogName[500];

#define WRAP_Open(x) \
{\
AVCodec *codec=avcodec_find_encoder(x);\
if(!codec) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error opening codec"#x));ADM_assert(0);} \
  res=avcodec_open(_context, codec); \
  if(res<0) {GUI_Error_HIG("Codec",QT_TR_NOOP("Internal error with context for  codec"#x".\n Did you use too low / too high target for 2 pass ?"));return 0;} \
}
/*****************************************/
uint8_t ffmpegEncoder::stopEncoder(void)
{
    printf("[lavc] stopEncoder (%x)\n",_context);
    if (_context)
    {
        avcodec_close (_context);
        ADM_dealloc (_context);
        _context = NULL;
    }
    return 1;
}
/*****************************************/
ffmpegEncoderCQ::~ffmpegEncoderCQ ()
  {
    printf("[LAVCODEC]Deleting ffmpegCQencoder\n");
    //stopEncoder ();
    if (_statfile)
      {
        fflush(_statfile);
	fclose (_statfile);
      }
  }
/*****************************************/
void ffmpegEncoder::postAmble (ADMBitstream * out, uint32_t sz)
{
  out->ptsFrame = _context->coded_frame->display_picture_number; //real_pict_num;;
 //printf("Out : %u\n",out->ptsFrame);
  out->len = (uint32_t) sz;
  out->flags = frameType ();;
  if(!_context->coded_frame->quality)
      out->out_quantizer=(int) floor (_frame.quality / (float) FF_QP2LAMBDA);
  else
      out->out_quantizer =(int) floor (_context->coded_frame->quality / (float) FF_QP2LAMBDA);

}
//static myENC_RESULT res;
/*****************************************/
ffmpegEncoder::ffmpegEncoder (uint32_t width, uint32_t height, FF_CODEC_ID id,PixelFormat format):encoder (width,
	 height)
{
  printf ("[LAVCODEC]Build: %d, colorspace :%x\n", LIBAVCODEC_BUILD,format);
  _targetColorSpace=format;//PIX_FMT_YUV420P
  _id = id;
  _swap = 0;
  _context = NULL;
  _settingsPresence = 0;

  _context = avcodec_alloc_context ();

  ADM_assert (_context);
  memset (&_frame, 0, sizeof (_frame));
  _frame.pts = AV_NOPTS_VALUE;
  _context->width = _w;
  _context->height = _h;

  _isMT = 0;
  encodePreamble(NULL);

};
/*****************************************/
uint8_t   ffmpegEncoder::encode(ADMImage *in,ADMBitstream *out)
{
    int32_t sz = 0;
    ADM_assert(out->bufferSize);
    encodePreamble (in->data);
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
        return 0;
    postAmble(out,sz);
    return 1;
}
/*****************************************/
ffmpegEncoder::~ffmpegEncoder ()
{
  printf ("[lavc] encoder destroying..\n");
  if (_isMT && _context)
    {
      printf ("[lavc] killing threads\n");
      avcodec_thread_free (_context);
      _isMT = 0;
    }
  stopEncoder ();
}

/*
   	Initialize codec in Q mode

*/

uint8_t
ffmpegEncoder::encodePreamble (uint8_t * in)
{
  _frame.key_frame = 0;
  _frame.pict_type = 0;

  switch(_targetColorSpace)
  {
    case PIX_FMT_YUV420P:
        _frame.linesize[0] = _w;
        _frame.linesize[1] = _w >> 1;
        _frame.linesize[2] = _w >> 1;
        _frame.data[0] = in;
        _frame.data[2] = in + _w * _h;
        _frame.data[1] = in + _w * _h + ((_w * _h) >> 2);
        break;
 case PIX_FMT_YUV422P:
        _frame.linesize[0] = _w;
        _frame.linesize[1] = _w >> 1;
        _frame.linesize[2] = _w >> 1;
        _frame.data[0] = in;
        _frame.data[2] = in + _w * _h;
        _frame.data[1] = in + _w * _h + ((_w * _h) >> 1);

        break;
    default:
      ADM_assert(0);
  }
  return 1;
}

uint32_t
ffmpegEncoder::frameType (void)
{
  int k, t;

  k = _context->coded_frame->key_frame;
  t = _context->coded_frame->pict_type;

  //printf(" Kf: %d type :%d \n",k,t);
  if (k == 1)
    return AVI_KEY_FRAME;
  if (t == FF_B_TYPE)
    return AVI_B_FRAME;
  //if(t==FF_I_TYPE) return AVI_KEY_FRAME;
  return 0;

}

/*
	This is called for mpeg1/2 initContext
	Set some specific stuff.

*/
uint8_t
ffmpegEncoder::gopMpeg1 (void)
{
  // small gop, 2 b frames allowed
  // min bitrate 500 max bitrate 2200
  int rate;

  rate = _context->time_base.den;
  rate = rate * 1000;
  rate /= _context->time_base.num;

  _context->me_range = 255;	// Fix motion vector for picky players (pioneer)

  if (_id == FF_MPEG2)
    {
      if (FRAME_FILM == identMovieType (rate))
	{
	  printf ("[LAVCODEC]Pulldown activated...\n");
	  _context->flags2 |= CODEC_FLAG2_32_PULLDOWN;
	}
    }
#ifdef  TEST_NOB		// disable B frames
  _context->max_b_frames = 0;
#else
  _context->max_b_frames = 2;
#endif
  printf ("[LAVCODEC]Using 2 b frames\n");
  if (_id == FF_MPEG2)
    {
      _context->mpeg_quant = 1;	//1; // Should be mpeg quant
    }
  else
    {
      _context->mpeg_quant = 0;	//1; // Should be mpeg quant
    }
  if (_settingsPresence)
    {
      if (_settings.widescreen)
	{
          printf("[LAVCODEC]WideScreen\n");
	  _context->sample_aspect_ratio.num = 16;
	  _context->sample_aspect_ratio.den = 9;
	}
      else
	{
	  _context->sample_aspect_ratio.num = 4;
	  _context->sample_aspect_ratio.den = 3;
	}

      _context->rc_max_rate_header = _settings.maxBitrate * 1000;//1800*1000;// 2400 max, 700 min
      _context->rc_buffer_size_header = _settings.bufferSize * 8 * 1024;
      // If we don't have a maxrate, don't set buffer_size
      if (1 && !_settings.override_ratecontrol)	// FIXME

	{
	  _context->rc_buffer_size = _context->rc_buffer_size_header;
	  _context->rc_max_rate = _context->rc_max_rate_header;
	}
      else
	{
	  _context->rc_buffer_size = 0;	// for xvid, no VBV so no ratecontrol
	  _context->rc_max_rate = 0;
	}
      _context->gop_size = _settings.gop_size;

    }
  else
    {
      _context->rc_buffer_size = 200 * 8 * 1024;	// 40 for VCD  & 200 for SVCD
      _context->gop_size = _settings.gop_size;

    }
  _context->rc_buffer_aggressivity = 1.0;
  _context->rc_initial_cplx = 3;
  _context->qmin = 2;
  _context->qmax = 31;

  _context->scenechange_threshold = 0xfffffff;	// Don't insert I frame out of order

  _frame.interlaced_frame = _settings.interlaced;
  if (_settings.interlaced)
    _frame.top_field_first = !_settings.bff;

#if defined(CODEC_FLAG_INTERLACED_DCT)
  _context->flags |= _settings.interlaced ? CODEC_FLAG_INTERLACED_DCT : 0;
#endif
#if defined(CODEC_FLAG_INTERLACED_ME)
  _context->flags |= _settings.interlaced ? CODEC_FLAG_INTERLACED_ME : 0;
#endif

  //
  //_context->dsp_mask= FF_MM_FORCE;
  printf ("[LAVCODEC]Mpeg12 settings:\n____________\n");
  printf ("[LAVCODEC]FF Max rate   (header) : %"LU" kbps\n",
	  (_context->rc_max_rate_header) / 1000);
  printf ("[LAVCODEC]FF Buffer Size(header) : %"LU" bits / %"LU" kB\n",
	  (_context->rc_buffer_size_header),
	  _context->rc_buffer_size_header / (8 * 1024));
  printf ("[LAVCODEC]FF Max rate   (rc) : %"LU" kbps\n", (_context->rc_max_rate) / 1000);
  printf ("[LAVCODEC]FF Buffer Size(rc) : %"LU" bits / %"LU" kB\n",
	  (_context->rc_buffer_size), _context->rc_buffer_size / (8 * 1024));

  printf ("[LAVCODEC]FF GOP Size    : %"LU"\n", _context->gop_size);
  printf ("[LAVCODEC]FF Bitrate    : %"LU" (kb/s)\n", _context->bit_rate/1000);

  return 1;
}

uint8_t
ffmpegEncoder::initContext (void)
{
  int res = 0;

  // set a gop size close to what's requested for most
  // player compatiblity
  if (_id == FF_MPEG1 || _id == FF_MPEG2)
    gopMpeg1 ();
  // if (_id == FF_HUFF || _id == FF_FFV1)
  _context->strict_std_compliance = -1;
  if (_id == FF_HUFF || _id == FF_FFV1 ||_id == FF_FFHUFF )
    _context->strict_std_compliance = -2;

  switch (_id)
    {
    case FF_MPEG4:
      encoderMT ();
      WRAP_Open (CODEC_ID_MPEG4);
      break;
    case FF_MSMP4V3:
      WRAP_Open (CODEC_ID_MSMPEG4V3);
      break;
    case FF_MPEG1:
      encoderMT ();
      WRAP_Open (CODEC_ID_MPEG1VIDEO);
      break;
    case FF_MPEG2:
      encoderMT ();
      WRAP_Open (CODEC_ID_MPEG2VIDEO);
      break;
    case FF_H263:
      WRAP_Open (CODEC_ID_H263);
      break;
    case FF_H263P:
      WRAP_Open (CODEC_ID_H263P);
      break;
    case FF_FLV1:
      WRAP_Open (CODEC_ID_FLV1);
      break;
    case FF_HUFF:
      WRAP_Open (CODEC_ID_HUFFYUV);
      break;
    case FF_FFV1:
      WRAP_Open (CODEC_ID_FFV1);
      break;
    case FF_MJPEG:
      WRAP_Open (CODEC_ID_MJPEG);
      break;
    case FF_FFHUFF:
      WRAP_Open (CODEC_ID_FFVHUFF);
      break;
    case FF_SNOW:
      WRAP_Open (CODEC_ID_SNOW);
      break;

    case FF_DV:
      if(_context->width!=720 || _context->height!=576)
            return 0; // should be caught by upper layers before going here...
      WRAP_Open (CODEC_ID_DVVIDEO);
      break;

    default:
      ADM_assert (0);
    }

  if (res < 0)
    {
      printf ("[lavc] Problem opening codec\n");
      return 0;
    }
  return 1;

}

void ffmpegEncoder::encoderMT (void)
{
  uint32_t threads = 0;

  prefs->get(FEATURE_THREADING_LAVC, &threads);

  if (threads == 0)
	  threads = ADM_cpu_num_processors();

  if (threads == 1)
	  threads = 0;

  if (threads)
  {
      printf ("[lavc] Enabling MT encoder with %u threads\n", threads);

      if (avcodec_thread_init (_context, threads) == -1)
	      printf ("[lavc] Failed!!\n");
	  else
          _isMT = 1;
  }
}

/*
		Set user selected matrices.

*/
uint8_t
ffmpegEncoder::setCustomMatrices (uint16_t * intra, uint16_t * inter)
{
  _context->intra_matrix = intra;
  _context->inter_matrix = inter;
  return 1;
}

uint8_t
ffmpegEncoder::setGopSize (uint32_t size)
{

  _context->gop_size = size;
  printf ("[LAVCODEC]Gop size is now %d\n", _context->gop_size);
  return 1;

}

uint8_t
ffmpegEncoder::setLogFile (const char *name)
{
  strcpy (LogName, name);
  return 1;

}

uint8_t
ffmpegEncoder::setConfig (FFcodecSetting * set)
{
  memcpy (&_settings, set, sizeof (_settings));
  _settingsPresence = 1;
  return 1;

}

uint8_t
ffmpegEncoderCQ::init (uint32_t val, uint32_t fps1000, uint8_t vbr)
{
  printf ("[LAVCODEC] Using Q=%u\n",val);
  mplayer_init ();
  _qual = val;
  _vbr = vbr;
  _context->flags |= CODEC_FLAG_QSCALE;

  if (_vbr)
    {
      _context->flags |= CODEC_FLAG_PASS1;
      _statfile = NULL;

    }
  _context->time_base = (AVRational)  {  1000, fps1000};
/*
  _context->frame_rate_base = 1000;
  _context->frame_rate = fps1000;
*/
  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;

  return initContext ();
}

uint32_t ffmpegEncoder::getCodedFrame (void)
{
  return _last_coded_frame;
}

uint8_t ffmpegEncoderCQ::encode (ADMImage * in, ADMBitstream * out)
{
  int32_t    sz = 0;
  uint32_t    f;
  uint8_t    r=0;

  _frame.quality = (int) floor (FF_QP2LAMBDA * _qual + 0.5);
  r=ffmpegEncoder::encode(in,out);
  out->out_quantizer=_qual;
  if (_vbr)			// update for lavcodec internal
    {
      if (!_statfile)
	{
	  printf ("[lavc] using %s as log file\n", LogName);
	  _statfile = fopen (LogName, "wb");
	}

      if (!_statfile)
	{
	  printf
	    ("[lavc] cannot open log file for writing! (%s)\n",
	     LogName);
	  return 0;
	}
      if (_context->stats_out)
	fprintf (_statfile, "%s", _context->stats_out);
    }

  return 1;

}

/*
_____________________________________________
*/
/*
   	Initialize codec in CBR mode

*/

uint8_t ffmpegEncoderCBR::init (uint32_t val, uint32_t fps1000)
{
//       mpeg4_encoder
  //
//              now init our stuff
//
  _br = val;
  mplayer_init ();
/*  _context->frame_rate_base = 1000;
  _context->frame_rate = fps1000;*/
  _context->time_base = (AVRational)
  {
  1000, fps1000};

  if(_id==FF_MPEG2 || _id==FF_MPEG1)
	  _context->bit_rate = _br;
  else
	  _context->bit_rate = _br*1000;

  printf ("[LAVCODEC] Using  bitrate in context :%"LU" kbps",_context->bit_rate/1000);

  return initContext ();

}


//------------------------------------------------------------------------------
//              VBR
//------------------------------------------------------------------------------
uint8_t ffmpegEncoderVBR::encode (ADMImage * in, ADMBitstream * out)
{
    uint16_t q;
    uint8_t kf;

    q=out->in_quantizer;
    _frame.quality = (int) floor (FF_QP2LAMBDA * q + 0.5);
    return ffmpegEncoder::encode(in,out);

}
/*
   			val is the average bitrate wanted, else it is useless
*/
uint8_t
ffmpegEncoderVBR::init (uint32_t val, uint32_t fps1000)
{
  uint32_t statSize;
  FILE *_statfile;

  printf ("[lavc] initializing in VBR mode\n");
  _qual = val;
  mplayer_init ();

//   _context->frame_rate_base = 1000;
//   _context->frame_rate = fps1000;

  _context->time_base = (AVRational)  {  1000, fps1000};

  /* If internal 2 passes mode is selected ... */
  _context->flags |= CODEC_FLAG_PASS2;

  _statfile = fopen (LogName, "rb");
  if (!_statfile)
    {
      printf ("internal file does not exists ?\n");
      return 0;
    }

  fseek (_statfile, 0, SEEK_END);
  statSize = ftello (_statfile);
  fseek (_statfile, 0, SEEK_SET);
  _context->stats_in = (char *) ADM_alloc (statSize + 1);
  _context->stats_in[statSize] = 0;
  fread (_context->stats_in, statSize, 1, _statfile);
  fclose(_statfile);

  _context->bit_rate = val;	// bitrate

  return initContext ();

}

ffmpegEncoderVBR::~ffmpegEncoderVBR ()
{


  ADM_dealloc (_context->stats_in);

}

//--------------------- FFmpeg VBR, external Qzation engine ------------------
uint8_t
ffmpegEncoderVBRExternal::encode (ADMImage * in, ADMBitstream * out)
{
    uint8_t r;
    uint32_t q;
  q=out->in_quantizer;
  _frame.quality = (int) floor (FF_QP2LAMBDA * q + 0.5);
  r= ffmpegEncoder::encode(in,out);
  out->out_quantizer=q;
  return r;
}


/**
	This is used only for Mpeg1, so it is a bit tuned for it
*/
uint8_t
ffmpegEncoderVBRExternal::init (uint32_t val, uint32_t fps1000)
{

  printf ("[lavc] initializing in VBRExternal mode\n");
  _qual = val;
  mplayer_init ();

/*  _context->frame_rate_base = 1000;
  _context->frame_rate = fps1000;*/
  _context->time_base = (AVRational) {  1000, fps1000};
  _context->flags |= CODEC_FLAG_QSCALE;;
  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;
  _context->max_qdiff = 10;

  // since this is used only for mpeg1 ...
  // PAL ?
  _context->bit_rate = 2500 * 1000 * 8;
  _context->sample_aspect_ratio.num = 4;
  _context->sample_aspect_ratio.den = 3;

  return initContext ();;
}

ffmpegEncoderVBRExternal::~ffmpegEncoderVBRExternal ()
{



}

//--------------------- FFmpeg VBR, external Qzation engine ------------------

void
ffmpegEncoder::mplayer_init (void)
{
  /*
     default values : Copy/past from mplayer
   */
  _context->pix_fmt = _targetColorSpace;

  if (!_settingsPresence)
    {
      printf ("[lavc] using default encode settings \n");
      _context->qmin = 2;
      _context->qmax = 31;
      _context->max_b_frames = 0;
      _context->mpeg_quant = 0;
      _context->me_method = ME_EPZS;
      _context->flags = 0;
      _context->max_qdiff = 3;
      _context->luma_elim_threshold = 0;
      _context->chroma_elim_threshold = 0;
      _context->lumi_masking = 0.0;;
      _context->dark_masking = 0.0;
      _context->qcompress = 0.5;
      _context->qblur = 0.5;
      _context->gop_size = 250;
    }
  else
    {
      if (_id == FF_MPEG1 || _id == FF_MPEG2 || _id == FF_FLV1)
	{
	  _context->gop_size = _settings.gop_size;
	}
      else
	{
	  _context->gop_size = 250;
	}
#define SETX(x) _context->x=_settings.x; printf("[LAVCODEC]"#x" : %d\n",_settings.x);
#define SETX_COND(x) if(_settings.is_##x) {_context->x=_settings.x; printf("[LAVCODEC]"#x" : %d\n",_settings.x);} else\
		{ printf(#x" is not activated\n");}
      SETX (me_method);
      SETX (qmin);
      SETX (qmax);
      SETX (max_b_frames);
      SETX (mpeg_quant);
      SETX (max_qdiff);
      SETX_COND (luma_elim_threshold);
      SETX_COND (chroma_elim_threshold);

#undef SETX
#undef SETX_COND

#define SETX(x)  _context->x=_settings.x; printf("[LAVCODEC]"#x" : %f\n",_settings.x);
#define SETX_COND(x)  if(_settings.is_##x) {_context->x=_settings.x; printf("[LAVCODEC]"#x" : %f\n",_settings.x);} else  \
									{printf("[LAVCODEC]"#x" No activated\n");}
      SETX_COND (lumi_masking);
      SETX_COND (dark_masking);
      SETX (qcompress);
      SETX (qblur);
      SETX_COND (temporal_cplx_masking);
      SETX_COND (spatial_cplx_masking);

#undef SETX
#undef SETX_COND

#define SETX(x) if(_settings.x){ _context->flags|=CODEC_FLAG##x;printf("[LAVCODEC]"#x" is set\n");}
      SETX (_GMC);


      switch (_settings.mb_eval)
	{
	case 0:
	  _context->mb_decision = FF_MB_DECISION_SIMPLE;
	  break;
	case 1:
	  _context->mb_decision = FF_MB_DECISION_BITS;
	  break;
	case 2:
	  _context->mb_decision = FF_MB_DECISION_RD;
	  break;
	default:
	  ADM_assert (0);


	}
      //SETX(_HQ);
      SETX (_4MV);
      SETX (_QPEL);
      //SETX (_TRELLIS_QUANT);
#warning FIXME TRELLIS
      if(_settings._TRELLIS_QUANT)
        { 
            _context->trellis=1;printf("[LAVCODEC] trellisis set\n");
        }
      SETX (_NORMALIZE_AQP);

      if (_settings.widescreen)
	{
	  _context->sample_aspect_ratio.num = 16;
	  _context->sample_aspect_ratio.den = 9;
	  printf ("[LAVCODEC]16/9 aspect ratio is set.\n");

	}
#undef SETX
    }
  if ((_id == FF_H263) || (_id == FF_H263P))
    _context->bit_rate_tolerance = 4000;
  else
    _context->bit_rate_tolerance = 8000000;


  _context->b_quant_factor = 1.25;
  _context->rc_strategy = 2;
  _context->b_frame_strategy = 0;
  _context->b_quant_offset = 1.25;
  _context->rtp_payload_size = 0;
  _context->strict_std_compliance = 0;
  _context->i_quant_factor = 0.8;
  _context->i_quant_offset = 0.0;
  _context->rc_qsquish = 1.0;
  _context->rc_qmod_amp = 0;
  _context->rc_qmod_freq = 0;
  _context->rc_eq = const_cast < char *>("tex^qComp");
  _context->rc_max_rate = 000;
  _context->rc_min_rate = 000;
  _context->rc_buffer_size = 000;
  _context->rc_buffer_aggressivity = 1.0;
  _context->rc_initial_cplx = 0;
  _context->dct_algo = 0;
  _context->idct_algo = 0;
  _context->p_masking = 0.0;

  _context->bit_rate = 0;

}

uint8_t
ffmpegEncoder::init (uint32_t val, uint32_t fps1000)
{
  UNUSED_ARG (val);
  UNUSED_ARG (fps1000);
  return 0;
}

uint8_t
ffmpegEncoder::init (uint32_t val, uint32_t fps1000, uint8_t sw)
{
  UNUSED_ARG (sw);
  return init (val, fps1000);
}

//------------------------------
uint8_t
ffmpegEncoderHuff::init (uint32_t val, uint32_t fps1000, uint8_t vbr)
{
  UNUSED_ARG (val);
  UNUSED_ARG (vbr);
  mplayer_init ();

  _context->time_base = (AVRational)
  {
  1000, fps1000};
//   _context->frame_rate_base = 1000;
//   _context->frame_rate = fps1000;


  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;
  _context->gop_size = 250;

  return initContext ();
}
//__________________________________
/**

*/

ffmpegEncoderHuff::ffmpegEncoderHuff (uint32_t width, uint32_t height,FF_CODEC_ID id)
  :   ffmpegEncoder (width,height, id,PIX_FMT_YUV422P)
{
  // Allocate our resampler & intermediate
  yuy2=new uint8_t[width*height*2];
  // And Color converted
    convert=new ADMColorspace(width,height, ADM_COLOR_YV12,ADM_COLOR_YUV422P);
}
/**

*/

ffmpegEncoderHuff::~ffmpegEncoderHuff()
{
  if(yuy2)
  {
    delete [] yuy2;
    yuy2=NULL;
  }
  if(convert)
  {
   delete convert;
    convert=NULL;
  }
  stopEncoder ();
}
/**

*/
  uint8_t   ffmpegEncoderHuff::encode(ADMImage *in,ADMBitstream *out)
{
    int32_t sz = 0;
    ADM_assert(out->bufferSize);
    encodePreamble(yuy2);
    /* Convert */
   convert->convert(in->data,yuy2);

    /***/
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
        return 0;
    postAmble(out,sz);
    return 1;
}
//------------------------------
uint8_t
  ffmpegEncoderFFHuff::init (uint32_t val, uint32_t fps1000, uint8_t vbr)
{
  UNUSED_ARG (val);
  UNUSED_ARG (vbr);
  mplayer_init ();

/*  _context->frame_rate_base = 1000;
  _context->frame_rate = fps1000;*/
  _context->time_base = (AVRational)
  {
  1000, fps1000};

  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;
  _context->gop_size = 250;

  return initContext ();
}

//_______________________________________
uint8_t
ffmpegEncoder::getExtraData (uint32_t * l, uint8_t ** d)
{
  *d = (uint8_t *) _context->extradata;
  *l = _context->extradata_size;
  printf ("[LAVCODEC]We got some extra data: %"LU"\n", *l);
  if (*l)
    return 1;

  else
    return 0;
}

//----
//------------------------------
uint8_t
ffmpegEncoderFFV1::init (uint32_t val, uint32_t fps1000, uint8_t vbr)
{
  UNUSED_ARG (val);
  UNUSED_ARG (vbr);
  mplayer_init ();
  _context->time_base = (AVRational)
  {
  1000, fps1000};
//   _context->frame_rate_base = 1000;
//   _context->frame_rate = fps1000;


  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;
  _context->gop_size = 250;
  printf ("[LAVCODEC]FFV1 codec initializing...\n");
  return initContext ();
}

/*---
*/
uint8_t
  ffmpegEncoderFFMjpeg::init (uint32_t val, uint32_t fps1000, uint8_t vbr)
{
  UNUSED_ARG (val);
  UNUSED_ARG (vbr);
  mplayer_init ();

  float f;

  f = val;
  f = 31. - (29. * f / 100.);

  _qual = (uint32_t) floor (f);

//   _context->frame_rate_base = 1000;
//   _context->frame_rate = fps1000;
  _context->time_base = (AVRational)
  {
  1000, fps1000};
  _context->flags = CODEC_FLAG_QSCALE;
  _context->bit_rate = 0;
  _context->bit_rate_tolerance = 1024 * 8 * 1000;
  _context->gop_size = 250;
  printf ("[LAVCODEC]FF Mjpeg codec initializing %d %% -> q =%d...\n", val, _qual);
  return initContext ();
}



uint8_t
ffmpegEncoderFFMjpeg::encode (ADMImage * in, ADMBitstream * out)
{
  _frame.quality = (int) floor (FF_QP2LAMBDA * _qual + 0.5);
  return ffmpegEncoder::encode(in,out);
}


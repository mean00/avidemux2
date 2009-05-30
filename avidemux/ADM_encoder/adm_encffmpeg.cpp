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


#include "ADM_lavcodec.h"


#include "fourcc.h"
#include "avi_vars.h"
#include "DIA_coreToolkit.h"
#include "ADM_default.h"
#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_videoFilter.h"
#include "ADM_codecs/ADM_ffmpeg.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encffmpeg.h"

uint32_t ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);

/*_________________________________________________*/
EncoderFFMPEG::EncoderFFMPEG (FF_CODEC_ID id, COMPRES_PARAMS * config)
{
  _codec = NULL;
  fd = NULL;
  strcpy (_logname, "");
  _frametogo = 0;
  memcpy (&_param, config, sizeof (_param));
  ADM_assert (sizeof (_settings) == config->extraSettingsLen);
  memcpy (&_settings, config->extraSettings, sizeof (_settings));
  _id = id;
};

//---------------huff----------
EncoderFFMPEGHuff::EncoderFFMPEGHuff (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_HUFF, config)
{
  _id = FF_HUFF;
  _frametogo = 0;
  _state=enc_CQ;

}
uint8_t
EncoderFFMPEGHuff::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  uint8_t r = 0;
  r = _codec->getExtraData (l, data);
  printf ("Huff has %d extra bytes\n", *l);
  return r;

}
uint8_t
EncoderFFMPEGHuff::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;



  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;

  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  _codec = new ffmpegEncoderHuff(_w, _h, _id);
  _codec->init (_param.qz, _fps, 0);
  return 1;



}

//_________________ffhuff_______________________________
EncoderFFMPEGFFHuff::EncoderFFMPEGFFHuff (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_FFHUFF, config)
{
  _id = FF_FFHUFF;
  _frametogo = 0;
}
uint8_t
EncoderFFMPEGFFHuff::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  uint8_t r = 0;
  r = _codec->getExtraData (l, data);
  printf ("Huff has %d extra bytes\n", *l);
  return r;

}
uint8_t
EncoderFFMPEGFFHuff::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;



  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  _codec = new ffmpegEncoderCQ (_w, _h, _id);
  _codec->init (_param.qz, _fps, 0);
  _state=enc_CQ;
  return 1;



}


//-------------------ffv1------------------
EncoderFFMPEGFFV1::EncoderFFMPEGFFV1 (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_HUFF, config)
{
  _id = FF_FFV1;
  _frametogo = 0;


}
uint8_t
EncoderFFMPEGFFV1::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  *l = 0;
  *data = NULL;
  return 0;

}
uint8_t
EncoderFFMPEGFFV1::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;
  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;
//  _vbuffer = new uint8_t[_w * _h * 3];
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  _codec = new ffmpegEncoderCQ (_w, _h, _id);
  _codec->init (_param.qz, _fps, 0);
  _state=enc_CQ;
  return 1;
}


//--------------end------------
//************************* SNOW **************************
EncodeFFMPEGSNow::EncodeFFMPEGSNow (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_HUFF, config)
{
  _id = FF_SNOW;
  _frametogo = 0;


}
uint8_t
EncodeFFMPEGSNow::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  *l = 0;
  *data = NULL;
  return 0;

}
uint8_t
EncodeFFMPEGSNow::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;



  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;
  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  _codec = new ffmpegEncoderCQ (_w, _h, _id);
  _codec->init (_param.qz, _fps, 0);
  _state=enc_CQ;
  return 1;



}

//*********************DV **********************************************
EncoderFFMPEGDV::EncoderFFMPEGDV (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_DV, config)
{
  _id = FF_DV;
  _frametogo = 0;


}
uint8_t
EncoderFFMPEGDV::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  *l = 0;
  *data = NULL;
  return 0;

}
uint8_t
EncoderFFMPEGDV::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;



  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;

  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  if(_w!=720 || _h!=576 || _fps!=25000)
  {
    GUI_Error_HIG(QT_TR_NOOP("Incompatible settings"),QT_TR_NOOP("At the moment, the DV codec only accepts 720*576@25"));
    return 0;
  }
  _codec = new ffmpegEncoderCQ (_w, _h, _id);
  _codec->init (_param.qz, _fps, 0);
  _state=enc_CQ;
  return 1;



}
//********************* FLV1 **********************************************
EncoderFFMPEGFLV1::EncoderFFMPEGFLV1 (COMPRES_PARAMS * config):
EncoderFFMPEG (FF_FLV1, config)
{
  _id = FF_FLV1;
  _frametogo = 0;


}
uint8_t
EncoderFFMPEGFLV1::hasExtraHeaderData (uint32_t * l, uint8_t ** data)
{
  *l = 0;
  *data = NULL;
  return 0;

}
uint8_t
EncoderFFMPEGFLV1::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{
  ADM_assert (instream);
  ADV_Info *info;



  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;

  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;


  _codec = new ffmpegEncoderCBR (_w, _h, _id);
  _codec->setConfig (&_settings);
  _codec->init (_param.bitrate, _fps, 0);
  _state=enc_CQ;
  return 1;



}

//************************* SNOW **************************
// return codec name as seen in avi header
//
const char *
EncoderFFMPEG::getCodecName (void)
{
  switch (_id)
    {
    case FF_DV:
        return "dvsd";
        break;
    case FF_FFHUFF:
      return "FFVH";
      break;
    case FF_HUFF:
      return "HFYU";
      break;
    case FF_MPEG4:
      return "DX50";
      break;
    case FF_H263P:
    case FF_H263:
      return "H263";
      break;
    case FF_FFV1:
      return "FFV1";
      break;
    case FF_SNOW:
      return "SNOW";
      break;
    case FF_FLV1:
      return "FLV1";
      break;

    default:
      ADM_assert (0);

    }
  return NULL;
}
//--------------------------------
uint8_t EncoderFFMPEG::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{

  ADM_assert (instream);
  ADV_Info *
    info;

  uint32_t
    flag1,
    flag2,
    flag3;
  flag1 = flag2 = flag3 = 0;

  info = instream->getInfo ();
  _fps = info->fps1000;
  _w = info->width;
  _h = info->height;

  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;


  switch (_param.mode)
    {
    case COMPRESS_SAME:
      printf ("FFmpeg in follow quant mode\n");
      _state = enc_Same;
      _codec = new ffmpegEncoderVBRExternal (_w, _h, _id);
      _codec->setConfig (&_settings);
      _codec->init (2, _fps, 0);
      break;

    case COMPRESS_CQ:
      printf ("ffmpeg cq mode: %"LU"\n", _param.qz);
      _state = enc_CQ;
      _codec = new ffmpegEncoderCQ (_w, _h, _id);
      _codec->setConfig (&_settings);
      _codec->init (_param.qz, _fps, 0);
      break;
    case COMPRESS_CBR:
      printf ("ffmpeg cbr mode: %"LU"\n", _param.bitrate);
      _state = enc_CBR;
      _codec = new ffmpegEncoderCBR (_w, _h, _id);
      _codec->setConfig (&_settings);
      _codec->init (_param.bitrate, _fps, flag1);
      break;
    case COMPRESS_2PASS:
    case COMPRESS_2PASS_BITRATE:
      ffmpegEncoderCQ * cdec;
      if(_param.mode==COMPRESS_2PASS)
        printf ("\n ffmpeg dual size: %"LU, _param.finalsize);
      else
        printf ("\n ffmpeg dual bitrate: %"LU, _param.avg_bitrate);
      _state = enc_Pass1;
      cdec = new ffmpegEncoderCQ (_w, _h, _id);	// Pass1
      cdec->setConfig (&_settings);
      // 1+  VBR stats required
      // no stats
      // force internal
      cdec->setLogFile (_logname);
      cdec->init (2, _fps, 1);

      _codec = cdec;
      if (flag1)
	_internal = 0;
      else
	_internal = 1;
      break;

      break;
    default:
      ADM_assert (0);

    }
  _in = instream;
  printf ("\n ffmpeg Encoder , w: %"LU" h:%"LU" mode:%d", _w, _h, _state);
  return 1;

}



uint8_t EncoderFFMPEG::startPass1 (void)
{
  ADM_assert (_state == enc_Pass1);
  _frametogo = 0;
  printf ("\n Starting pass 1\n");
  printf (" Creating logfile :<%s>\n", _logname);
  return 1;
}


uint8_t EncoderFFMPEG::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
    {
      return 1;
    }
  return 0;
}

uint8_t EncoderFFMPEG::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  printf("EncoderFF : using <%s> as logfile, for %u frames\n",lofile,nbframe);
  return 1;

}

//______________________________
uint8_t EncoderFFMPEG::encode (uint32_t frame, ADMBitstream *out)
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
      return _codec->encode (_vbuffer, out );
      break;
    case enc_Same:
      {
	uint32_t inq;
        if (_vbuffer->flags & AVI_KEY_FRAME)
            out->flags = AVI_KEY_FRAME;
	else
	   out->flags = 0;
	inq = _vbuffer->_Qp;
        if(inq>31) inq=31;
        if(inq<2) inq=2;
        out->in_quantizer =inq;

	if (!_codec->encode (_vbuffer, out ))
	  {
	    printf ("\n codec error on 1st pass !");
	    return 0;
	  }
	//printf("Inq:%lu outq:%lu\n",inq,enc.out_quantizer);
	_frametogo++;
	return 1;
	break;
      }
    case enc_Pass1:

      //              ADM_assert(fd);
      if (!_codec->encode (_vbuffer, out ))
	{
	  printf ("\n codec error on 1st pass !");
	  return 0;
	}
      _frametogo++;
      return 1;
      break;
    case enc_Pass2:
      // Encode it !
      if (!_codec->encode (_vbuffer, out ))
            return 0;
      return 1;
      break;

    default:
      ADM_assert (0);
    }
  return 0;
}

//_______________________________
uint8_t EncoderFFMPEG::stop (void)
{
  if (_codec)
    delete
      _codec;
  _codec = NULL;
  return 1;

}

uint8_t EncoderFFMPEG::startPass2 (void)
{

  ADM_assert (_state = enc_Pass1);
  printf ("\n Starting pass 2\n");


  printf ("\n VBR paramaters computed\n");
  _state = enc_Pass2;
  // Delete codec and start new one
  if (_codec)
    {
      delete _codec;
      _codec = NULL;
    }
  _codec = new ffmpegEncoderVBR (_w, _h, _internal, _id);	//0 -> external 1 -> internal
  _codec->setConfig (&_settings);
  printf ("\n ready to encode in 2pass : %s\n", _logname);
  uint32_t    vbr = 0;
  switch(_param.mode)
  {
    case COMPRESS_2PASS:
          // compute average bitrate
            vbr= ADM_computeBitrate(_fps, _frametogo,_param.finalsize);
            break;
    case COMPRESS_2PASS_BITRATE:
            vbr=_param.avg_bitrate*1000; // in b/s
            break;
    default:
          ADM_assert(0);
  }

  _codec->setLogFile (_logname);
  if(!_codec->init (vbr, _fps))
  {
    printf("2 pass start failed!\n");
    return 0;
  }

  printf ("\n ** Total size     : %"LU" MBytes \n", _param.finalsize);
//  printf(" ** Total frame    : %"LU"  \n",_totalframe);
  printf (" (using avg bitrate of %"LU" kbps", vbr / 1000);
  return 1;
}
//-----------------------ffmpegEncoderHuff



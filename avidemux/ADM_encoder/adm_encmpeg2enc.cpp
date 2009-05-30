/***************************************************************************

    copyright            : (C) 2006 by mean
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

#include "ADM_default.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "fourcc.h"
#include "ADM_quota.h"
#include "avi_vars.h"

#include "ADM_assert.h"


#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_videoFilter.h"
#include "ADM_encoder/adm_encoder.h"
#include "ADM_encoder/adm_encConfig.h"

#include "audioprocess.hxx"

#include "ADM_libraries/ADM_libmpeg2enc/ADM_mpeg2enc.h"
#include "audioeng_buildfilters.h"
#include "prefs.h"
#include "adm_encmpeg2enc.h"
#include "ADM_libraries/ADM_xvidratectl/ADM_ratecontrol.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_ENCODER
#include "ADM_osSupport/ADM_debug.h"


#warning FIXME: Duplicate define with mpeg2enc -> bad
#define MPEG_PREFILL 5

static ADM_newXvidRcVBV *_xrc = NULL;
extern uint32_t ADM_computeBitrate(uint32_t fps1000, uint32_t nbFrame, uint32_t sizeInMB);

/**
      \fn verifyLog
      \brief Ask the RC if the 2pass log file is correct
      @param file filename
      @param nb nbFrames expected to be present
      @return 1 if file is ok, 0 is corrupted

*/
uint8_t EncoderMpeg2enc::verifyLog(const char *file,uint32_t nbFrame)
{
      return ADM_newXvidRcVBV::verifyLog(file,nbFrame+2);
}
/*_________________________________________________*/
EncoderMpeg2enc::EncoderMpeg2enc (MPEG2ENC_ID id, COMPRES_PARAMS * config)
{
  _frametogo = 0;
  _pass1Done = 0;
  _lastQz = 0;
  _lastBitrate = 0;
  _totalframe = 0;
  _id=id;
  memcpy (&_param, config, sizeof (_param));
  ADM_assert (config->extraSettingsLen == sizeof (_settings)); //Mpeg2encParam
  memcpy (&_settings, config->extraSettings, sizeof (_settings));
  _delayed=0;
  _availableFrames=0;

};
#define MPEG1_MIN_Q 2
#define MPEG1_MAX_Q 28

uint8_t     EncoderMpeg2enc::encode (uint32_t frame, ADMBitstream *out)
{
  uint32_t l,flags;
  ADM_assert (_codec);
  ADM_assert (_in);
  ADM_rframe rf;
  uint32_t asked;
  uint32_t nq;
  uint32_t nf;
  ADM_rframe f;

_retry:
  asked=frame+_delayed;
  if(asked>=_availableFrames) asked=_availableFrames-1;
  if (!_in->getFrameNumberNoAlloc (asked, &l, _vbuffer, &flags))
  {
    printf ("\n Error : Cannot read incoming frame !");
    return 0;
  }

  if(_state==enc_Pass2)
  {
      _xrc->getQz (&nq, &f);
      switch (f)
      {
        case RF_I:
          nf = AVI_KEY_FRAME;
          break;
        default:
          nf = 0;
      }

      if (nq < MPEG1_MIN_Q)
        nq = MPEG1_MIN_Q;
      if (nq > MPEG1_MAX_Q)
        nq = MPEG1_MAX_Q;

        //printf("asked :%d ",nq);
        out->in_quantizer=nq;
        out->flags=nf;
   }else
  {
      out->in_quantizer=0;
  }
   if(!_codec->encode (_vbuffer, out ))
   {
     printf("[mpeg2enc]Codec error frame %u delay %u\n", frame,_delayed);
     return 0;
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

   switch(_state)
   {
     case enc_Pass1:
        {
          if (!out->len)
          {
            printf ("Skipping delay\n");
            goto _retry;
          }

          _xrc->logPass1 (out->out_quantizer, rf, out->len);
          _frametogo++;
        }
        break;
     case enc_Pass2:
     {
       if (!out->len)
       {
         printf ("Skipping delay\n");
         goto _retry;
       }
       {
         _xrc->logPass2 (out->out_quantizer, rf, out->len);
       }
     }
     break;
     default:
       break;
   }
   return 1;
}
EncoderMpeg2enc::~EncoderMpeg2enc ()
{
  stop ();
};



//--------------------------------
uint8_t     EncoderMpeg2enc::configure (AVDMGenericVideoStream * instream, int useExistingLogFile)
{

  ADM_assert (instream);
  ADV_Info *info;
  fd = NULL;

  uint32_t flag1, flag2, flag3,qz,br;
  flag1 = flag2 = flag3 = 0;

  info = instream->getInfo ();
  _fps1000 = info->fps1000;
  _w = info->width;
  _h = info->height;

  _vbuffer = new ADMImage (_w, _h);
  ADM_assert (_vbuffer);
  _in = instream;

  _availableFrames=instream->getInfo()->nb_frames;

  uint32_t interlaced=_settings.interlaced;
  uint32_t bff=_settings.bff;
  uint32_t widescreen=_settings.widescreen;

  _codec=NULL;
  switch (_param.mode)
  {
    case COMPRESS_CQ:
          qz=_param.qz;
          br=_settings.maxBitrate*1000;
          _state = enc_CQ;
      break;
    case COMPRESS_CBR:
          qz=0;
          br=_param.bitrate*1000;
          _state = enc_CBR;
          break;
    case COMPRESS_2PASS:
    case COMPRESS_2PASS_BITRATE:
          qz=2;
          br=0;
          _state = enc_Pass1;
          _xrc = new ADM_newXvidRcVBV (_fps1000, _logname);
          break;
  }
  switch(_id)
  {
    case MPEG2ENC_VCD:
      {
        Mpeg2encVCD *dec;
        dec = new Mpeg2encVCD (_w, _h);
        dec->init (1, 0, _fps1000, interlaced, bff, widescreen, 0);
        _codec = dec;
      }
      break;
    case MPEG2ENC_SVCD:
    {
        Mpeg2encSVCD * dec;
        dec = new Mpeg2encSVCD (_w, _h);
        dec->setMatrix (_settings.user_matrix);
        dec->init (qz, br, _fps1000, interlaced, bff, widescreen, 0);
        _codec = dec;
    }
        break;
    case MPEG2ENC_DVD:
      {
        Mpeg2encDVD *dec;
        dec = new Mpeg2encDVD (_w, _h);
        dec->setMatrix (_settings.user_matrix);
        dec->init (qz, br, _fps1000, interlaced, bff, widescreen, 0);
        _codec = dec;
      }
      break;
      default:
        ADM_assert (0);
    }

  ADM_assert(_codec);

  _in = instream;
  printf ("\n Mpeg2enc Encoder , w: %"LU" h:%"LU" mode:%d", _w, _h, _state);
  return 1;
}




uint8_t    EncoderMpeg2enc::startPass1 (void)
{
  ADM_assert (_state == enc_Pass1);
  _frametogo = 0;
  printf ("\n Starting pass 1\n");
  printf (" Creating logfile :%s\n", _logname);
  _pass1Done = 1;
  {
    printf ("Using Xvid 2 pass rate control (%s)\n", _logname);
    _xrc->startPass1 ();
  }
  return 1;
}


uint8_t   EncoderMpeg2enc::isDualPass (void)
{
  if ((_state == enc_Pass1) || (_state == enc_Pass2))
  {
    return 1;
  }
  return 0;
}

uint8_t    EncoderMpeg2enc::setLogFile (const char *lofile, uint32_t nbframe)
{
  strcpy (_logname, lofile);
  _frametogo = nbframe;
  _totalframe = nbframe;
  return 1;
}

//_______________________________
uint8_t     EncoderMpeg2enc::stop (void)
{
  printf ("[Mpeg2enc]Stopping encoder\n");
  if (_codec)     delete _codec;
  _codec = NULL;
    if (_state == enc_Pass1 || _state == enc_Pass2)
    {
      if (_xrc)
      {
        delete _xrc;
        _xrc = NULL;
      }

    }
  return 1;
}
//_______________________________

uint8_t    EncoderMpeg2enc::startPass2 (void)
{
  uint32_t br,avg_bitrate,size;

  ADM_assert (_state == enc_Pass1);
  printf ("[Mpeg2enc]-------* Starting pass 2*-------------\n");

  if(_param.mode==COMPRESS_2PASS)
  {
    br=ADM_computeBitrate( _fps1000,_totalframe,_param.finalsize);
    size=_param.finalsize;
    printf("[Mpeg2enc] Final Size: %u MB, avg bitrate %u kb/s \n",size,br/1000);
  }else if(_param.mode==COMPRESS_2PASS_BITRATE)
  {
    double d;
    br=_param.avg_bitrate;
    d=_totalframe;
    d/=_fps1000;
    d*=_param.avg_bitrate*1000*1000;
    d/=(1024*1024);
    size=(uint32_t )d;

    printf("[Mpeg2enc]  Final Size: %u MB 2pass avg bitrate %u kb/s\n",size,br);

  }else ADM_assert(0);

  uint32_t maxbr;

  maxbr=_settings.maxBitrate*1000; // B/s

  printf("[Mpeg2enc] Max bitrate %u kbps\n",maxbr/1000);
  /* Check Max bitrate */
  if(br>maxbr)
  {
     br=maxbr;
     printf("[Mpeg2enc] Max bitrate exceeeded, Clipping..\n");
  }


  printf ("[Mpeg2enc] ** Total size     : %"LU" MBytes \n", _param.finalsize);
  printf ("[Mpeg2enc] ** Total frame    : %"LU"  \n", _totalframe);

  printf ("[Mpeg2enc] VBR parameters computed\n");
  _state = enc_Pass2;
  // Delete codec and start new one
  if (_codec)
  {
    delete _codec;
    _codec = NULL;
  }

  /*********/
  uint32_t interlaced=_settings.interlaced;
  uint32_t bff=_settings.bff;
  uint32_t widescreen=_settings.widescreen;
  uint32_t vbv;

  switch(_id)
  {
    case MPEG2ENC_SVCD:

      Mpeg2encSVCD * dec;
      dec = new Mpeg2encSVCD (_w, _h);
      dec->setMatrix (_settings.user_matrix);
      dec->init (2, br, _fps1000, interlaced, bff, widescreen, 0);	// WLA
      _codec = dec;
      vbv=122*1024;
      break;
    case MPEG2ENC_DVD:
    {

      Mpeg2encDVD *dec;
      dec = new Mpeg2encDVD (_w, _h);
      dec->setMatrix (_settings.user_matrix);
      dec->init (2, br, _fps1000, interlaced, bff, widescreen, 0);	// WLA
      _codec = dec;
      vbv=224*1024;
    }
    break;
    default:
      ADM_assert (0);
      break;
  }
  _xrc->setVBVInfo (_settings.maxBitrate, 0,  vbv);
  ADM_assert (_xrc->startPass2 (size, _totalframe));

  printf ("\n XV:ready to encode in 2pass(%s)\n", _logname);
  _frametogo = 0;
  return 1;


}

////***********************************
////***********************************
////***********************************
////***********************************
////***********************************

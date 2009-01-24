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

#include <math.h>

#include "ADM_default.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"

#include "audioencoder.h"
#include "audioencoderInternal.h"
#include "audioencoder_twolame.h"
#include "audioencoder_twolame_param.h"

extern "C"
{
#include "twolame.h"
}

#define OPTIONS (twolame_options_struct *)_twolameOptions

static TWOLAME_encoderParam twolameParam=
{
    128,
    ADM_STEREO
};
static uint8_t configure (void);
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Twolame);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "TwoLame",
  "MP2 (Twolame)",
  "TwoLame MP2 encoder plugin Mean 2008",
  2,                    // Max channels
  1,0,0,                // Version
  WAV_MP2,
  200,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(twolameParam);

/******************* / Declare plugin*******************************************************/

AUDMEncoder_Twolame::AUDMEncoder_Twolame(AUDMAudioFilter * instream)  :AUDMEncoder    (instream)
{
  printf("[TwoLame] Creating Twolame\n");
  _twolameOptions=NULL;
  _wavheader->encoding=WAV_MP2;
};


AUDMEncoder_Twolame::~AUDMEncoder_Twolame()
{
  printf("[TwoLame] Deleting TwoLame\n");
  if(_twolameOptions)
  {
    twolame_close((twolame_options_struct **)&_twolameOptions);
  }
  _twolameOptions=NULL;
  cleanup();
};
/**
    \fn initialize

*/
uint8_t AUDMEncoder_Twolame::initialize(void)
{
  int ret;
  TWOLAME_MPEG_mode mmode;
  uint32_t frequence;
  TWOLAME_encoderParam *lameConf=&twolameParam;


  _twolameOptions = twolame_init();
  if (_twolameOptions == NULL)
    return 0;

  if(_wavheader->channels>2)
  {
    printf("[TwoLame]Too many channels\n");
    return 0;
  }
  _wavheader->byterate=(lameConf->bitrate*1000)>>3;


  _chunk = 1152*_wavheader->channels;


  printf("[TwoLame]Incoming :fq : %"LU", channel : %"LU" bitrate: %"LU" \n",
        _wavheader->frequency,_wavheader->channels,lameConf->bitrate);


  twolame_set_in_samplerate(OPTIONS, _wavheader->frequency);
  twolame_set_out_samplerate (OPTIONS, _wavheader->frequency);
  twolame_set_num_channels(OPTIONS, _wavheader->channels);
  if(_wavheader->channels==1) mmode=TWOLAME_MONO;
  else
    switch (lameConf->mode)
  {
    case ADM_STEREO:
      mmode = TWOLAME_STEREO;
      break;
    case ADM_JSTEREO:
      mmode = TWOLAME_JOINT_STEREO;
      break;
    case ADM_MONO:
      mmode=TWOLAME_MONO;
      break;

    default:
      printf("\n **** unknown mode, going stereo ***\n");
      mmode = TWOLAME_STEREO;
      break;

  }
  twolame_set_mode(OPTIONS,mmode);
  twolame_set_error_protection(OPTIONS,TRUE);
    	//toolame_setPadding (options,TRUE);
  twolame_set_bitrate (OPTIONS,lameConf->bitrate);
  twolame_set_verbosity(OPTIONS, 2);
  if(twolame_init_params(OPTIONS))
  {
    printf("[TwoLame]Twolame init failed\n");
    return 0;
  }



  printf("[TwoLame]Libtoolame successfully initialized\n");
  return 1;
}
/**
        \fn getPacket
*/
uint8_t	AUDMEncoder_Twolame::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  int nbout;

  *samples = 1152; //FIXME
  *len = 0;
  ADM_assert(tmptail>=tmphead);
  if(!refillBuffer(_chunk ))
  {
    return 0;
  }

  if(tmptail-tmphead<_chunk)
  {
    return 0;
  }

  dither16(&(tmpbuffer[tmphead]),_chunk,_wavheader->channels);

  ADM_assert(tmptail>=tmphead);
  if (_wavheader->channels == 1)
  {
    nbout =twolame_encode_buffer(OPTIONS, (int16_t *)&(tmpbuffer[tmphead]),(int16_t *)&(tmpbuffer[tmphead]), _chunk, dest, 16 * 1024);
  }
  else
  {
    nbout = twolame_encode_buffer_interleaved(OPTIONS, (int16_t *)&(tmpbuffer[tmphead]), _chunk/2, dest, 16 * 1024);
  }
  tmphead+=_chunk;
  ADM_assert(tmptail>=tmphead);
  if (nbout < 0) {
    printf("\n Error !!! : %d\n", nbout);
    return 0;
  }
  *len=nbout;
  return 1;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TR_NOOP(#x)}

/**
    \fn configure
*/
uint8_t configure (void)
{
 int ret=0;

  uint32_t m=(uint32_t)twolameParam.mode;

    diaMenuEntry channelMode[] =
    {
        {ADM_STEREO, QT_TR_NOOP ("Stereo"), NULL},
        {ADM_JSTEREO, QT_TR_NOOP ("Joint stereo"), NULL},
        {ADM_MONO, QT_TR_NOOP ("Mono"), NULL}
    };
    diaMenuEntry bitrateM[]={
                              BITRATE(56),
                              BITRATE(64),
                              BITRATE(80),
                              BITRATE(96),
                              BITRATE(112),
                              BITRATE(128),
                              BITRATE(160),
                              BITRATE(192),
                              BITRATE(224),
                              BITRATE(384)
                          };
    diaElemMenu bitrate(&(twolameParam.bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);

    diaElemMenu menuMode (&m, QT_TR_NOOP ("C_hannel mode:"),SZT (channelMode), channelMode);

    diaElem *elems[]={&bitrate,&menuMode};

    if( diaFactoryRun(QT_TR_NOOP("TwoLame Configuration"),2,elems))
    {
        twolameParam.mode=(ADM_mode)m;
        return 1;
    }

    return 0;
}
// EOF

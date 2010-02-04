/***************************************************************************
    \file audioencoder_twolame.cpp
    copyright            : (C) 2006/2009 by mean
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

extern "C"
{
#include "twolame.h"
}

#define OPTIONS (twolame_options_struct *)_twolameOptions
uint32_t bitrateTwoLame=128;
static bool configure (void);
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
ADM_DECLARE_AUDIO_ENCODER_CONFIG(NULL,NULL,bitrateTwoLame);

/******************* / Declare plugin*******************************************************/
/**

*/
AUDMEncoder_Twolame::AUDMEncoder_Twolame(AUDMAudioFilter * instream,bool globalHeader)  :ADM_AudioEncoder    (instream)
{
  printf("[TwoLame] Creating Twolame\n");
  _twolameOptions=NULL;
  wavheader.encoding=WAV_MP2;
};

/**

*/
AUDMEncoder_Twolame::~AUDMEncoder_Twolame()
{
  printf("[TwoLame] Deleting TwoLame\n");
  if(_twolameOptions)
  {
    twolame_close((twolame_options_struct **)&_twolameOptions);
  }
  _twolameOptions=NULL;

};
/**
    \fn initialize

*/
bool AUDMEncoder_Twolame::initialize(void)
{
  int ret;
  TWOLAME_MPEG_mode mmode;
  uint32_t frequence;
  int channels=wavheader.channels;

  _twolameOptions = twolame_init();
  if (_twolameOptions == NULL)
    return 0;

  if(channels>2)
  {
    printf("[TwoLame]Too many channels\n");
    return 0;
  }
  wavheader.byterate=(bitrateTwoLame*1000)>>3;


  _chunk = 1152*channels;


  printf("[TwoLame]Incoming :fq : %"LU", channel : %"LU" bitrate: %"LU" \n",
        wavheader.frequency,channels,bitrateTwoLame);


  twolame_set_in_samplerate(OPTIONS, wavheader.frequency);
  twolame_set_out_samplerate (OPTIONS, wavheader.frequency);
  twolame_set_num_channels(OPTIONS, channels);
  if(channels==1) mmode=TWOLAME_MONO;
  else
      mmode = TWOLAME_STEREO;
  twolame_set_mode(OPTIONS,mmode);
  twolame_set_error_protection(OPTIONS,TRUE);
    	//toolame_setPadding (options,TRUE);
  twolame_set_bitrate (OPTIONS,bitrateTwoLame);
  twolame_set_verbosity(OPTIONS, 2);
  if(twolame_init_params(OPTIONS))
  {
    printf("[TwoLame]Twolame init failed\n");
    return false;
  }
  printf("[TwoLame]Libtoolame successfully initialized\n");
  return true;
}
/**
        \fn getPacket
*/
bool 	AUDMEncoder_Twolame::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  int nbout;
  int channels=wavheader.channels;
  *samples = 1152; //FIXME
  *len = 0;
  ADM_assert(tmptail>=tmphead);
  if(!refillBuffer(_chunk ))
  {
    return false;
  }

  if(tmptail-tmphead<_chunk)
  {
    return false;
  }

  dither16(&(tmpbuffer[tmphead]),_chunk,channels);

  ADM_assert(tmptail>=tmphead);
  if (channels == 1)
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
    printf("[TwoLame] Error !!! : %d\n", nbout);
    return false;
  }
  *len=nbout;
  return true;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TR_NOOP(#x)}

/**
    \fn configure
*/
bool configure (void)
{
 int ret=0;

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
    diaElemMenu bitrate(&(bitrateTwoLame),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);


    diaElem *elems[]={&bitrate};

    if( diaFactoryRun(QT_TR_NOOP("TwoLame Configuration"),1,elems))
    {
        
        return true;
    }

    return false;
}
// EOF

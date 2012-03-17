
/***************************************************************************
    copyright            : (C) 2002-6 by mean
    email                : fixounet@free.fr

    Interface to Aften

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

//
extern "C"
{
#include "aften/aften.h"
};
#include "audioencoder_aften.h"
#include "aften_encoder_desc.cpp"

#define _HANDLE ((AftenContext *)_handle)

static aften_encoder defaultConfig={128,};
/*
static AFTEN_encoderParam aftenParam= {
  128

};*/

static bool configure (CONFcouple **setup);


/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Aften);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Aften",
  "AC3 (Aften)",
  "Aften AC3 encoder plugin Mean/Gruntster 2008",
  6,                    // Max channels
  1,0,0,                // Version
  WAV_AC3,
  200,                  // Priority

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG();

/******************* / Declare plugin*******************************************************/


/**
    \fn AUDMEncoder_Aften

*/

AUDMEncoder_Aften::AUDMEncoder_Aften(AUDMAudioFilter * instream,bool globalHeader,
    CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
{
  uint32_t channels;
  ADM_info("[Aften] Creating aften\n");
  channels=instream->getInfo()->channels;
  _handle=(void *)new AftenContext;
  memset(_handle,0,sizeof(AftenContext));
  aften_set_defaults(_HANDLE);
  wavheader.encoding=WAV_AC3;
  _HANDLE->system.n_threads=1;
  _globalHeader= globalHeader;
  _config=defaultConfig;
  if(setup) // load config if possible
    ADM_paramLoad(setup,aften_encoder_param,&_config);

  switch(channels)
  {
    case 1:
        outputChannelMapping[1] = ADM_CH_FRONT_LEFT;
        break;
    case 2:
    	outputChannelMapping[0] = ADM_CH_FRONT_LEFT;
    	outputChannelMapping[1] = ADM_CH_FRONT_RIGHT;
      break;
    default :

    CHANNEL_TYPE *f=outputChannelMapping;

        *f++ = ADM_CH_FRONT_LEFT;
        *f++ = ADM_CH_FRONT_CENTER;
        *f++ = ADM_CH_FRONT_RIGHT;

        *f++ = ADM_CH_REAR_LEFT;
        *f++ = ADM_CH_REAR_RIGHT;

        *f++ = ADM_CH_LFE;
        break;
  }

};

/**
    \fn ~AUDMEncoder_Aften

*/

AUDMEncoder_Aften::~AUDMEncoder_Aften()
{
    ADM_info("[Aften] Deleting aften\n");
    if(_handle)
      aften_encode_close(_HANDLE);
    delete(_HANDLE);
    _handle=NULL;
};


/**
    \fn initialize

*/
bool AUDMEncoder_Aften::initialize(void)
{
int ret=0;
unsigned int mask;

    if(FLOAT_TYPE_FLOAT!=aften_get_float_type())
    {
            ADM_error("Aften was configured to use double !");
            return false;
    }

    wavheader.byterate=(_config.bitrate*1000)/8;
    _HANDLE->sample_format=A52_SAMPLE_FMT_FLT;
    _HANDLE->channels=wavheader.channels;
    _HANDLE->samplerate=wavheader.frequency;

    _HANDLE->params.bitrate=_config.bitrate;
    switch(wavheader.channels)
    {
        case 1: mask = 0x04;  break;
        case 2: mask = 0x03;  break;
        case 3: mask = 0x07;  break;
        case 4: mask = 0x107; break;
        case 5: mask = 0x37;  break;
        case 6: mask = 0x3F;  break;
      }

	aften_wav_channels_to_acmod(wavheader.channels, mask, &(_HANDLE->acmod), &(_HANDLE->lfe));

    int er= aften_encode_init(_HANDLE);
    if(er<0)
    {
      ADM_warning("[Aften] init error %d\n",er);
      return false;
    }
    _chunk=256*6*wavheader.channels;
    ADM_info("[Aften] Initialized with fd %u Channels %u bitrate %u\n",_HANDLE->samplerate,
                                                                    _HANDLE->channels,_HANDLE->params.bitrate);
    return true;
}


/**
        \fn encode
*/
bool    AUDMEncoder_Aften::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t count=0;
  int r;
  void *ptr;
_again:
        *len = 0;
        _chunk=256*6*wavheader.channels;
        if(!refillBuffer(_chunk ))
        {
          return 0;
        }
        ptr=(void *)&(tmpbuffer[tmphead]);
        ADM_assert(tmptail>=tmphead);
        reorderChannels(&(tmpbuffer[tmphead]),256*6,_incoming->getChannelMapping(),outputChannelMapping);
        r=aften_encode_frame(_HANDLE, dest,(void *)ptr
#ifndef AFTEN_08
      ,256*6
#endif
        );
        if(r<0)
        {
          printf("[Aften] Encoding error %d\n",r);
          return 0;
        }

        *samples=256*6;
        *len=r;
        tmphead+=_chunk;
        return 1;
}

/**
    \fn configure
*/
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TR_NOOP(#x)}

bool configure (CONFcouple **setup)
{
 int ret=0;
    aften_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,aften_encoder_param,&config);
    }


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
    diaElemMenu bitrate(&(config.bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);



    diaElem *elems[]={&bitrate};

    if( diaFactoryRun(QT_TR_NOOP("Aften Configuration"),1,elems))
    {
      if(*setup) delete *setup;
      *setup=NULL;
      ADM_paramSave(setup,aften_encoder_param,&config);
      defaultConfig=config;
      return true;
    }
    return false;
}
// EOF

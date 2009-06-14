
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
#if defined(USE_AFTEN_06)
	#include "aften.h"
#else	// Aften 0.05 & 0.07 onwards
	#include "aften/aften.h"
#endif
};
#include "audioencoder_aften_param.h"
#include "audioencoder_aften.h"

#define _HANDLE ((AftenContext *)_handle)

static AFTEN_encoderParam aftenParam= {
  128
  
};
static uint8_t configure (void);
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
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically 

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(aftenParam);

/******************* / Declare plugin*******************************************************/


/**
    \fn AUDMEncoder_Aften

*/

AUDMEncoder_Aften::AUDMEncoder_Aften(AUDMAudioFilter * instream)  :AUDMEncoder    (instream)
{
  uint32_t channels;
  channels=instream->getInfo()->channels;
  _handle=(void *)new AftenContext;
  memset(_handle,0,sizeof(AftenContext));
  aften_set_defaults(_HANDLE);
  _wavheader->encoding=WAV_AC3;
#if defined(USE_AFTEN_05) || defined(USE_AFTEN_06)
#elif defined(USE_AFTEN_07)
  _HANDLE->params.n_threads=1; // MThread collides with avidemux multithreading
#else
  _HANDLE->system.n_threads=1;
#endif
};

/**
    \fn ~AUDMEncoder_Aften

*/

AUDMEncoder_Aften::~AUDMEncoder_Aften()
{
    if(_handle)
      aften_encode_close(_HANDLE);
    delete(_HANDLE);
    _handle=NULL;

    printf("[Aften] Deleting aften\n");
    cleanup();
};


/**
    \fn initialize

*/
uint8_t AUDMEncoder_Aften::initialize(void)
{


int ret=0;

#if defined(USE_AFTEN_05) || defined(USE_AFTEN_06)
int mask;
#else
unsigned int mask;
#endif

    _wavheader->byterate=(aftenParam.bitrate*1000)/8;
    _HANDLE->sample_format=A52_SAMPLE_FMT_FLT;
    _HANDLE->channels=_wavheader->channels;
    _HANDLE->samplerate=_wavheader->frequency;
    
    _HANDLE->params.bitrate=aftenParam.bitrate;
    switch(_wavheader->channels)
    {
        case 1: mask = 0x04;  break;
        case 2: mask = 0x03;  break;
        case 3: mask = 0x07;  break;
        case 4: mask = 0x107; break;
        case 5: mask = 0x37;  break;
        case 6: mask = 0x3F;  break;
      }

#if defined(USE_AFTEN_05) || defined(USE_AFTEN_06)
	aften_wav_chmask_to_acmod(_wavheader->channels, mask, &(_HANDLE->acmod), &(_HANDLE->lfe));
#else
	aften_wav_channels_to_acmod(_wavheader->channels, mask, &(_HANDLE->acmod), &(_HANDLE->lfe));
#endif

   //   _HANDLE->params.verbose=2;
    int er= aften_encode_init(_HANDLE);
    if(er<0)
    {
      printf("[Aften] init error %d\n",er); 
      return 0;
    }
    _chunk=256*6*_wavheader->channels;
    printf("[Aften] Initialized with fd %u Channels %u bitrate %u\n",_HANDLE->samplerate,
                                                                    _HANDLE->channels,_HANDLE->params.bitrate);
    return 1;
}


/**
        \fn getPacket
*/
uint8_t	AUDMEncoder_Aften::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t count=0;
  int r;
  void *ptr;
_again:
        *len = 0;
        _chunk=256*6*_wavheader->channels;
        if(!refillBuffer(_chunk ))
        {
          return 0; 
        }
        ptr=(void *)&(tmpbuffer[tmphead]);
        ADM_assert(tmptail>=tmphead);

#ifdef USE_AFTEN_05
		aften_remap_wav_to_a52(ptr, 256*6, _wavheader->channels, A52_SAMPLE_FMT_FLT, _HANDLE->acmod, _HANDLE->lfe);
#else
		aften_remap_wav_to_a52(ptr, 256*6, _wavheader->channels, A52_SAMPLE_FMT_FLT, _HANDLE->acmod);
#endif

        r=aften_encode_frame(_HANDLE, dest,(void *)ptr);
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
uint8_t configure (void)
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
    diaElemMenu bitrate(&(aftenParam.bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);
  
    

    diaElem *elems[]={&bitrate};
    
    return ( diaFactoryRun(QT_TR_NOOP("Aften Configuration"),1,elems));
    
}
// EOF

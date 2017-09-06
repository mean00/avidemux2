
/***************************************************************************
    copyright            : (C) 2002-6 by mean
    email                : fixounet@free.fr

    Interface to FAAC

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

#include "opus/opus.h"
#include "audioencoder_opus.h"

#include "opus_encoder_desc.cpp"

#define OPUS_DEFAULT_CONF {128}

static opus_encoder defaultConfig = OPUS_DEFAULT_CONF;

static bool configure(CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);

/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Opus);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Opus",            
  "Opus Encoder",      
  "Opus encoder plugin Mean 2017",
  2,                    // Max channels
  1,0,0,                // Version
  WAV_OPUS,
  200,                  // Priority
 
  NULL,         //** put your own function here**
  getDefaultConfiguration,
  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG( );

/******************* / Declare plugin*******************************************************/

AUDMEncoder_Opus::AUDMEncoder_Opus(AUDMAudioFilter * instream,bool globalHeader,
    CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
{
  uint32_t channels;
  channels=instream->getInfo()->channels;
  _globalHeader=globalHeader;
  _handle=NULL;
  switch(channels)
  {
    case 1:
        outputChannelMapping[0] = ADM_CH_MONO;
        break;
    case 2:
    	outputChannelMapping[0] = ADM_CH_FRONT_LEFT;
    	outputChannelMapping[1] = ADM_CH_FRONT_RIGHT;
      break;
    default :
        ADM_warning("Unsupported channel mapping\n");
        break;
  }
  wavheader.encoding=WAV_OPUS;
  _config=defaultConfig;
  if(setup) // load config if possible
    ADM_paramLoad(setup,opus_encoder_param,&_config);
};

/**
    \fn ~AUDMEncoder_Opus
*/
AUDMEncoder_Opus::~AUDMEncoder_Opus()
{
    if(_handle)
        opus_encoder_destroy(_handle);
    _handle=NULL;
    if(ordered) delete [] ordered;
    ordered=NULL;
    printf("[Opus] Deleting faac\n");

};


/**
    \fn initialize

*/
bool AUDMEncoder_Opus::initialize(void)
{
unsigned long int samples_input, max_bytes_output;
int ret=0;
int channels=wavheader.channels;

    printf("[Opus] Incoming Fq :%u\n",wavheader.frequency);
    if(channels>2)
    {
        ADM_warning("Unsupported channel configuration\n");
        return false;
    }
    switch(wavheader.frequency)
    {
        case 8000:
        case 12000:
        case 16000:
        case 24000:
        case 48000:
            break;
    default:
            ADM_warning("Unsupported frequency configuration\n");
            return false;            
    }
    
    //
    int err=0;
    _handle = opus_encoder_create(wavheader.frequency,wavheader.channels,OPUS_APPLICATION_AUDIO ,&err);
    if(!_handle)
    {
          ADM_warning("[Opus ]Cannot open opus with fq=%d, channel=%d, error=%d\n",wavheader.frequency,wavheader.channels, err);
          return false;
    }
    //
    opus_encoder_ctl(_handle,OPUS_SET_BITRATE(_config.bitrate));
    // update
    wavheader.byterate=(_config.bitrate*1000)/8;
    wavheader.blockalign=4096;
    wavheader.bitspersample=0;
    wavheader.encoding=WAV_OPUS;
    _chunk=samples_input;

    ordered=new float[_chunk];
    
    return true;
}
/**
    \fn refillBuffer
*/
//_____________________________________________
//  Need to multiply the float by 32767, can't use
//  generic fill buffer
//----------------------------------------------
uint8_t AUDMEncoder_Opus::refillBuffer(int minimum)
{
  uint32_t filler=wavheader.frequency*wavheader.channels;
  uint32_t nb;
  AUD_Status status;
  if(AudioEncoderRunning!=_state) return 0;
  while(1)
  {
    ADM_assert(tmptail>=tmphead);
    if((tmptail-tmphead)>=minimum) return 1;
  
    if(tmphead && tmptail>filler/2)
    {
      memmove(&tmpbuffer[0],&tmpbuffer[tmphead],(tmptail-tmphead)*sizeof(float)); 
      tmptail-=tmphead;
      tmphead=0;
    }
    ADM_assert(filler>tmptail);
    nb=_incoming->fill( (filler-tmptail)/2,&tmpbuffer[tmptail],&status);
    if(!nb)
    {
      if(status!=AUD_END_OF_STREAM) ADM_assert(0);
      
      if((tmptail-tmphead)<minimum)
      {
        memset(&tmpbuffer[tmptail],0,sizeof(float)*(minimum-(tmptail-tmphead)));
        tmptail=tmphead+minimum;
        _state=AudioEncoderNoInput;  
        return minimum;
      }
      else continue;
    } else
    {
      float *s=&(tmpbuffer[tmptail]);
      for(int i=0;i<nb;i++)
      {
        *s=*s*32767.;
        s++;
      }
      tmptail+=nb;
    }
  }
}
#define SIZE_INTERNAL 64*1024 
#define FA_BUFFER_SIZE (SIZE_INTERNAL/4)
/**
    \fn encode
*/
bool	AUDMEncoder_Opus::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
 uint32_t count=0; 
 int channels=wavheader.channels;
_again:
        *samples = _chunk/channels;
        *len = 0;

        if(!refillBuffer(_chunk ))
        {
          return 0; 
        }
        ADM_assert(tmptail>=tmphead);
        
        *len=opus_encode_float(_handle,&(tmpbuffer[tmphead]),_chunk,dest,_chunk);        
        if(!*len) 
        {
          count++;
          if(count<20)
            goto _again;
          *samples=0;
        }
        tmphead+=_chunk;
        *samples=_chunk;
        return 1;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TRANSLATE_NOOP("faac",#x)}

/**
    \fn configure
*/
bool configure (CONFcouple **setup)
{
 int ret=0;
    opus_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,opus_encoder_param,&config);
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
    diaElemMenu bitrate(&(config.bitrate),   QT_TRANSLATE_NOOP("Opus","_Bitrate:"), SZT(bitrateM),bitrateM);
  
    

    diaElem *elems[]={&bitrate};
    
    if ( diaFactoryRun(QT_TRANSLATE_NOOP("faac","Opus Configuration"),1,elems))
    {
        if(*setup) delete *setup;
        *setup=NULL;
        ADM_paramSave(setup,opus_encoder_param,&config);
        defaultConfig=config;
        return true;
    }
    return false;
}

void getDefaultConfiguration(CONFcouple **c)
{
	opus_encoder config = OPUS_DEFAULT_CONF;

	ADM_paramSave(c, opus_encoder_param, &config);
}
// EOF

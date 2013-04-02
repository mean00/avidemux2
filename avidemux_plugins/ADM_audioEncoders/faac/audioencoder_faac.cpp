
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

#include "faac.h"
#include "audioencoder_faac.h"

#include "faac_encoder_desc.cpp"

#define FAAC_DEFAULT_CONF {128}

static faac_encoder defaultConfig = FAAC_DEFAULT_CONF;

static bool configure(CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);

/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Faac);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Faac",            
  "AAC (Faac)",      
  "Faac AAC encoder plugin Mean 2008",             
  6,                    // Max channels
  1,0,0,                // Version
  WAV_AAC,
  200,                  // Priority
 
  NULL,         //** put your own function here**
  getDefaultConfiguration,
  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG( );

/******************* / Declare plugin*******************************************************/

AUDMEncoder_Faac::AUDMEncoder_Faac(AUDMAudioFilter * instream,bool globalHeader,
    CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
{
  uint32_t channels;
  channels=instream->getInfo()->channels;
  _globalHeader=globalHeader;
  switch(channels)
  {
    case 1:outputChannelMapping[1] = ADM_CH_FRONT_LEFT;break;
    case 2:
    	outputChannelMapping[0] = ADM_CH_FRONT_LEFT;
    	outputChannelMapping[1] = ADM_CH_FRONT_RIGHT;
      break;
    default :

    CHANNEL_TYPE *f=outputChannelMapping;
        *f++ = ADM_CH_FRONT_CENTER;
        *f++ = ADM_CH_FRONT_LEFT;
        *f++ = ADM_CH_FRONT_RIGHT;

        *f++ = ADM_CH_REAR_LEFT;
        *f++ = ADM_CH_REAR_RIGHT;
        *f++ = ADM_CH_LFE;
        

  }
  wavheader.encoding=WAV_AAC;
  _config=defaultConfig;
  if(setup) // load config if possible
    ADM_paramLoad(setup,faac_encoder_param,&_config);
};

/**
    \fn ~AUDMEncoder_Faac
*/
AUDMEncoder_Faac::~AUDMEncoder_Faac()
{
    if(_handle)
        faacEncClose(_handle);
    _handle=NULL;
    if(ordered) delete [] ordered;
    ordered=NULL;
    printf("[FAAC] Deleting faac\n");

};


/**
    \fn initialize

*/
bool AUDMEncoder_Faac::initialize(void)
{
unsigned long int samples_input, max_bytes_output;
faacEncConfigurationPtr cfg;
int ret=0;
int channels=wavheader.channels;

    printf("[FAAC] Incoming Fq :%u\n",wavheader.frequency);
     _handle = faacEncOpen(wavheader.frequency,
                                 channels,
                                 &samples_input,
                                &max_bytes_output);
    if(!_handle)
    {
          printf("[FAAC]Cannot open faac with fq=%"PRIu32" chan=%"PRIu32" br=%"PRIu32"\n",
          wavheader.frequency,channels,_config.bitrate);
          return 0;
    }
    printf(" [FAAC] : Sample input:%"PRIu32", max byte output%"PRIu32" \n",(uint32_t)samples_input,(uint32_t)max_bytes_output);
    cfg= faacEncGetCurrentConfiguration(_handle);
    
    // Set default conf, same as ffmpeg
    cfg->aacObjectType = LOW;
    cfg->mpegVersion = MPEG4;
    cfg->bandWidth= (wavheader.frequency*3)/4; // Should be relevant
    cfg->useTns = 0;
    cfg->allowMidside = 0;
    cfg->bitRate = (_config.bitrate*1000)/channels; // It is per channel
    cfg->outputFormat = 0; // 0 Raw 1 ADTS
    cfg->inputFormat = FAAC_INPUT_FLOAT;
    cfg->useLfe=0;	
    if (!(ret=faacEncSetConfiguration(_handle, cfg))) 
    {
        printf("[FAAC] Cannot set conf for faac with fq=%"PRIu32" chan=%"PRIu32" br=%"PRIu32" (err:%d)\n",
				wavheader.frequency,channels,_config.bitrate,ret);
	return 0;
    }
     unsigned char *data=NULL;
     unsigned long size=0;
     if((ret=faacEncGetDecoderSpecificInfo(_handle, &data,&size)))
     {
        printf("FAAC: GetDecoderSpecific info failed (err:%d)\n",ret);
        return 0;
     }
     _extraSize=size;
     _extraData=new uint8_t[size];
     memcpy(_extraData,data,size);

    // update
     wavheader.byterate=(_config.bitrate*1000)/8;
//    _wavheader->dwScale=1024;
//    _wavheader->dwSampleSize=0;
    wavheader.blockalign=4096;
    wavheader.bitspersample=0;

    _chunk=samples_input;

    ordered=new float[_chunk];
    printf("[Faac] Initialized :\n");
    
    printf("[Faac]Version        : %s\n",cfg->name);
    printf("[Faac]Bitrate        : %"PRIu32"\n",(uint32_t)cfg->bitRate);
    printf("[Faac]Mpeg2 (1)/4(0) : %u\n",cfg->mpegVersion);
    printf("[Faac]Use lfe      ) : %u\n",cfg->useLfe);
    printf("[Faac]Sample output  : %"PRIu32"\n",_chunk / channels);
    printf("[Faac]Bitrate        : %lu\n",cfg->bitRate*channels);

    
    return 1;
}
/**
    \fn refillBuffer
*/
//_____________________________________________
//  Need to multiply the float by 32767, can't use
//  generic fill buffer
//----------------------------------------------
uint8_t AUDMEncoder_Faac::refillBuffer(int minimum)
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
bool	AUDMEncoder_Faac::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
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
        reorder(&(tmpbuffer[tmphead]),ordered,*samples,_incoming->getChannelMapping(),outputChannelMapping);
        *len = faacEncEncode(_handle, (int32_t *)ordered, _chunk, dest, FA_BUFFER_SIZE);
        if(!*len) 
        {
          count++;
          if(count<20)
            goto _again;
          *samples=0;
        }
        tmphead+=_chunk;
        return 1;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TR_NOOP(#x)}

/**
    \fn configure
*/
bool configure (CONFcouple **setup)
{
 int ret=0;
    faac_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,faac_encoder_param,&config);
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
    
    if ( diaFactoryRun(QT_TR_NOOP("Aften Configuration"),1,elems))
    {
        if(*setup) delete *setup;
        *setup=NULL;
        ADM_paramSave(setup,faac_encoder_param,&config);
        defaultConfig=config;
        return true;
    }
    return false;
}

void getDefaultConfiguration(CONFcouple **c)
{
	faac_encoder config = FAAC_DEFAULT_CONF;

	ADM_paramSave(c, faac_encoder_param, &config);
}
// EOF

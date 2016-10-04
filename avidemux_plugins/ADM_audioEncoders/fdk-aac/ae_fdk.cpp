
/***************************************************************************
    copyright            : (C) 2002-6 by mean
    email                : fixounet@free.fr

    Interface to FDKAAC

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

#include "fdk-aac/aacenc_lib.h"
#include "ae_fdk.h"

#include "fdk_encoder_desc.cpp"

#define FDKAAC_DEFAULT_CONF {128}

static fdk_encoder defaultConfig = FDKAAC_DEFAULT_CONF;

static bool configure(CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);


/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Fdkaac);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Faac",            
  "AAC (FDK)",      
  "FDK AAC encoder plugin Mean 2016",
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

AUDMEncoder_Fdkaac::AUDMEncoder_Fdkaac(AUDMAudioFilter * instream,bool globalHeader,
    CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
{
  uint32_t channels;
  _inited=false;
  channels=instream->getInfo()->channels;
  _globalHeader=globalHeader;
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
    ADM_paramLoad(setup,fdk_encoder_param,&_config);
};

/**
    \fn ~AUDMEncoder_Fdkaac
*/
AUDMEncoder_Fdkaac::~AUDMEncoder_Fdkaac()
{
    if(_inited)
    {
        aacEncClose(&_aacHandle);
        _inited=false;
    }
    if(ordered) delete [] ordered;
    ordered=NULL;
    printf("[FDKAAC] Deleting faac\n");

};


bool AUDMEncoder_Fdkaac::setParam(const char *name, int nameAsInt, int value)
{
   int error=   aacEncoder_SetParam(_aacHandle,(AACENC_PARAM)nameAsInt,value); 
    if(error!=AACENC_OK) 
    {
        ADM_warning("%s failed\n",name); 
        return false;
    }
    ADM_info("Parameter %s = %d\n",name,(int)value);
    // Read it back
    int newVal=aacEncoder_GetParam(_aacHandle,(AACENC_PARAM)nameAsInt);
    ADM_info("Asked = %d, finally it is %d\n",value,newVal);
    if(value!=newVal)
      {
        ADM_warning("Parameter setting failed\n");
        
      }
    return true;
}

/**
    \fn initialize

*/
bool AUDMEncoder_Fdkaac::initialize(void)
{
unsigned long int samples_input, max_bytes_output;
int ret=0;
int channels=wavheader.channels;

    printf("[FDKAAC] Incoming Fq :%u\n",wavheader.frequency);
    AACENC_ERROR error;
    error= aacEncOpen(&_aacHandle,0,0);
    if(error!=AACENC_OK)
    {
        ADM_warning("Cannot open fdk AAC for channels=%d\n",channels);
          return 0;
    }
    // fine tuning
#define SET_PARAM(name,value) if(!setParam(#name,name,value)) ADM_warning("oops\n");

    SET_PARAM(AACENC_TRANSMUX,TT_MP4_RAW) // Raw binary     
    SET_PARAM(AACENC_AOT, AOT_AAC_LC) // Mpeg4 Low
    SET_PARAM(AACENC_BITRATE,_config.bitrate*1000)
    SET_PARAM(AACENC_SAMPLERATE,(wavheader.frequency))
    
    
    // Read back
#define GET_PARAM(name) ADM_info(#name" = %d\n",(int)aacEncoder_GetParam(_aacHandle,name));
    ADM_info("Read back parameters:\n");
    GET_PARAM(AACENC_AOT)
    GET_PARAM(AACENC_BITRATE)
    GET_PARAM(AACENC_BITRATEMODE)
    GET_PARAM(AACENC_SAMPLERATE)
    GET_PARAM(AACENC_SBR_MODE)
    GET_PARAM(AACENC_GRANULE_LENGTH)
    GET_PARAM(AACENC_CHANNELMODE)
    GET_PARAM(AACENC_CHANNELORDER)
    GET_PARAM(AACENC_SBR_RATIO)
    GET_PARAM(AACENC_AFTERBURNER)
    GET_PARAM(AACENC_BANDWIDTH)
    GET_PARAM(AACENC_TRANSMUX)
    GET_PARAM(AACENC_HEADER_PERIOD)
    GET_PARAM(AACENC_SIGNALING_MODE)
    GET_PARAM(AACENC_TPSUBFRAMES)
    GET_PARAM(AACENC_PROTECTION)
    GET_PARAM(AACENC_ANCILLARY_BITRATE)
    GET_PARAM(AACENC_METADATA_MODE)
        
    // Get specific configuration
    AACENC_InfoStruct        pInfo;
    memset(&pInfo,0,sizeof(pInfo));
    if(AACENC_OK != aacEncInfo(_aacHandle,&pInfo))
    {
        ADM_warning("Cannot get info\n");
        return false;
    }
    int size=pInfo.confSize;
     _extraSize=size;
     _extraData=new uint8_t[size];
     memcpy(_extraData,pInfo.confBuf,size);

    // update
    wavheader.byterate=(_config.bitrate*1000)/8;
    wavheader.blockalign=4096;
    wavheader.bitspersample=0;

    _chunk=pInfo.frameLength*wavheader.channels;

    ordered=new float[_chunk];
    ADM_info("[Fdk] Initialized :\n");
#if 0
    ADM_info("[Fdk]Version        : %s\n",cfg->name);
    ADM_info("[Fdk]Bitrate        : %" PRIu32"\n",(uint32_t)cfg->bitRate);
    ADM_info("[Fdk]Mpeg2 (1)/4(0) : %u\n",cfg->mpegVersion);
    ADM_info("[Fdk]Use lfe      ) : %u\n",cfg->useLfe);
    ADM_info("[Fdk]Sample output  : %" PRIu32"\n",_chunk / channels);
    ADM_info("[Fdk]Bitrate        : %lu\n",cfg->bitRate*channels);
#endif
    
    return true;
}
/**
    \fn refillBuffer
*/
//_____________________________________________
//  Need to multiply the float by 32767, can't use
//  generic fill buffer
//----------------------------------------------
uint8_t AUDMEncoder_Fdkaac::refillBuffer(int minimum)
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
bool	AUDMEncoder_Fdkaac::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  return false;
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
    //    *len = aacEncEncode(_aacHandle, (int32_t *)ordered, _chunk, dest, FA_BUFFER_SIZE);
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
#define BITRATE(x) {x,QT_TRANSLATE_NOOP("faac",#x)}

/**
    \fn configure
*/
bool configure (CONFcouple **setup)
{
 int ret=0;
    fdk_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,fdk_encoder_param,&config);
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
    diaElemMenu bitrate(&(config.bitrate),   QT_TRANSLATE_NOOP("faac","_Bitrate:"), SZT(bitrateM),bitrateM);
  
    

    diaElem *elems[]={&bitrate};
    
    if ( diaFactoryRun(QT_TRANSLATE_NOOP("faac","Faac Configuration"),1,elems))
    {
        if(*setup) delete *setup;
        *setup=NULL;
        ADM_paramSave(setup,fdk_encoder_param,&config);
        defaultConfig=config;
        return true;
    }
    return false;
}

void getDefaultConfiguration(CONFcouple **c)
{
	fdk_encoder config = FDKAAC_DEFAULT_CONF;

	ADM_paramSave(c, fdk_encoder_param, &config);
}
// EOF

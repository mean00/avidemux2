/***************************************************************************
                        
    copyright            : (C) 2002-2006 by mean
    email                : fixounet@free.fr
    
    Interface to FFmpeg mpeg1/2 audio encoder
    
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

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "audioencoder.h"
#include "audioencoderInternal.h"

#include ADM_AE_SET

#include "audioencoder_lavcodec.h"

static bool configure (void);
static uint32_t lavBitrate=128;
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Lavcodec);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  ADM_LAV_NAME,
  ADM_LAV_MENU,
  ADM_LAV_DESC,
  ADM_LAV_MAX_CHANNEL,
  ADM_LAV_VERSION,
  makeName(WAV),

  100,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically 

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(NULL,NULL,lavBitrate);

/******************* / Declare plugin*******************************************************/
#define CONTEXT ((AVCodecContext  	*)_context)


/**
    \fn AUDMEncoder_Lavcodec
*/
AUDMEncoder_Lavcodec::AUDMEncoder_Lavcodec(AUDMAudioFilter * instream,bool globalHeader)  :ADM_AudioEncoder    (instream)
{
  
  _context=NULL;
  _globalHeader=globalHeader;
   printf("[Lavcodec] Creating Lavcodec audio encoder (0x%x)\n",makeName(WAV));
#if defined(ADM_LAV_GLOBAL_HEADER) // Only AAC ?
    if(globalHeader)
        _globalHeader=true;
    else
#endif
    _globalHeader=false;

  wavheader.encoding=makeName(WAV);
};
/**
    \fn extraData
*/
uint8_t AUDMEncoder_Lavcodec::extraData(uint32_t *l,uint8_t **d)
{
    ADM_assert(_context);
    int size=0;
    size=CONTEXT->extradata_size;
    if(size)
    {
        *d=CONTEXT->extradata;
        *l=(uint32_t)size;
    }
    else    
    {
        *d=NULL;
        *l=0;
    }
    return true;
}
/**
    \fn ~AUDMEncoder_Lavcodec
*/

AUDMEncoder_Lavcodec::~AUDMEncoder_Lavcodec()
{
  printf("[Lavcodec] Deleting Lavcodec\n");
  if(_context)
  {
    avcodec_close(CONTEXT);
    av_free(_context);
  }
  _context=NULL;
};

/**
    \fn initialize
*/
bool AUDMEncoder_Lavcodec::initialize(void)
{
  int ret;
  _context=( void *)avcodec_alloc_context();
  

  if( _incoming->getInfo()->channels>ADM_LAV_MAX_CHANNEL)
  {
    ADM_error("[Lavcodec]Too many channels\n");
    return 0; 
  }
  wavheader.byterate=(lavBitrate*1000)>>3;         
      
  _chunk = ADM_LAV_SAMPLE_PER_P*wavheader.channels; // AC3
  ADM_info("[Lavcodec]Incoming : fq : %"LU", channel : %"LU" bitrate: %"LU" \n",
  wavheader.frequency,wavheader.channels,lavBitrate);
  
  
  CONTEXT->channels     =  wavheader.channels;
  CONTEXT->sample_rate  =  wavheader.frequency;
  CONTEXT->bit_rate     = (lavBitrate*1000); // bits -> kbits

  if(true==_globalHeader)
  {
    ADM_info("Configuring audio codec to use global headers\n");
    CONTEXT->flags|=CODEC_FLAG_GLOBAL_HEADER;
  }

  AVCodec *codec;
  CodecID codecID;

  
  codecID=makeName(CODEC_ID);
  codec = avcodec_find_encoder(codecID);
  ADM_assert(codec);
  
  ret = avcodec_open(CONTEXT, codec);
  if (0> ret) 
  {
    ADM_error("[Lavcodec] init failed err : %d!\n",ret);
    return 0;
  }


  ADM_info("[Lavcodec]Lavcodec successfully initialized,wavTag : 0x%x\n",makeName(WAV));
  return 1;       
}
/**
    \fn encode
*/
bool	AUDMEncoder_Lavcodec::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t nbout;
  int channels=wavheader.channels;
  *samples = _chunk/channels; //FIXME
  *len = 0;
  if(AudioEncoderStopped==_state)
        return false;

   refillBuffer (_chunk);
   if(AudioEncoderNoInput==_state)
    {
        int left=tmptail-tmphead;
        if (left < _chunk)
        {
            if(left) // Last block
            {
               dither16(&(tmpbuffer[tmphead]),left,channels);
               ADM_assert(tmptail>=tmphead);
#warning buffer overread
               nbout = avcodec_encode_audio(CONTEXT, dest, 5000, (short *) &(tmpbuffer[tmphead]));
               tmphead=tmptail;
               *samples = left/channels; 
               *len=nbout;
               ADM_info("[Lav] Last audio block\n");
               goto cnt;
            }
              // Flush
               ADM_info("[Lav] Flush\n");
              _state=AudioEncoderStopped;
              if(CONTEXT->codec->capabilities & CODEC_CAP_DELAY)
              {
                  nbout=avcodec_encode_audio(CONTEXT, dest, 5000,NULL);
                  if(nbout<0) 
                  {
                        ADM_warning("Error while flushing lame\n");
                        return false;   
                  }
                        
                  *len=nbout;
                  *samples=_chunk/channels;  
                  ADM_info("[Lav] Flushing, last block is %d bytes\n",nbout);
                  return true;
              }else
              {
              }
              ADM_info("[Lav] No data to flush\n",nbout);
              return true;
        }
    }


  dither16(&(tmpbuffer[tmphead]),_chunk,channels);

  ADM_assert(tmptail>=tmphead);
  nbout = avcodec_encode_audio(CONTEXT, dest, 5000, (short *) &(tmpbuffer[tmphead]));

  tmphead+=_chunk;
cnt:
  if (nbout < 0) 
  {
    ADM_error("[Lavcodec] Error !!! : %"LD"\n", nbout);
    return 0;
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
    diaElemMenu bitrate(&(lavBitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);
  
    

    diaElem *elems[]={&bitrate};
    
    return ( diaFactoryRun(QT_TR_NOOP(ADM_LAV_MENU" (lav) Configuration"),1,elems));
    
}	

// EOF

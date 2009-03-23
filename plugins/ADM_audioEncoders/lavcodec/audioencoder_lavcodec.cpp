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
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
//#include <lame/lame.h>
#include "audioencoder.h"
#include "audioencoderInternal.h"
//


#include "ADM_lavcodec.h"



#define Join(x,y) x##_##y
#if defined(ADM_LAV_MP2) && !defined(ADM_LAV_AC3)
  #define makeName(x) Join(x,MP2)
  #define AUDMEncoder_Lavcodec AUDMEncoder_Lavcodec_MP2
#else
 #if !defined(ADM_LAV_MP2) && defined(ADM_LAV_AC3)
  #define makeName(x) Join(x,AC3)
  #define AUDMEncoder_Lavcodec AUDMEncoder_Lavcodec_AC3
 #else
   #error
 #endif
#endif

#include "audioencoder_lavcodec.h"

typedef struct 
{
    uint32_t bitrate;
}LavAudioEncoder_PARAM;
static LavAudioEncoder_PARAM lavConfig={128};
static bool configure (void);
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Lavcodec);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
#ifdef ADM_LAV_MP2      
  "LavMP2",            
  "MP2 (lav)",      
  "MP2 LavCodec encoder plugin Mean 2008",             
  2,                    // Max channels
  1,0,0,                // Version
#else
  

 "LavAC3",            
  "AC3 (lav)",      
  "AC3 LavEncoder encoder plugin Mean 2008",             
  6,                    // Max channels
  1,0,0,                // Version
#endif
  makeName(WAV),

  100,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically 

  NULL,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(lavConfig);

/******************* / Declare plugin*******************************************************/
#define CONTEXT ((AVCodecContext  	*)_context)


/**
    \fn AUDMEncoder_Lavcodec
*/
AUDMEncoder_Lavcodec::AUDMEncoder_Lavcodec(AUDMAudioFilter * instream)  :ADM_AudioEncoder    (instream)
{
  
  _context=NULL;
   printf("[Lavcodec] Creating Lavcodec audio encoder (0x%x)\n",makeName(WAV));

  wavheader.encoding=makeName(WAV);
  
  
};

/**
    \fn ~AUDMEncoder_Lavcodec
*/

AUDMEncoder_Lavcodec::~AUDMEncoder_Lavcodec()
{
  printf("[Lavcodec] Deleting Lavcodec\n");
  if(_context)
  {
    avcodec_close(CONTEXT);
    ADM_dealloc(_context);
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
  

#ifdef ADM_LAV_MP2      
  if( _incoming->getInfo()->channels>2)
  {
    printf("[Lavcodec]Too many channels\n");
    return 0; 
  }
#endif
  wavheader.byterate=(lavConfig.bitrate*1000)>>3;         
      
#ifdef ADM_LAV_MP2 
    _chunk = 1152*wavheader.channels;
#else
    _chunk = 1536*wavheader.channels; // AC3
#endif
  printf("[Lavcodec]Incoming : fq : %lu, channel : %lu bitrate: %lu \n",
         wavheader.frequency,wavheader.channels,lavConfig.bitrate);
  
  
  CONTEXT->channels     =  wavheader.channels;
  CONTEXT->sample_rate  =  wavheader.frequency;
  CONTEXT->bit_rate     = (lavConfig.bitrate*1000); // bits -> kbits

  AVCodec *codec;
  CodecID codecID;

  
  codecID=makeName(CODEC_ID);
  codec = avcodec_find_encoder(codecID);
  ADM_assert(codec);
  
  ret = avcodec_open(CONTEXT, codec);
  if (0> ret) 
  {
    printf("[Lavcodec] init failed err : %d!\n",ret);
    return 0;
  }


  printf("[Lavcodec]Lavcodec successfully initialized,wavTag : 0x%x\n",makeName(WAV));
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

  if(!refillBuffer(_chunk ))
  {
    return 0; 
  }
        
  if(tmptail-tmphead<_chunk)
  {
    return 0; 
  }

  dither16(&(tmpbuffer[tmphead]),_chunk,channels);

  ADM_assert(tmptail>=tmphead);
  nbout = avcodec_encode_audio(CONTEXT, dest, 5000, (short *) &(tmpbuffer[tmphead]));

  tmphead+=_chunk;
  if (nbout < 0) 
  {
    printf("[Lavcodec] Error !!! : %ld\n", nbout);
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
    diaElemMenu bitrate(&(lavConfig.bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);
  
    

    diaElem *elems[]={&bitrate};
    
    return ( diaFactoryRun(QT_TR_NOOP("Aften Configuration"),1,elems));
    
}	

// EOF

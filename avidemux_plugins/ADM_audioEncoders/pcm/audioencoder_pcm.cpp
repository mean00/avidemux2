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
#include "audioencoder.h"
#include "audioencoderInternal.h"
#include "audioencoder_pcm.h"
#include "pcm_encoder_desc.cpp"

#define PCM_DEFAULT_CONF {OUTPUT_MODE_PCM}

static pcm_encoder defaultConfig = PCM_DEFAULT_CONF;

static bool configure(CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);

/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_PCM);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "PCM",
  "PCM",
  "PCM encoder plugin Mean 2008",
  8,                    // Max channels
  1,0,0,                // Version
  WAV_PCM,
  200,                  // Priority
  NULL,                 //** put your own function here**
  getDefaultConfiguration,
  NULL   // Defined by macro automatically

};

ADM_DECLARE_AUDIO_ENCODER_CONFIG();


/******************* / Declare plugin*******************************************************/



// Ctor: Duplicate
//__________

AUDMEncoder_PCM::AUDMEncoder_PCM(AUDMAudioFilter * instream,bool globalHeader, CONFcouple *setup)
  :ADM_AudioEncoder    (instream,setup)
{
  ADM_info("Creating (L)PCM encoder.\n");
  wavheader.bitspersample=16;
  wavheader.blockalign=2*wavheader.channels;

  if(!setup || false == ADM_paramLoad(setup,pcm_encoder_param,&_config))
        _config = defaultConfig;

  switch(_config.output_mode)
  {
        case OUTPUT_MODE_LPCM:
            wavheader.encoding = WAV_LPCM;
            break;
        case OUTPUT_MODE_PCM:
            wavheader.encoding = WAV_PCM;
            break;
        default:
            ADM_warning("Invalid output mode %d, using PCM.\n",_config.output_mode);
            wavheader.encoding = WAV_PCM;
            break;
  }
};


AUDMEncoder_PCM::~AUDMEncoder_PCM()
{
    ADM_info("Deleting (L)PCM encoder.\n");
}

/**
    \fn initialize
*/
bool AUDMEncoder_PCM::initialize(void)
{

  wavheader.byterate=wavheader.channels*wavheader.frequency*2;
  _chunk = (wavheader.frequency/100)*wavheader.channels*2;

  printf("[PCM] Incoming fq : %" PRIu32", channel : %" PRIu32" \n",wavheader.frequency,wavheader.channels);
  printf("[PCM] Encoder initialized in %s mode.\n",wavheader.encoding == WAV_PCM ? "PCM" : "LPCM");
  return 1;
}
/**
    \fn getPacket
*/
bool         AUDMEncoder_PCM::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  *samples = _chunk; //FIXME
  *len = 0;
  uint32_t channels=wavheader.channels;
  if(!refillBuffer(_chunk ))
  {
    return 0;
  }

  if(tmptail-tmphead<_chunk)
  {
    return 0;
  }
        // Do in place replace
  dither16(&(tmpbuffer[tmphead]),_chunk,channels);
  if(wavheader.encoding == WAV_PCM)
    memcpy(dest,&(tmpbuffer[tmphead]),_chunk*2);
  else
  {
    uint16_t *in,*out,tmp;
    in=(uint16_t*)&(tmpbuffer[tmphead]);
    out=(uint16_t *)dest;
    for(int i=0;i<_chunk;i++)
    {
      tmp=*in++;
      tmp=((tmp&0xff)<<8)+(tmp>>8);
      *out++=tmp;
    }
  }
  tmphead+=_chunk;
  *len=_chunk*2;
  *samples=_chunk/channels;
  return 1;
}

void getDefaultConfiguration(CONFcouple **c)
{
    pcm_encoder cfg = PCM_DEFAULT_CONF;
    ADM_paramSave(c, pcm_encoder_param, &cfg);
}

/**
    \fn configure
    \brief Dialog to choose encoding mode
    @return true on success, false on failure
*/
bool configure(CONFcouple **setup)
{
    pcm_encoder cfg;
    if(!(*setup) || false == ADM_paramLoad(*setup,pcm_encoder_param,&cfg))
        cfg = defaultConfig;

    diaMenuEntry outputMode[] = {
        {OUTPUT_MODE_PCM, QT_TRANSLATE_NOOP("pcm","PCM")},
        {OUTPUT_MODE_LPCM, QT_TRANSLATE_NOOP("pcm","LPCM")}
    };
    diaElemMenu oMode(&cfg.output_mode, QT_TRANSLATE_NOOP("pcm","Output format:"), 2, outputMode);

    diaElem *elems[] = {&oMode};

    if(diaFactoryRun(QT_TRANSLATE_NOOP("pcm","PCM Configuration"), 1, elems))
    {
        if(*setup) delete *setup;
        *setup = NULL;
        ADM_paramSave(setup,pcm_encoder_param,&cfg);
        defaultConfig = cfg;
        return true;
    }
    return false;
}

// EOF

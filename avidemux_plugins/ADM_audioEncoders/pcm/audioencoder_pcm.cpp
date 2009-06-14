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

#include "audioencoder.h"
#include "audioencoderInternal.h"
#include "audioencoder_pcm.h"




/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_PCM);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  NULL,		//** put your own function here**
  "PCM",
  "PCM",
  "PCM encoder plugin Mean 2008",
  6,                    // Max channels
  1,0,0,                // Version
  WAV_PCM,
  200,                  // Priority
  NULL,  // Defined by macro automatically
  NULL,  // Defined by macro automatically

  NULL,           // Defined by macro automatically
  NULL,            // Defined by macro automatically

  NULL,         //** put your own function here**

  NULL
};
//ADM_DECLARE_AUDIO_ENCODER_CONFIG(NULL);
extern "C" ADM_audioEncoder *getInfo (void)
{
  return &encoderDesc;
}

/******************* / Declare plugin*******************************************************/



// Ctor: Duplicate
//__________

AUDMEncoder_PCM::AUDMEncoder_PCM(AUDMAudioFilter * instream)  :ADM_AudioEncoder    (instream)
{
  printf("[PCM] Creating PCM\n");
  wavheader.encoding=WAV_PCM;

};


AUDMEncoder_PCM::~AUDMEncoder_PCM()
{
  printf("[PCM] Deleting PCM\n");

};

/**
    \fn initialize
*/
bool AUDMEncoder_PCM::initialize(void)
{

  wavheader.byterate=wavheader.channels*wavheader.frequency*2;
  _chunk = (wavheader.frequency/100)*wavheader.channels*2;



  printf("[PCM]Incoming :fq : %"LU", channel : %"LU" \n",wavheader.frequency,wavheader.channels);
  printf("[PCM]PCM successfully initialized\n");
  return 1;
}
/**
    \fn getPacket
*/
bool         AUDMEncoder_PCM::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t nbout;

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
  if(1) //!revert)
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

// EOF

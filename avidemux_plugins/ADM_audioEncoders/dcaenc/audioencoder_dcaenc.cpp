/***************************************************************************
        \file audioencoder_dcaenc.cpp
        \brief Avidemux audio encoder plugin, front end for dcaenc
    copyright            : (C) 2011/2012 by mean
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

#include "ADM_default.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "audioencoder.h"
#include "audioencoderInternal.h"

extern "C"
{
#include <dcaenc.h>
}
#include "audioencoder_dcaenc.h"
#include "dcaencoder.h"
static dcaencoder config={300};
static uint32_t dcaencBitrate=300;

#define BLOCK_SIZE 1024

extern "C"
{
static bool configure (void);
static bool setOption(const char *paramName, uint32_t value);
};
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_DcaEnc);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "DcaEnc",
  "DTS (dcaenc)",
  "DCA encoder plugin Mean 2009",
  6,                    // Max channels
  1,0,0,                // Version
  WAV_DTS,              // WavTag
  200,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically

  setOption,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(NULL,NULL,dcaencBitrate);

/******************* / Declare plugin*******************************************************/
#define MYFLAGS (lame_global_flags *)lameFlags
/**
    \fn AUDMEncoder_DcaEnc Constructor
    \brief
*/
AUDMEncoder_DcaEnc::AUDMEncoder_DcaEnc (AUDMAudioFilter * instream,bool globalHeader):ADM_AudioEncoder  (instream)
{
  ADM_info ("[dcaenc] Creating lame\n");
  context = NULL;
  wavheader.encoding = WAV_DTS;
};

/**
    \fn AUDMEncoder_DcaEnc
    \brief Destructor
*/

AUDMEncoder_DcaEnc::~AUDMEncoder_DcaEnc ()
{

  ADM_info ("[dcaenc] Deleting ...\n");
  if (context)
    {
        dcaenc_destroy(context,NULL);
    }
  context = NULL;
};


/**
    \fn initialize
dcaenc    @return 1 on success, 0 on error
*/

bool AUDMEncoder_DcaEnc::initialize (void)
{
  int chan_config=0;
  switch(wavheader.channels)
  {
    case 1: chan_config=DCAENC_CHANNELS_MONO;break;
    case 2: chan_config=DCAENC_CHANNELS_STEREO;break;
    default:
         ADM_warning("Unsupported channel configuration \n");
         break;
  }
  context=dcaenc_create(wavheader.frequency,chan_config,config.bitrate*1000,DCAENC_FLAG_BIGENDIAN
                            );

  if(!context)
  {
      ADM_warning("Cannot create dcaenc context   \n");
      return false;
  }
  inputSize=dcaenc_input_size(context);
  outputSize=dcaenc_output_size(context);
  ADM_info("Converting %d samples to %d bytes\n",inputSize,outputSize);
  return true;
}
/**
    \fn isVBR
    @return 1 if the stream is vbr, 0 is cbr

*/
bool AUDMEncoder_DcaEnc::isVBR (void)
{
    return false;

}
/**
    \fn dither32
    \brief convert float to signed int32, format wanted by dcaenc
*/
static void dither32(float *s,int nb,int channels)
{
    float shift=(1LL<<32)-1;
    for(int i=0;i<nb;i++)
    {
        float f=s[i];
        f*=shift;
        s[i]=(int32_t )f;
    }
}
/**
    \fn send
    \brief Encode a block
*/
int AUDMEncoder_DcaEnc::send(uint32_t nbSample, uint8_t *dest)
{

  int nbout;
  dither32 (&(tmpbuffer[tmphead]), nbSample, wavheader.channels);
  ADM_assert (tmptail >= tmphead);
  int32_t *sample32=(int32_t *)& (tmpbuffer[tmphead]);
  int r=dcaenc_convert_s32(context,sample32,dest);
  if(r<0)
  {
    ADM_warning("Error while converting (%d)\n",r);
    return 0;
  }
  return outputSize;
}
/**
    \fn encode
    \brief Get an encoded dca packet
    @param dest [in] Where to write datas
    @param len  [out] Length of encoded datas in bytes
    @param samples [out] Number of samples
    @return true on success, false on error

*/
bool AUDMEncoder_DcaEnc::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{

  int32_t nbout;
  int neededSamples=inputSize*wavheader.channels;
  *samples = inputSize;	//FIXME
  *len = 0;
  if(AudioEncoderStopped==_state)
        return false;

    refillBuffer (neededSamples);
    if(AudioEncoderNoInput==_state)
    {
        int left=tmptail-tmphead;
        if (left < neededSamples)
        {
            if(left)
            {
                nbout=send(left,dest);
                tmphead=tmptail;
                ADM_info("[dcaenc]Sending last packet\n");
                goto cont;
            }
              // Flush
              _state=AudioEncoderStopped;
              // flush pad with 0n todo
              if(nbout<0)
              {
                    ADM_warning("Error while flushing dcaenc\n");
                    return false;
              }

              *len=nbout;
              *samples=inputSize;
              ADM_info("[dcaenc] Flushing, last block is %d bytes\n",nbout);
              return true;
    }
  }
  nbout=send(neededSamples,dest);
  tmphead += neededSamples;
cont:
  if (nbout < 0)
    {
      printf ("[dcaenc] Error !!! : %"PRIi32"\n", nbout);
      return false;
    }
  *len = nbout;
  if (!*len)
    *samples =0;
  else
    *samples=inputSize;
  return true;
}
/**
      \fn configure
      \brief Dialog to set lame settings
      @return 1 on success, 0 on failure

*/

bool configure (void)
{
  return true;
}
/**
     \fn setOption
     \brief Allow giving codec specific options as string+value
*/
bool setOption(const char *paramName, uint32_t value)
{
    return true;
}
// EOF

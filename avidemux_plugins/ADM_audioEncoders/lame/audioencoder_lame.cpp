/***************************************************************************
        \file audioencoder_lame.cpp
        \brief Avidemux audio encoder plugin, front end for libmp3lame
    copyright            : (C) 2006/2009 by mean
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
#include <lame/lame.h>
#include "audioencoder.h"
#include "audioencoderInternal.h"
#include "audioencoder_lame_param.h"
#include "audioencoder_lame.h"

static LAME_encoderParam myLameParam = {
  128,
  ADM_LAME_PRESET_CBR,		// preset;
  2,				//uint32_t        quality;
  0,				//uint32_t        disableReservoir; // usefull for strict CBR (FLV)
};
extern "C"
{
static bool configure (void);
static bool setOption(const char *paramName, uint32_t value);
};
/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Lame);

static ADM_audioEncoder encoderDesc = {
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,		//** put your own function here**
  "Lame",
  "MP3 (lame)",
  "Lame MP3 encoder plugin Mean 2009",
  2,                    // Max channels
  1,0,0,                // Version
  WAV_MP3,              // WavTag
  200,                  // Priority
  getConfigurationData,  // Defined by macro automatically
  setConfigurationData,  // Defined by macro automatically

  getBitrate,           // Defined by macro automatically
  setBitrate,            // Defined by macro automatically

  setOption,         //** put your own function here**

  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG(myLameParam);

/******************* / Declare plugin*******************************************************/
#define MYFLAGS (lame_global_flags *)lameFlags
/**
    \fn AUDMEncoder_Lame Constructor
    \brief
*/
AUDMEncoder_Lame::AUDMEncoder_Lame (AUDMAudioFilter * instream):ADM_AudioEncoder  (instream)
{
  printf ("[Lame] Creating lame\n");
  lameFlags = NULL;
  wavheader.encoding = WAV_MP3;
};

/**
    \fn AUDMEncoder_Lame
    \brief Destructor
*/

AUDMEncoder_Lame::~AUDMEncoder_Lame ()
{

  printf ("[Lame] Deleting lame\n");
  if (lameFlags)
    {
      lame_close (MYFLAGS);
    }
  lameFlags = NULL;
};


/**
    \fn initialize
    \brief initialize lame encoder
    @return 1 on success, 0 on error
*/

bool AUDMEncoder_Lame::initialize (void)
{

  int ret;
  MPEG_mode_e mmode;
  uint32_t frequence;
  LAME_encoderParam *lameConf = (LAME_encoderParam *) & myLameParam;


  lameFlags = lame_init ();
  if (lameFlags == NULL)
    return false;

  if (_incoming->getInfo ()->channels > 2)
    {
      printf ("[Lame]Too many channels\n");
      return false;
    }

  // recompute output length


  ret = lame_set_in_samplerate (MYFLAGS, wavheader.frequency);
  ret = lame_set_num_channels (MYFLAGS,  wavheader.channels);


  frequence = wavheader.frequency;
  printf ("[Lame] output frequency : %"LU"\n", frequence);
  ret = lame_set_out_samplerate (MYFLAGS, frequence);

  ret = lame_set_quality (MYFLAGS, 2);

  if (wavheader.channels == 2)
    {

          mmode = STEREO;
    }
  else
    {
      mmode = MONO;
      printf ("[Lame] mono audio mp3");
    }

  ret = lame_set_brate (MYFLAGS, lameConf->bitrate);
  ret = lame_set_mode (MYFLAGS, mmode);	// 0 stereo 1 jstero
  ret = lame_set_quality (MYFLAGS, lameConf->quality);	// 0 stereo 1 jstero
  ret = lame_set_disable_reservoir (MYFLAGS, lameConf->disableReservoir);
  printf ("[Lame]Using quality of %d\n", lame_get_quality (MYFLAGS));
  ret = lame_init_params (MYFLAGS);
  if (ret == -1)
    {
        printf("[Lame] Init params failes %d\n",ret);
        return false;
    }
  // update bitrate in header
  wavheader.byterate = (lameConf->bitrate >> 3) * 1000;
#define BLOCK_SIZE 1152
  // configure CBR/ABR/...

  switch (lameConf->preset)
    {
    default:
    case ADM_LAME_PRESET_CBR:
      break;
    case ADM_LAME_PRESET_ABR:

      lame_set_preset (MYFLAGS, lameConf->bitrate);
      wavheader.blockalign = BLOCK_SIZE;
      break;
    case ADM_LAME_PRESET_EXTREME:
      wavheader.blockalign = BLOCK_SIZE;
      lame_set_preset (MYFLAGS, EXTREME);
      break;


    }

  lame_print_config (MYFLAGS);
  lame_print_internals (MYFLAGS);
  _chunk = BLOCK_SIZE * wavheader.channels;

  return true;
}
/**
    \fn isVBR
    @return 1 if the stream is vbr, 0 is cbr

*/
bool AUDMEncoder_Lame::isVBR (void)
{
  if (myLameParam.preset == ADM_LAME_PRESET_CBR)
    return false;
  return true;

}
/**
    \fn encode
    \brief Get an encoded mp3 packet
    @param dest [in] Where to write datas
    @param len  [out] Length of encoded datas in bytes
    @param samples [out] Number of samples
    @return true on success, false on error

*/
bool AUDMEncoder_Lame::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{

  int32_t nbout;

  *samples = BLOCK_SIZE;	//FIXME
  *len = 0;

  if (!refillBuffer (_chunk))
    {
      return false;
    }

  if (tmptail - tmphead < _chunk)
    {
      return false;
    }
  dither16 (&(tmpbuffer[tmphead]), _chunk, wavheader.channels);
  ADM_assert (tmptail >= tmphead);
  if (wavheader.channels == 1)
    {
      nbout =	lame_encode_buffer (MYFLAGS, (int16_t *) & (tmpbuffer[tmphead]),
			    (int16_t *) & (tmpbuffer[tmphead]), _chunk, dest,
			    16 * 1024);

    }
  else
    {
      nbout =	lame_encode_buffer_interleaved (MYFLAGS,
					(int16_t *) & (tmpbuffer[tmphead]),
					_chunk / 2, dest, 16 * 1024);
    }
  tmphead += _chunk;
  if (nbout < 0)
    {
      printf ("[Lame] Error !!! : %"LD"\n", nbout);
      return false;
    }
  *len = nbout;
  if (!*len)
    *samples = 0;
  return true;
}
/**
      \fn configure
      \brief Dialog to set lame settings
      @return 1 on success, 0 on failure

*/
#define QT_TR_NOOP(x) x

bool configure (void)
{
  int ret = 0;
  char string[400];
  uint32_t mmode, ppreset;
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define PX(x) &(lameParam->x)

  LAME_encoderParam *lameParam = &myLameParam;
  ppreset = lameParam->preset;

  diaMenuEntry encodingMode[] = {
    {ADM_LAME_PRESET_CBR, QT_TR_NOOP ("CBR"), NULL},
    {ADM_LAME_PRESET_ABR, QT_TR_NOOP ("ABR"), NULL},
  };
  diaElemMenu Mode (&ppreset, QT_TR_NOOP ("Bit_rate mode:"),   SZT (encodingMode), encodingMode);

#define BITRATE(x) {x,QT_TR_NOOP(#x)}
  diaMenuEntry bitrateM[] = {
    BITRATE (56),
    BITRATE (64),
    BITRATE (80),
    BITRATE (96),
    BITRATE (112),
    BITRATE (128),
    BITRATE (160),
    BITRATE (192),
    BITRATE (224)
  };

//***
  diaElemMenu bitrate (PX(bitrate), QT_TR_NOOP ("_Bitrate:"), SZT (bitrateM),
		       bitrateM);
  diaElemUInteger quality (PX (quality), QT_TR_NOOP ("_Quality:"), 0, 9);
  diaElemToggle reservoir (PX (disableReservoir),
			   QT_TR_NOOP ("_Disable reservoir:"));

  diaElem *elems[] = { &Mode, &bitrate,&quality, &reservoir };

  if (diaFactoryRun (QT_TR_NOOP ("LAME Configuration"), 4, elems))
    {
      lameParam->preset=(ADM_LAME_PRESET)ppreset;
      return 1;
    }
  return 0;
}
/**
     \fn setOption
     \brief Allow giving codec specific options as string+value
*/
bool setOption(const char *paramName, uint32_t value)
{
    if(!strcasecmp(paramName,"MP3DisableReservoir"))
    {
        myLameParam.disableReservoir=value;
        return 1;
    }
    return 0;
}
// EOF

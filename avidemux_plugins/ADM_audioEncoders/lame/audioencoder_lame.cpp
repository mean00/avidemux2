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
#include "lame_encoder.h"
#include "lame_encoder_desc.cpp"

#define LAME_DEFAULT_CONF {128, ADM_LAME_PRESET_CBR, 2, 0}
static lame_encoder defaultConfig = LAME_DEFAULT_CONF;

extern "C"
{
static bool configure (CONFcouple **setup);
static bool setOption(CONFcouple **c, const char *paramName, uint32_t value);
static void getDefaultConfiguration(CONFcouple **c);
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

  setOption,         //** put your own function here**
  getDefaultConfiguration,
  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG();

/******************* / Declare plugin*******************************************************/
#define MYFLAGS (lame_global_flags *)lameFlags
/**
    \fn AUDMEncoder_Lame Constructor
    \brief
*/
AUDMEncoder_Lame::AUDMEncoder_Lame (AUDMAudioFilter * instream,bool globalHeader, 
    CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
{
  printf ("[Lame] Creating lame\n");
  lameFlags = NULL;
  wavheader.encoding = WAV_MP3;
  _config=defaultConfig;
  if(setup) // load config if possible
    ADM_paramLoad(setup,lame_encoder_param,&_config);
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

  ret = lame_set_brate (MYFLAGS, _config.bitrate);
  ret = lame_set_mode (MYFLAGS, mmode);	// 0 stereo 1 jstero
  ret = lame_set_quality (MYFLAGS, _config.quality);	// 0 stereo 1 jstero
  ret = lame_set_disable_reservoir (MYFLAGS, _config.disableBitReservoir);
  printf ("[Lame]Using quality of %d\n", lame_get_quality (MYFLAGS));
  // update bitrate in header
  wavheader.byterate = (_config.bitrate >> 3) * 1000;
#define BLOCK_SIZE 1152
  // configure CBR/ABR/...

  switch (_config.preset)
    {
    default:
    case ADM_LAME_PRESET_CBR:
        ADM_info("Lame : CBR Mode\n");
      break;
    case ADM_LAME_PRESET_ABR:
        ADM_info("Lame : ABR Mode\n");

      lame_set_preset (MYFLAGS, _config.bitrate);
      wavheader.blockalign = BLOCK_SIZE;
      break;
    case ADM_LAME_PRESET_EXTREME:
        ADM_info("Lame : Extreme Mode\n");
      wavheader.blockalign = BLOCK_SIZE;
      lame_set_preset (MYFLAGS, EXTREME);
      break;


    }
  ret = lame_init_params (MYFLAGS);
  if (ret == -1)
    {
        printf("[Lame] Init params failes %d\n",ret);
        return false;
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
  if (_config.preset == ADM_LAME_PRESET_CBR)
    return false;
  return true;

}
/**
    \fn send
    \brief Encode a block
*/
int AUDMEncoder_Lame::send(uint32_t nbSample, uint8_t *dest)
{
  int nbout;
  dither16 (&(tmpbuffer[tmphead]), nbSample, wavheader.channels);
  ADM_assert (tmptail >= tmphead);
  int16_t *sample16=(int16_t *)& (tmpbuffer[tmphead]);
  if (wavheader.channels == 1)
    {
      nbout =	lame_encode_buffer (MYFLAGS, 
                sample16,
			    sample16, nbSample, dest,
			    16 * 1024);

    }
  else
    {
      nbout =	lame_encode_buffer_interleaved (MYFLAGS,
					sample16,
					nbSample / 2, dest, 16 * 1024);
    }
    return nbout;
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
  if(AudioEncoderStopped==_state)
        return false;

    refillBuffer (_chunk);
    if(AudioEncoderNoInput==_state)
    {
        int left=tmptail-tmphead;
        if (left < _chunk)
        {
            if(left)
            {
                nbout=send(left,dest);
                tmphead=tmptail;
                ADM_info("[lame]Sending last packet\n");
                goto cont;
            }
              // Flush
              _state=AudioEncoderStopped;
              nbout=lame_encode_flush(MYFLAGS,dest,16*1024);
              if(nbout<0) 
              {
                    ADM_warning("Error while flushing lame\n");
                    return false;   
              }
                    
              *len=nbout;
              *samples=BLOCK_SIZE;  
              ADM_info("[Lame] Flushing, last block is %d bytes\n",nbout);
              return true;
    }
  }
  nbout=send(_chunk,dest);
  tmphead += _chunk;
cont:
  if (nbout < 0)
    {
      printf ("[Lame] Error !!! : %"LD"\n", nbout);
      return false;
    }
  *len = nbout;
  if (!*len)
    *samples = 0;
  else
    *samples=BLOCK_SIZE;
  return true;
}
/**
      \fn configure
      \brief Dialog to set lame settings
      @return 1 on success, 0 on failure

*/
#define QT_TR_NOOP(x) x

bool configure (CONFcouple **setup)
{
  int ret = 0;
  char string[400];
  uint32_t mmode, ppreset;
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define PX(x) &(config.x)

    lame_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,lame_encoder_param,&config);
    }

  ppreset = config.preset;

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
  diaElemMenu bitrate (&(config.bitrate), QT_TR_NOOP ("_Bitrate:"), SZT (bitrateM),
		       bitrateM);
  diaElemUInteger quality (PX (quality), QT_TR_NOOP ("_Quality:"), 0, 9);
  bool reservoir32=config.disableBitReservoir;
  diaElemToggle reservoir (&reservoir32,
			   QT_TR_NOOP ("_Disable reservoir:"));

  diaElem *elems[] = { &Mode, &bitrate,&quality, &reservoir };

  if (diaFactoryRun (QT_TR_NOOP ("LAME Configuration"), 4, elems))
    {
      config.preset=(ADM_LAME_PRESET)ppreset;
      config.disableBitReservoir=reservoir32;
      if(*setup) delete *setup;
      *setup=NULL;
      ADM_paramSave(setup,lame_encoder_param,&config);
      defaultConfig=config;
      return 1;
    }
  return 0;
}
/**
     \fn setOption
     \brief Allow giving codec specific options as string+value
*/
bool setOption(CONFcouple **c, const char *paramName, uint32_t value)
{
   lame_encoder config=defaultConfig;
    if(*c)
    {
        ADM_paramLoad(*c,lame_encoder_param,&config);
    }else
    {
        config=defaultConfig;
    }
    if(!strcasecmp(paramName,"MP3DisableReservoir"))
    {
        config.disableBitReservoir=value;
        ADM_paramSave(c,lame_encoder_param,&config);
        return 1;
    }
    ADM_error("Not supported\n");
    return 0;
}

void getDefaultConfiguration(CONFcouple **c)
{
	lame_encoder config = LAME_DEFAULT_CONF;

	ADM_paramSave(c, lame_encoder_param, &config);
}

// EOF

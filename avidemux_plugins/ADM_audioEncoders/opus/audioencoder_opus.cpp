/***************************************************************************
    copyright            : (C) 2017 mean
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
#include "DIA_coreToolkit.h"

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
/**
 * 
 * @param s
 */
static void OpusError(const char *s)
{
    GUI_Error_HIG("Opus",QT_TRANSLATE_NOOP("Opus",s));
}

/**
 * \fn ctor
 * @param instream
 * @param globalHeader
 * @param setup
 */

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
    printf("[Opus] Deleting encoder\n");

};


/**
    \fn initialize

*/
bool AUDMEncoder_Opus::initialize(void)
{
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
            OpusError("Unsupported frequency :\n   Only 8, 12, 16, 24 and 48 kHz are supported.");
            return false;            
    }
    //
    int  ratio=(wavheader.frequency+26)/50; // 20ms frames,  so we have ratio frames per sec
                                            // 48k => 960 ratio
    if(_config.bitrate*1000 < 3*ratio*8)
    {
        OpusError("Bitrate is too low for that frequency.");
        ADM_warning("Bitrate is too low (%d vs %d)\n",_config.bitrate*1000,ratio);
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
    int r=opus_encoder_ctl(_handle,OPUS_SET_BITRATE(_config.bitrate*1000));
    if(r<0)
    {
        ADM_warning("Failed to set bitrate to %d kbps\n",_config.bitrate);
        return false;
    }
    int br;
    opus_encoder_ctl(_handle,OPUS_GET_BITRATE(&br));
    ADM_info("Bitrate : Asked %d, actually set = %d\n",_config.bitrate,br);
    // update
    wavheader.byterate=(_config.bitrate*1000)/8;
    wavheader.blockalign=1;
    wavheader.bitspersample=0;
    wavheader.encoding=WAV_OPUS;
    // create header
    int initialPadding=0;
    if(OPUS_OK != opus_encoder_ctl(_handle,OPUS_GET_LOOKAHEAD(&initialPadding)))
        ADM_warning("[Opus] Cannot get number of lookahead samples.\n");
    _extraSize=19; // max 2 channels
    _extraData=new uint8_t[_extraSize];
    uint8_t *p=_extraData;
    memset(p,0,_extraSize);
    const char *head="OpusHead";
    memcpy(p,head,8);
    p+=8;
#define PUT_BYTE(v) *p=v&0xFF; p++;
    PUT_BYTE(1) // version
    PUT_BYTE(channels)

    uint16_t padding=(uint32_t)initialPadding&0xFFFF;
    PUT_BYTE(padding)
    PUT_BYTE(padding>>8)
    PUT_BYTE(wavheader.frequency)
    PUT_BYTE(wavheader.frequency>>8)
    PUT_BYTE(wavheader.frequency>>16)
    PUT_BYTE(wavheader.frequency>>24)

    return true;
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
 opus_int32 done;
 
 int processedSamples=(20*wavheader.frequency)/1000; // xx ms worth of data
 int sixty=processedSamples*channels;
 
    while(1)
    {
        count++;
        if(!refillBuffer(sixty ))
        {
          return false; 
        }
        ADM_assert(tmptail>=tmphead);
        
        
#if 1        
        done=opus_encode_float(_handle,&(tmpbuffer[tmphead]),
                                processedSamples,
                               dest,
                               4*4000);        
#else
        dither16 (&(tmpbuffer[tmphead]), processedSamples, channels);
        done=opus_encode(_handle,(opus_int16 *)&(tmpbuffer[tmphead]),processedSamples,
                               dest,
                               4000);        
#endif        
        tmphead+=sixty;  
        //printf("processed sample = %d, output = %d\n",processedSamples,done);        
        if(done<=0) 
        {
          if(count<20)
              continue;
          return false;
        }
        break;
    }
    *len=done;    
    *samples=processedSamples;
    return true;
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
                            BITRATE(24),
                            BITRATE(32),
                            BITRATE(48),
                            BITRATE(56),
                            BITRATE(64),
                            BITRATE(80),
                            BITRATE(96),
                            BITRATE(112),
                            BITRATE(128)
                              
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
/**
 * \fn    getDefaultConfiguration
 * @param c
 */
void getDefaultConfiguration(CONFcouple **c)
{
	opus_encoder config = OPUS_DEFAULT_CONF;
	ADM_paramSave(c, opus_encoder_param, &config);
}
// EOF

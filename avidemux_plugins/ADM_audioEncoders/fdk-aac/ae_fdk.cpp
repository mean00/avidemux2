
/***************************************************************************
 * \file ae_fdk
 * \brief interface to franhaufer AAC library    
 * \author mean fixounet@free.fr 2006-2016

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



#define FDKAAC_DEFAULT_CONF {128,true,AOT_AAC_LC,false}

static fdk_encoder defaultConfig = FDKAAC_DEFAULT_CONF;

static bool configure(CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);

#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

/********************* Declare Plugin *****************************************************/
ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(AUDMEncoder_Fdkaac);

static ADM_audioEncoder encoderDesc = { 
  ADM_AUDIO_ENCODER_API_VERSION,
  create,			// Defined by macro automatically
  destroy,			// Defined by macro automatically
  configure,    		//** put your own function here**
  "FDK_AAC",            
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

/**
 * \fn ctor
 * @param instream
 * @param globalHeader
 * @param setup
 */
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
        break;
  }
  wavheader.encoding=WAV_AAC;
  wavheader.blockalign=4096;
  wavheader.bitspersample=0;
  
  _config=defaultConfig;
  
  // Pre setup the input/output parameters
  
 
    inAudioId=IN_AUDIO_DATA;
    inAudioSize=0;
    inAudioPtr=NULL;
 
    outAudioId=OUT_BITSTREAM_DATA;
    outAudioSize=0;
    outAudioPtr=NULL;
    
    inDesc.numBufs=1;
    inDesc.bufferIdentifiers=&inAudioId;
    inDesc.bufSizes=&inAudioSize;
    inDesc.bufs=&inAudioPtr;
    inDesc.bufElSizes=&inElemSize;
    
    
    outDesc.numBufs=1;
    outDesc.bufferIdentifiers=&outAudioId;
    outDesc.bufSizes=&outAudioSize;
    outDesc.bufs=&outAudioPtr;
    outDesc.bufElSizes=&outElemSize;
  
  
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
    ADM_info("[FDKAAC] Deleting faac\n");

};
/**
 * 
 * @param name
 * @param nameAsInt
 * @param value
 * @return 
 */

bool AUDMEncoder_Fdkaac::setParam(const char *name, int nameAsInt, int value)
{
   int error=   aacEncoder_SetParam(_aacHandle,(AACENC_PARAM)nameAsInt,value); 
    if(error!=AACENC_OK) 
    {
        ADM_warning("%s failed\n",name); 
        return false;
    }
    ADM_info("Parameter %s (%d)= %d\n",name,nameAsInt,(int)value);
    return true;
}
/**
 *  \fn dumpConfiguration
 * @return 
 */
bool AUDMEncoder_Fdkaac::dumpConfiguration()
{
    #define GET_PARAM(name) ADM_info(#name" = %d\n",(int)aacEncoder_GetParam(_aacHandle,name));
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
    return true;
}
/**
    \fn initialize

*/
bool AUDMEncoder_Fdkaac::initialize(void)
{
int ret=0;
int channels=wavheader.channels;

    ADM_info("[FDKAAC] Incoming Fq :%u\n",wavheader.frequency);
    AACENC_ERROR error;
    error= aacEncOpen(&_aacHandle,0,wavheader.channels);
    if(error!=AACENC_OK)
    {
        ADM_warning("Cannot open fdk AAC for channels=%d\n",channels);
        return 0;
    }
    //
    
    // Mandatory settings
#define SET_PARAM(name,value) if(!setParam(#name,name,value)) ADM_warning("oops\n");
    
    
    CHANNEL_MODE mode;
    switch(wavheader.channels)
    {
#define MKCHAN(x,y,z,t)         case x: mode=y;break;
        MKCHAN(1,MODE_1,        1,0)
        MKCHAN(2,MODE_2,        0,1)
        MKCHAN(3,MODE_1_2,      1,1) // L+R+Center
        MKCHAN(4,MODE_1_2_1,    2,1) // L+R+Center+Rear
        MKCHAN(5,MODE_1_2_2,    1,2) // L+R+Center + RearLeft+RearRight
        MKCHAN(6,MODE_1_2_2_1,  2,2) // Same +LFE
                
        default:
            ADM_warning("Improper channel configuration (%d)\n",wavheader.channels);
            return false;
            break;
    }
    SET_PARAM(AACENC_AOT, _config.profile) // Mpeg4 LC
    SET_PARAM(AACENC_TRANSMUX,TT_MP4_RAW) // Raw binary         
    SET_PARAM(AACENC_BITRATEMODE,0) // CBR
    SET_PARAM(AACENC_BITRATE,_config.bitrate*1000)
    SET_PARAM(AACENC_SAMPLERATE,(wavheader.frequency))    
    SET_PARAM(AACENC_AFTERBURNER,_config.afterburner);
    SET_PARAM(AACENC_CHANNELMODE,mode)            
    SET_PARAM(AACENC_SBR_MODE,_config.sbr)            
    
    //
    // make a dry run of the encoder so that we have extradata
    //
    error= aacEncEncode(_aacHandle, NULL, NULL, NULL, NULL);
    if(error!= AACENC_OK) 
    {
        ADM_warning("Cannot setup fdk encoder (%d)\n",error);
        return false;
    }

    // Read back

    ADM_info("Read back parameters:\n");
    dumpConfiguration();
        
    // Get specific configuration
    AACENC_InfoStruct        pInfo;
    memset(&pInfo,0,sizeof(pInfo));
    if(AACENC_OK != aacEncInfo(_aacHandle,&pInfo))
    {
        ADM_warning("Cannot get info\n");
        return false;
    }
    
     _extraSize=pInfo.confSize;
     _extraData=new uint8_t[1+_extraSize];
     memcpy(_extraData,pInfo.confBuf,_extraSize);

    // update
    wavheader.byterate=(_config.bitrate*1000)/8;

    // How many samples do we need to output one AAC packet ?
    _chunk=pInfo.frameLength*wavheader.channels;

    ordered=new float[_chunk*2]; // keep a big margin in case of incomplete processing of the previous one
    ADM_info("[Fdk] Initialized with %d bytes of extra data, framelength=%d\n",_extraSize,pInfo.frameLength);
    _running=true;
    return true;
}
/**
 * \fn AUDMEncoder_Fdkaac::availableSamplePerChannel
 * @return 
 */
int AUDMEncoder_Fdkaac::availableSamplesPerChannel(void)
{
    int incomingSamplesPerChannel=(tmptail-tmphead)/wavheader.channels;

    if(incomingSamplesPerChannel>_chunk/wavheader.channels)
            incomingSamplesPerChannel=(_chunk/wavheader.channels);   
    return incomingSamplesPerChannel;
}
/**
    \fn encode
*/
bool	AUDMEncoder_Fdkaac::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
 uint32_t count=0;
 int channels=wavheader.channels;
  
 AACENC_InArgs inArgs={0};
 AACENC_OutArgs outArgs={0};
    aprintf("FDK => Encode\n");
 
    inAudioId=IN_AUDIO_DATA;
    outAudioId=OUT_BITSTREAM_DATA;
   
    inDesc.numBufs=1;
    outDesc.numBufs=1;
    
    AACENC_InfoStruct pInfo;
    
    *samples =0;
    *len = 0;
    
_again:
        if(!_running)
          return false;
        int incomingSamplesPerChannel=availableSamplesPerChannel();

        aacEncInfo(_aacHandle,&pInfo);
        
        // We dont have enough data
        if((pInfo.inBufFillLevel+incomingSamplesPerChannel)<_chunk/channels)
          {
            aprintf("Refill\n");
            if(!refillBuffer(_chunk ))
            {
                    ADM_info("Flush\n");
                    inDesc.numBufs=-1;
                    inAudioSize=0;
                    inArgs.numInSamples=0;
                    _running=false;
                    int error=aacEncEncode(_aacHandle,&inDesc,&outDesc,&inArgs,&outArgs);
                    if(error!=AACENC_OK)
                    {
                        ADM_warning("[FDK] Flushing error %d\n",error);
                        return false;
                    }
                    
                    *len=outArgs.numOutBytes;
                    *samples=outArgs.numInSamples/channels;
                    ADM_info("Flushing last packet =%d bytes\n",*len);
                    return !!(*len);
            }
            ADM_assert(tmptail>=tmphead);            
            goto _again;
          }
        
        aprintf("State : inBufFillLevel=%d frameLength per channel=%d incomingSamplesPerChannel=%d chunk=%d\n",pInfo.inBufFillLevel,pInfo.frameLength,incomingSamplesPerChannel,_chunk/channels);                
        
       // Try to encode with whatever is in the buffer
        
        reorder(&(tmpbuffer[tmphead]),ordered,incomingSamplesPerChannel,_incoming->getChannelMapping(),outputChannelMapping);
        dither16(ordered,incomingSamplesPerChannel*channels,channels);
        tmphead+=incomingSamplesPerChannel*channels; 
         
        inAudioSize=2*incomingSamplesPerChannel*channels;
        inAudioPtr=ordered;
        inElemSize=2;
        
        outAudioPtr=dest;
        outAudioSize= 768*channels; // ?
        outElemSize=1;
        
        inArgs.numInSamples=inAudioSize>>1; // in sample
        
        int error=aacEncEncode(_aacHandle,&inDesc,&outDesc,&inArgs,&outArgs);
        if(error!=AACENC_OK)
        {
            ADM_warning("[FDK] Encoding error %d\n",error);
            return false;
        }
       
        if(!outArgs.numOutBytes)
          {
            aprintf("[FDK] No output...\n");                     
            if(!refillBuffer(_chunk ))
            {
                return 0; 
            }
            ADM_assert(tmptail>=tmphead);            
            goto _again;
          }
        *len=outArgs.numOutBytes;
        *samples=outArgs.numInSamples/channels;
        aprintf("[FDK]Generated %d bytes, consumed %d samples\n",*len,*samples);
        
        return 1;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TRANSLATE_NOOP("FDK-AAC",#x)}

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
                              BITRATE(384),
                              BITRATE(448),
                              BITRATE(576),
                              BITRATE(640)
                          };
    
#define PROFILE(x,y) {y,QT_TRANSLATE_NOOP("FDK-AAC",x)}    
     diaMenuEntry profileM[]={
         PROFILE("LC",AOT_AAC_LC),
         PROFILE("HE-AAC",AOT_AAC_LC),
         PROFILE("HE-AACv2",AOT_AAC_LC),
     };
    uint32_t prof=0;
#if 0
    diaElemMenu profile(&prof,   QT_TRANSLATE_NOOP("FDK-AAC","_Profile::"), SZT(profileM),profileM);
    diaElemMenu bitrate(&(config.bitrate),   QT_TRANSLATE_NOOP("FDK-AAC","_Bitrate:"), SZT(bitrateM),bitrateM);
  
    diaElemToggle sbr(&(config.sbr),QT_TRANSLATE_NOOP("FDK-AAC","SBR enabled"));
    diaElemToggle afterburner(&(config.afterburner),QT_TRANSLATE_NOOP("FDK-AAC","After burner"));

    diaElem *elems[]={&profile,&bitrate,&sbr,&afterburner};
    
    if ( diaFactoryRun(QT_TRANSLATE_NOOP("FDK-AAC","FDK-AAC Configuration"),4,elems))
#else
     diaElemMenu bitrate(&(config.bitrate),   QT_TRANSLATE_NOOP("FDK-AAC","_Bitrate:"), SZT(bitrateM),bitrateM);
     diaElemToggle afterburner(&(config.afterburner),QT_TRANSLATE_NOOP("FDK-AAC","After burner"));
     diaElem *elems[]={&bitrate,&afterburner};
     if ( diaFactoryRun(QT_TRANSLATE_NOOP("FDK-AAC","FDK-AAC Configuration"),2,elems))
#endif        
    {
        if(*setup) delete *setup;
        *setup=NULL;
        ADM_paramSave(setup,fdk_encoder_param,&config);
        defaultConfig=config;
        return true;
    }
    return false;
}
/**
 * 
 * @param c
 */
void getDefaultConfiguration(CONFcouple **c)
{
	fdk_encoder config = FDKAAC_DEFAULT_CONF;

	ADM_paramSave(c, fdk_encoder_param, &config);
}
// EOF

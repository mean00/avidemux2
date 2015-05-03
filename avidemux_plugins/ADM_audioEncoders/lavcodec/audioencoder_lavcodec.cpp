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
    #include "libavutil/channel_layout.h"
#include "libavutil/error.h"
}

#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "audioencoder.h"
#include "audioencoderInternal.h"

#include "audioencoder_lavcodec.h"
#include "lavcodec_encoder_desc.cpp"

#define LAV_DEFAULT_CONF {128}
static lav_encoder  defaultConfig = LAV_DEFAULT_CONF;

static bool         configure (CONFcouple **setup);
static void getDefaultConfiguration(CONFcouple **c);
#if 0
#define cprintf printf
#else
#define cprintf(...) {}
#endif
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
  NULL,         //** put your own function here**
  getDefaultConfiguration,
  NULL
};
ADM_DECLARE_AUDIO_ENCODER_CONFIG();

/******************* / Declare plugin*******************************************************/
#define CONTEXT ((AVCodecContext  	*)_context)


/**
    \fn AUDMEncoder_Lavcodec
*/
AUDMEncoder_Lavcodec::AUDMEncoder_Lavcodec(AUDMAudioFilter * instream,bool globalHeader,
        CONFcouple *setup)  :ADM_AudioEncoder    (instream,setup)
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
 // Default config
  _config=defaultConfig;
  if(setup) // load config if possible
        ADM_paramLoad(setup,lav_encoder_param,&_config);
  planarBuffer=NULL;
  planarBufferSize=0;
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
  if(_frame)   av_frame_free(&_frame);
  _frame=NULL;
  if(planarBuffer) delete [] planarBuffer;
  planarBuffer=NULL;;
};

/**
    \fn initialize
*/
bool AUDMEncoder_Lavcodec::initialize(void)
{
  int ret;
  

  if( _incoming->getInfo()->channels>ADM_LAV_MAX_CHANNEL)
  {
    ADM_error("[Lavcodec]Too many channels\n");
    return 0;
  }
  AVCodec *codec;
  AVCodecID codecID;
  codecID=makeName(CODEC_ID);
  codec = avcodec_find_encoder(codecID);
  ADM_assert(codec);
  _context=( void *)avcodec_alloc_context3(codec);
  _frame=av_frame_alloc();
  
  wavheader.byterate=(_config.bitrate*1000)>>3;

  _chunk = ADM_LAV_SAMPLE_PER_P*wavheader.channels; // AC3
  planarBuffer=new float[_chunk];
  planarBufferSize=_chunk;
  ADM_info("[Lavcodec]Incoming : fq : %"PRIu32", channel : %"PRIu32" bitrate: %"PRIu32" \n",
  wavheader.frequency,wavheader.channels,_config.bitrate);

    if(wavheader.channels>2) 
    {
        ADM_warning("Channel remapping activated\n");
        needChannelRemapping=true;
    }
    else needChannelRemapping=false;
  
  CONTEXT->channels     =  wavheader.channels;
  CONTEXT->sample_rate  =  wavheader.frequency;
  CONTEXT->bit_rate     = (_config.bitrate*1000); // bits -> kbits
  CONTEXT->sample_fmt   =  AV_SAMPLE_FMT_FLT;
  CONTEXT->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
  CONTEXT->frame_size=_chunk/wavheader.channels;
  CONTEXT->channel_layout=av_get_default_channel_layout(wavheader.channels);
  
  if(true==_globalHeader)
  {
    ADM_info("Configuring audio codec to use global headers\n");
    CONTEXT->flags|=CODEC_FLAG_GLOBAL_HEADER;
  }
  
    computeChannelLayout();
    CONTEXT->sample_fmt   =  AV_SAMPLE_FMT_FLTP;
    ret = avcodec_open2(CONTEXT, codec,NULL);
    if (ret<0)
    {            
                CONTEXT->sample_fmt=AV_SAMPLE_FMT_S16;
                ret = avcodec_open2(CONTEXT, codec,NULL);
                if (ret<0)
                {
                        printError("Init failed",ret);
                        return 0;
                }
     
   }
    //ADM_info("Frame size : %d, %d\n",CONTEXT->frame_size,_chunk/wavheader.channels);
    _frame->format=CONTEXT->sample_fmt;    
    outputFlavor=asFloatPlanar;
    ADM_info("[Lavcodec]Lavcodec successfully initialized,wavTag : 0x%x\n",makeName(WAV));
    return 1;
}

/**
 * \fn printError
 * @param s : banner
 * @param er : error code
 */
void AUDMEncoder_Lavcodec::printError(const char *s,int er)
{
    char strer[256]={0};
                av_strerror(er, strer, sizeof(strer));
                ADM_error("[Lavcodec] %s,err : %d %s!\n",s,er,strer);
                
}
bool	AUDMEncoder_Lavcodec::encodeBlock(int count, uint8_t *dest,int &encoded)
{
    if(wavheader.channels>2) return encodeBlockMultiChannels(count,dest,encoded);
    else return encodeBlockSimple(count,dest,encoded);
}
/**
 * \fn lastBlock
 * \brief purge internal data if any
 * @param encoded
 * @return 
 */
bool AUDMEncoder_Lavcodec::lastBlock(AVPacket *pkt,int &encoded)
{
    int gotPacket;
    int nbout=avcodec_encode_audio2(CONTEXT, pkt,NULL,&gotPacket);
         if(nbout<0)
         {
             printError("Encoding lastBlock",nbout);
             return false;
         }
         if(gotPacket)
               encoded=pkt->size;
         return true;
}

/**
 * \fn encodeBlockMultiChannel
 * \brief encode a block when # of channels is > 2
 * @param count
 * @return 
 */
bool	AUDMEncoder_Lavcodec::encodeBlockMultiChannels(int count, uint8_t *dest,int &encoded)
{
     encoded=0;
     AVPacket pkt; // out
     av_init_packet(&pkt);
     pkt.size=5000;
     pkt.data=dest;

     
      if(!count)
      {
          return lastBlock(&pkt,encoded);
      }
     
     int gotPacket;
     int channel=wavheader.channels;
     int nbBlocks=count/channel;
     _frame->channels=wavheader.channels;
     _frame->channel_layout=CONTEXT->channel_layout;     
     _frame->nb_samples=nbBlocks;

        CHANNEL_TYPE *f=_incoming->getChannelMapping();
        CHANNEL_TYPE *o=channelMapping;
     
      
        
        int er;
        if( CONTEXT->sample_fmt   ==  AV_SAMPLE_FMT_FLTP)
        {
                    // reorder and de-interleave
                reorderToPlanar(&(tmpbuffer[tmphead]),planarBuffer,nbBlocks,f,o);
                er=avcodec_fill_audio_frame(_frame, channel,
                              AV_SAMPLE_FMT_FLTP, (uint8_t *)planarBuffer,
                               count*sizeof(float), 0);
        }else
        {
              dither16(&(tmpbuffer[tmphead]),count,channel);
              er=avcodec_fill_audio_frame(_frame, channel,
                              AV_SAMPLE_FMT_S16, (uint8_t *)&(tmpbuffer[tmphead]),
                               count*sizeof(uint16_t), 0);
        }
        if(er<0)
        {
            printError("Fill audio",er);
            return false;
        }
         
    int  nbout = avcodec_encode_audio2(CONTEXT, &pkt,_frame,&gotPacket);
    if(nbout>=0 && gotPacket)
    {
        cprintf("Got %d bytes \n",pkt.size);
        encoded=pkt.size;
    }
    else
    {
        printError("Encoding",nbout);
        return false;
    }
    return true;
}


/**
 * \fn i2p
 * \brief convert interleaved float to planar float
 * @param count
 * @return 
 */
float * AUDMEncoder_Lavcodec::i2p(int count)
{
    
    int nbBlock=count/wavheader.channels;
    if(nbBlock*wavheader.channels!=count)
          ADM_warning("Bloc does not match : count=%d, channels=%d\n",count,wavheader.channels);
    float *ss=&(tmpbuffer[tmphead]);
    if(wavheader.channels==1) 
                return ss;

   
    float *d=planarBuffer;
    for(int c=0;c<wavheader.channels;c++)
    {
        float *s=ss+c;
        for( int block=0;block<nbBlock;block++)
        {
            *d++=*s;
            s+=wavheader.channels;
        }
    }
    return planarBuffer;
 }

/**
 * \fn encodeBlockSimple
 * \brief mono or stereo
 */
bool	AUDMEncoder_Lavcodec::encodeBlockSimple(int count, uint8_t *dest,int &encoded)
{
     encoded=0;
     AVPacket pkt; // out
     av_init_packet(&pkt);
     pkt.size=5000;
     pkt.data=dest;     
     
      if(!count)
      {
          return lastBlock(&pkt,encoded);
      }
     int gotPacket;
     int channel=wavheader.channels;
     _frame->channel_layout=CONTEXT->channel_layout;

     int nbBlocks=count/channel;
      _frame->nb_samples=nbBlocks;
      
      
      int er;
      if( CONTEXT->sample_fmt   ==  AV_SAMPLE_FMT_FLTP)
        {
                float *in=i2p(count);
                // reorder and de-interleave
                er=avcodec_fill_audio_frame(_frame, channel,
                              AV_SAMPLE_FMT_FLTP, (uint8_t *)in,
                               count*sizeof(float), 0);
        }else
        { // s16
              dither16(&(tmpbuffer[tmphead]),count,channel);
              er=avcodec_fill_audio_frame(_frame, channel,
                              AV_SAMPLE_FMT_S16, (uint8_t *)&(tmpbuffer[tmphead]),
                               count*sizeof(uint16_t), 0);
        }
     if(er<0)
     {
        printError("Fill audio",er);
        return false;
     }
     
     
    int  nbout = avcodec_encode_audio2(CONTEXT, &pkt,_frame,&gotPacket);
    if(nbout>=0 && gotPacket)
    {
        cprintf("Got %d bytes \n",pkt.size);
        encoded=pkt.size;
    }
    else
    {
        printError("Encoding",nbout);
        return false;
    }
    return true;
}
/**
 * \fn computeChannelLayout
 * @return 
 */
bool AUDMEncoder_Lavcodec::computeChannelLayout(void)
{
#define CHANMIX(x,y) case AV_CH_##y: *o=ADM_CH_##x;cprintf(" =>%s\n",ADM_printChannel(*o));o++;break;
    
        int channels=wavheader.channels;
        CHANNEL_TYPE *o=channelMapping;
        for(int i=0;i<channels;i++)
        {
          uint64_t chan=av_channel_layout_extract_channel(CONTEXT->channel_layout,i);
          cprintf("Channel %s ",av_get_channel_name(chan))  ;
          switch(chan)
          {
              CHANMIX(FRONT_LEFT,FRONT_LEFT)
              CHANMIX(FRONT_RIGHT,FRONT_RIGHT)
              CHANMIX(LFE,LOW_FREQUENCY)
              CHANMIX(FRONT_CENTER,FRONT_CENTER)
              CHANMIX(REAR_LEFT,BACK_LEFT)
              CHANMIX(REAR_RIGHT,BACK_RIGHT)
                default:
                    ADM_warning("Channel no mapped : %s\n");
                    *o++=ADM_CH_FRONT_LEFT;
                    break;
          }
        }
        return true;
}
/**
    \fn encode
*/
bool	AUDMEncoder_Lavcodec::encode(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
  uint32_t nbout;
  int retries=16;
  bool r;
  int sz;
again:
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
                
               encodeBlock(left,dest,sz);
               *samples = left/channels;
               *len=sz;
               ADM_info("[Lav] Last audio block\n");
               goto cnt;
            }
              // Flush
               ADM_info("[Lav] Flush\n");
              _state=AudioEncoderStopped;
              if(CONTEXT->codec->capabilities & CODEC_CAP_DELAY)
              {
                  if(false==encodeBlock(0,dest,sz))
                  {
                        ADM_warning("Error while flushing lame\n");
                        return false;                      
                  }
                  *len=sz;
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
   
   r=encodeBlock(_chunk,dest,sz);
   tmphead+=_chunk;
cnt:
  if(!r && retries)
  {
    retries--;
    ADM_info("Audio encoder (lav): no packet, retrying\n");
    goto again;
  }
  *len=sz;
  *samples=_chunk/channels;
  return true;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TR_NOOP(#x)}

/**
    \fn configure
*/
bool configure (CONFcouple **setup)
{
 int ret=0;
    lav_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,lav_encoder_param,&config);
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
                              BITRATE(448)
                          };
    diaElemMenu bitrate(&(config.bitrate),   QT_TR_NOOP("_Bitrate:"), SZT(bitrateM),bitrateM);



    diaElem *elems[]={&bitrate};

    if ( diaFactoryRun(QT_TR_NOOP(ADM_LAV_MENU" (lav) Configuration"),1,elems))
    {
        if(*setup) delete *setup;
        *setup=NULL;
        ADM_paramSave(setup,lav_encoder_param,&config);
        defaultConfig=config;
        return true;
    }
    return false;
}

void getDefaultConfiguration(CONFcouple **c)
{
	lav_encoder config = LAV_DEFAULT_CONF;

	ADM_paramSave(c, lav_encoder_param, &config);
}
// EOF
     

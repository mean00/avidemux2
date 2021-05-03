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
  _frame=NULL;
  _pkt = NULL;
  _globalHeader=globalHeader;
   ADM_info("[Lavcodec] Creating Lavcodec audio encoder (0x%x)\n",makeName(WAV));
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
  outputFlavor = unsupported;
  encoderState = normal;
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
  ADM_info("[Lavcodec] Deleting Lavcodec\n");
  if(_pkt)
        av_packet_free(&_pkt);
  if(_context)
  {
    avcodec_close(CONTEXT);
    av_free(_context);
  }
  _context=NULL;
  if(_frame)   
  {
      av_frame_free(&_frame);
  }
  _frame=NULL;
  if(planarBuffer) 
  {
      delete [] planarBuffer;
  }
  planarBuffer=NULL;;
};

/**
    \fn initialize
*/
bool AUDMEncoder_Lavcodec::initialize(void)
{
    if(_incoming->getInfo()->channels > ADM_LAV_MAX_CHANNEL) // TODO: query codec
    {
        ADM_error("[Lavcodec] Too many channels\n");
        return false;
    }
    AVCodec *codec = NULL;
    AVCodecID codecID = avMakeName;
    // Try to find an encoder
    codec = avcodec_find_encoder(codecID);
    if(!codec)
    {
#define STR(x) #x
#define MKSTRING(x) STR(x)
        ADM_error("[Lavcodec] Cannot find encoder for %s\n",MKSTRING(avMakeName));
        return false;
    }
    // Does the encoder support a sample format we do?
    const AVSampleFormat *sfmt = codec->sample_fmts;
    while(*sfmt != AV_SAMPLE_FMT_NONE)
    {
#define MATCH(x,y) if(*sfmt == AV_SAMPLE_FMT_ ##x) { outputFlavor = as ##y; break; }
        MATCH(FLTP,FloatPlanar)
        MATCH(FLT,Float)
        MATCH(S16,Int16)
        sfmt++;
    }
    if(outputFlavor == unsupported)
    {
        ADM_error("[Lavcodec] The encoder doesn't support any of sample formats we can offer.\n");
        return false;
    }
    ADM_info("[Lavcodec] Selected %s as sample format.\n", av_get_sample_fmt_name(*sfmt));
    // Allocate and fill codec context
    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    if(!ctx)
    {
        ADM_error("[Lavcodec] Cannot allocate context.\n");
        return false;
    }
    ctx->channels = wavheader.channels;
    ctx->channel_layout = av_get_default_channel_layout(wavheader.channels);
    ctx->sample_rate = wavheader.frequency;
    ctx->sample_fmt = *sfmt;
    ctx->frame_size = ADM_LAV_SAMPLE_PER_P;
    ctx->bit_rate = _config.bitrate * 1000; // bits -> kbits
    ctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if(_globalHeader)
    {
        ADM_info("Configuring audio codec to use global headers\n");
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    _context = (void *)ctx;

    _chunk = ADM_LAV_SAMPLE_PER_P * wavheader.channels; // AC3

    // Can we use the encoder?
    int ret = avcodec_open2(ctx,codec,NULL);
    if (ret<0)
    {
         printError("Init failed",ret);
         return false;
    }

    wavheader.byterate=(_config.bitrate*1000)>>3;
    computeChannelLayout();

    // Allocate input frame
    _frame = av_frame_alloc();
    if(!_frame)
    {
        ADM_error("[Lavcodec] Cannot allocate frame.\n");
        return false;
    }

    _frame->nb_samples = ctx->frame_size;
    _frame->format = ctx->sample_fmt;
    _frame->channel_layout = ctx->channel_layout;

    ret = av_frame_get_buffer(_frame, 0);
    if(ret < 0)
    {
        printError("av_frame_get_buffer",ret);
        return false;
    }

    if(outputFlavor != asInt16)
    {
        planarBuffer = new float[_chunk];
        planarBufferSize = _chunk;
    }
    ADM_info("[Lavcodec]Incoming : fq : %" PRIu32", channel : %" PRIu32" bitrate: %" PRIu32" \n",
                wavheader.frequency,wavheader.channels,_config.bitrate);

    // Allocate packet to hold encoder output
    _pkt = av_packet_alloc();
    if(!_pkt)
    {
        ADM_error("Cannot allocate AVPacket.\n");
        return false;
    }
    ADM_info("[Lavcodec]Lavcodec successfully initialized,wavTag : 0x%x\n",makeName(WAV));
    return true;
}

/**
 * \fn printError
 * @param s : banner
 * @param er : error code
 */
void AUDMEncoder_Lavcodec::printError(const char *s,int er)
{
    char strer[AV_ERROR_MAX_STRING_SIZE]={0};
    av_make_error_string(strer, AV_ERROR_MAX_STRING_SIZE, er);
    ADM_error("[Lavcodec] %s error %d (\"%s\")\n",s,er,strer);
}

/**
 * \fn fillFrame
 * \brief Prepare input for the encoder.
 * @param  int  Size of input in floats, should be equal _chunk
 * @return bool True on success, else false.
 */
bool AUDMEncoder_Lavcodec::fillFrame(int count)
{
    int er, bufSize, channels = wavheader.channels;
    float *fptr = &(tmpbuffer[tmphead]);
    uint8_t *buffer;
    AVSampleFormat format;
    switch(outputFlavor)
    {
        case asInt16:
            dither16(fptr,count,channels);
            buffer = (uint8_t *)fptr;
            bufSize = count * sizeof(uint16_t);
            format = AV_SAMPLE_FMT_S16;
            break;
        case asFloat:
            buffer = (uint8_t *)fptr;
            bufSize = count * sizeof(float);
            format = AV_SAMPLE_FMT_FLT;
            break;
        case asFloatPlanar:
            if(channels > 2)
            {
                reorderToPlanar(fptr, planarBuffer, _frame->nb_samples, _incoming->getChannelMapping(), channelMapping);
                buffer = (uint8_t *)planarBuffer;
            }else
            {
                buffer = (uint8_t *)i2p(count);
            }
            bufSize = count * sizeof(float);
            format = AV_SAMPLE_FMT_FLTP;
            break;
        default:
            ADM_assert(0);
            break;
    }
    er = avcodec_fill_audio_frame(_frame, channels, format, buffer, bufSize, 0);
    if(er < 0)
    {
        printError("avcodec_fill_audio_frame",er);
        return false;
    }
    tmphead += count;
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
          cprintf("Channel %s\t",av_get_channel_name(chan));
          switch(chan)
          {
              CHANMIX(FRONT_LEFT,FRONT_LEFT)
              CHANMIX(FRONT_RIGHT,FRONT_RIGHT)
              CHANMIX(LFE,LOW_FREQUENCY)
              CHANMIX(FRONT_CENTER,FRONT_CENTER)
              CHANMIX(REAR_LEFT,BACK_LEFT)
              CHANMIX(REAR_RIGHT,BACK_RIGHT)
                default:
                    cprintf(" =>???\n");
                    ADM_warning("Channel %s not mapped\n",av_get_channel_name(chan));
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
    int spoonful, er;
    int channels = wavheader.channels;

    *samples = _chunk/channels; //FIXME
    *len = 0;

again:
    if(_state == AudioEncoderStopped)
        return false;

    spoonful = _chunk;
    er = 0;

    refillBuffer(_chunk);

    if(_state == AudioEncoderNoInput)
    {
        ADM_warning("[Lavcodec] No more input\n");
        int left=tmptail-tmphead;
        if (left <= 0)
        {
            if(encoderState == normal)
            {
                ADM_info("[Lavcodec] Initiating flushing\n");
                encoderState = flushing;
            }
        } else if (left <= _chunk)
        {
            ADM_info("[Lavcodec] Last audio block, %d samples left, frame size: %d\n",left/channels,ADM_LAV_SAMPLE_PER_P);
            spoonful = left;
            *samples = left/channels;
        }
    }
    if(encoderState == flushing)
    {
        er = avcodec_send_frame(CONTEXT, NULL);
        encoderState = flushed;
    }else if(encoderState == normal)
    { // feed raw audio to encoder
        if(!fillFrame(spoonful))
            return false;
        er = avcodec_send_frame(CONTEXT, _frame);
    }
    if(er < 0)
    {
        if(er != AVERROR(EAGAIN))
        {
            printError("avcodec_send_frame",er);
            return false;
        }
    }

    er = avcodec_receive_packet(CONTEXT, _pkt);

    if(er < 0)
    {
        av_packet_unref(_pkt);
        if(er == AVERROR(EAGAIN))
            goto again;
        if(er == AVERROR_EOF)
            _state = AudioEncoderStopped;
        else
            printError("avcodec_receive_packet",er);
        return false;
    }
    cprintf("Got %d bytes \n",_pkt->size);
    memcpy(dest,_pkt->data,_pkt->size);
    *len = _pkt->size;
    av_packet_unref(_pkt);
    return true;
}
#define SZT(x) sizeof(x)/sizeof(diaMenuEntry )
#define BITRATE(x) {x,QT_TRANSLATE_NOOP("lavcodec",#x)}

/**
    \fn configure
*/
bool configure (CONFcouple **setup)
{
    lav_encoder config=defaultConfig;
    if(*setup)
    {
        ADM_paramLoad(*setup,lav_encoder_param,&config);
    }

    MENU_BITRATE
    diaElemMenu bitrate(&(config.bitrate),   QT_TRANSLATE_NOOP("lavcodec","_Bitrate:"), SZT(bitrateM),bitrateM);



    diaElem *elems[]={&bitrate};

    if ( diaFactoryRun(QT_TRANSLATE_NOOP("lavcodec",ADM_LAV_MENU" Configuration"),1,elems))
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
     

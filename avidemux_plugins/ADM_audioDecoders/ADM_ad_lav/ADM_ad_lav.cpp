/**
    \file ADM_ad_lav.cpp
    \brief Audio decoders built around libavcodec
    \author mean (c) 2009

*/

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
}

#include "ADM_ad_plugin.h"
#include "fourcc.h"
#include "ADM_audioXiphUtils.h"

#define ADMWA_BUF (4*1024*16) // 64 kB internal
/**
 * \class ADM_AudiocoderLavcodec
 */
class ADM_AudiocoderLavcodec : public     ADM_Audiocodec
{
protected:

typedef enum
{
    asFloat,
    asFloatPlanar,
    asS16Planar,
    asS32Planar,
    asS32
} ADM_outputFlavor;

            ADM_outputFlavor    outputFlavor;
            AVCodecContext      *_context;
            AVFrame             *_frame;

            uint8_t     *_paddedExtraData;
            uint8_t    _buffer[ ADMWA_BUF];

            uint32_t   _tail,_head;
            uint32_t   _blockalign;
            uint32_t    channels;
            uint32_t    outputFrequency;

            bool        frequencyChecked;
            bool        nbChannelsChecked;

            bool        decodeToS16(float **outptr,uint32_t *nbOut);
            bool        decodeToFloat(float **outptr,uint32_t *nbOut);
            bool        decodeToFloatPlanar(float **outptr,uint32_t *nbOut);
            bool        decodeToS16Planar(float **outptr,uint32_t *nbOut);
            bool        decodeToS32Planar(float **outptr,uint32_t *nbOut);
            bool        decodeToS32(float **outptr,uint32_t *nbOut);
            bool        decodeToFloatPlanarStereo(float **outptr,uint32_t *nbOut);

            bool        setChannelMapping(void);

public:
                        ADM_AudiocoderLavcodec(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
    virtual             ~ADM_AudiocoderLavcodec() ;

    virtual bool        resetAfterSeek(void);
    virtual bool        reconfigureCompleted(void);
    virtual uint8_t     run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
    virtual uint8_t     isCompressed(void) {return 1;}

    virtual uint32_t    getOutputFrequency(void);
    virtual uint32_t    getOutputChannels(void);
};

// Supported formats + declare our plugin
//*******************************************************

static  ad_supportedFormat Formats[]={
        {WAV_WMA,AD_MEDIUM_QUAL},
        {WAV_WMAPRO,AD_MEDIUM_QUAL},
        {WAV_QDM2,AD_MEDIUM_QUAL},
        {WAV_AMV_ADPCM,AD_MEDIUM_QUAL},
        {WAV_NELLYMOSER,AD_MEDIUM_QUAL},
        {WAV_DTS,AD_MEDIUM_QUAL},
        {WAV_MP3,AD_MEDIUM_QUAL},
        {WAV_MP2,AD_MEDIUM_QUAL},
        {WAV_FLAC,AD_MEDIUM_QUAL},
        {WAV_AC3,AD_LOW_QUAL},   // liba52 preferred ???
        {WAV_AAC,AD_HIGH_QUAL},
        {0x706D,AD_HIGH_QUAL},
        {WAV_EAC3,AD_MEDIUM_QUAL},
        {WAV_OGG_VORBIS,AD_HIGH_QUAL},
};

DECLARE_AUDIO_DECODER(ADM_AudiocoderLavcodec,						// Class
                        0,0,1, 							       // Major, minor,patch
                        Formats, 							// Supported formats
                        "Lavcodec decoder plugin for avidemux (c) Mean/Gruntster\n"); 	// Desc
//********************************************************


/**
        \fn resetAfterSeek
*/
   bool ADM_AudiocoderLavcodec::resetAfterSeek( void )
   {
            avcodec_flush_buffers(_context);
            _tail=_head=0;
            frequencyChecked=false;
            nbChannelsChecked=false;
            return 1;
   };
/**
        \fn ADM_AudiocoderLavcodec
*/

 ADM_AudiocoderLavcodec::ADM_AudiocoderLavcodec(uint32_t fourcc,WAVHeader *info,uint32_t l,uint8_t *d)
       :  ADM_Audiocodec(fourcc,*info)
 {
    ADM_info(" [ADM_AD_LAV] Using decoder for type 0x%x\n",info->encoding);
    ADM_info(" [ADM_AD_LAV] #of channels %d\n",info->channels);
    _tail=_head=0;
    _paddedExtraData=NULL;
    _blockalign=0;
    _frame=av_frame_alloc();
    AVCodecID codecID = AV_CODEC_ID_NONE;
    outputFrequency=info->frequency;
    channels=info->channels;
    frequencyChecked=false;
    nbChannelsChecked=false;
    switch(fourcc)
    {
      case WAV_WMAPRO:
        codecID = AV_CODEC_ID_WMAPRO;
        break;
      case WAV_WMA:
        codecID = AV_CODEC_ID_WMAV2;
        break;
      case WAV_QDM2:
        codecID = AV_CODEC_ID_QDM2;
        break;
      case WAV_AMV_ADPCM:
        codecID = AV_CODEC_ID_ADPCM_IMA_AMV;
        _blockalign=1;
        break;
      case WAV_NELLYMOSER:
        codecID = AV_CODEC_ID_NELLYMOSER;
        _blockalign=1;
        break;
      case WAV_DTS:
        codecID = AV_CODEC_ID_DTS;
        _blockalign = 1;
        break;
      case WAV_FLAC:
        codecID = AV_CODEC_ID_FLAC;
        _blockalign = 1;
        break;        
      case WAV_MP3:
        codecID = AV_CODEC_ID_MP3;
        _blockalign = 1;
        break;
      case WAV_MP2:
        codecID = AV_CODEC_ID_MP2;
        _blockalign = 1;
        break;
      case WAV_AC3:
        codecID = AV_CODEC_ID_AC3;
        _blockalign = 1;
        break;
      case WAV_EAC3:
        codecID = AV_CODEC_ID_EAC3;
        _blockalign = 1;
        break;
      case WAV_OGG_VORBIS:
        codecID = AV_CODEC_ID_VORBIS;
        _blockalign = 1;
        break;
      case WAV_AAC:
      case 0x706D:
        codecID = AV_CODEC_ID_AAC;
        _blockalign = 1;
        break;
      default:
             ADM_assert(0);
    }   

    AVCodec *codec=avcodec_find_decoder(codecID);
    if(!codec) {ADM_assert(0);}

    _context=avcodec_alloc_context3(codec);
    ADM_assert(_context);

    // Fills in some values...
    _context->codec_type=AVMEDIA_TYPE_AUDIO;
    _context->sample_rate = info->frequency;
    _context->channels = info->channels;
    _context->block_align = info->blockalign;
    _context->bit_rate = info->byterate*8;
    _context->sample_fmt=AV_SAMPLE_FMT_FLT;
    _context->request_sample_fmt=AV_SAMPLE_FMT_FLT;

    if(fourcc==WAV_OGG_VORBIS)
    {
        // Need to translate from adm to xiph
        int xiphLen=(int)l+(l/255)+4+5;
        uint8_t *xiph=new uint8_t[xiphLen+AV_INPUT_BUFFER_PADDING_SIZE];
        memset(xiph,0,xiphLen+AV_INPUT_BUFFER_PADDING_SIZE);
        xiphLen=ADMXiph::admExtraData2xiph(l,d,xiph);
        _paddedExtraData=xiph;
        l=xiphLen;
    }else if(l)
    {
        _paddedExtraData=new uint8_t[l+AV_INPUT_BUFFER_PADDING_SIZE];
        memset(_paddedExtraData,0,l+AV_INPUT_BUFFER_PADDING_SIZE);
        memcpy(_paddedExtraData,d,l);
    }
    _context->extradata=_paddedExtraData;
    _context->extradata_size=l;

    if (!_blockalign) 
    {
      _blockalign = _context->block_align;
    }

    ADM_info("[ADM_AD_LAV] Using %d bytes of extra header data, %d channels\n", _context->extradata_size,_context->channels);
    mixDump((uint8_t *)_context->extradata,_context->extradata_size);
    
    if (avcodec_open2(_context, codec, NULL) < 0)
    {
         ADM_warning("[audioCodec] Cannot use float, retrying with floatp \n");
         _context->sample_fmt=AV_SAMPLE_FMT_FLTP;
         _context->request_sample_fmt=AV_SAMPLE_FMT_FLTP;
         if (avcodec_open2(_context, codec, NULL) < 0)
          {
              ADM_warning("[audioCodec] floatp failed also. Crashing.. \n");
              ADM_assert(0);
          }
          ADM_info("Decoder created using floatp..\n");
    }

    switch(_context->sample_fmt)
    {
        case AV_SAMPLE_FMT_FLT:
            outputFlavor=asFloat;
            ADM_info("Decoder created using float..\n");
            break;
        case AV_SAMPLE_FMT_FLTP:
            outputFlavor=asFloatPlanar;            
            ADM_info("Decoder created using float planar...\n");
            break;
        case AV_SAMPLE_FMT_S16P:
            outputFlavor=asS16Planar;
            ADM_info("Decoder created using s16 planar...\n");
            break;
        case AV_SAMPLE_FMT_S32P:
            outputFlavor=asS32Planar;            
            ADM_info("Decoder created using s32 planar...\n");
            break;            
        case AV_SAMPLE_FMT_S32:
            outputFlavor=asS32;
            ADM_info("Decoder created using s32 ...\n");
            break;                        
        default:
            ADM_info("Decoder created using ??? %d...\n",_context->sample_fmt);
            ADM_assert(0);
            break;
    }



    if(!_blockalign)
    {
      if(_context->block_align) 
      {
          _blockalign=_context->block_align;
      }
      else
      {
        ADM_info("[ADM_ad_lav] : no blockalign taking 378\n");
        _blockalign=378;
      }
    }
    ADM_info("[ADM_ad_lav] init successful (blockalign %d), channels=%d\n",_blockalign,_context->channels);
    if(_context->sample_rate!=outputFrequency)
    {
        ADM_warning("Output frequency does not match input frequency (SBR ?) : %d / %d\n",
            _context->sample_rate,outputFrequency);
        reconfigureNeeded=true;
    }
    if(_context->channels!=info->channels)
    {
        ADM_warning("Decoder and demuxer disagree about # of channels: %d / %d\n",_context->channels,info->channels);
        reconfigureNeeded=true;
    }
}
/**
    \fn dtor
*/
 ADM_AudiocoderLavcodec::~ADM_AudiocoderLavcodec()
{
    avcodec_close(_context);
    av_free(_context);
    _context=NULL;
    av_frame_free(&_frame);
    if(_paddedExtraData)
    {
        delete [] _paddedExtraData;
       _paddedExtraData=NULL;
    }
}
/**
    \fn decodeToS16
 * \brief THIS IS NOT USED ANY LONGER
*/
bool ADM_AudiocoderLavcodec::decodeToS16(float **outptr,uint32_t *nbOut)
{
    // convert s16 to float...
    int nbSample=  _frame->nb_samples; 
    for(int i=0;i<nbSample;i++)
    {
       for(int c=0;c<channels;c++)
       {
          int16_t *p=(int16_t *)(_frame->data[c]);
          (*outptr)[c]=((float)*(p+i))/32767.;
       }
       *outptr+=channels;
    }
    (*nbOut)+=nbSample*channels;
    return true;
}

/**
    \fn decodeToFloat
*/

bool ADM_AudiocoderLavcodec::decodeToFloat(float **outptr,uint32_t *nbOut)
{
    int nbSample=  _frame->nb_samples; 
    // copy
    memcpy(*outptr,_frame->data[0],sizeof(float)*nbSample*channels); // not sure...     
    (*outptr)+=nbSample*channels;
    (*nbOut)+=nbSample*channels;
    return true;
}

/**
 * \fn decodeToFloatPlanarStereo
 * @param outptr
 * @param nbOut
 * @return 
 */
bool ADM_AudiocoderLavcodec::decodeToFloatPlanarStereo(float **outptr,uint32_t *nbOut)
{

    // Interleave
    int nbSample=  _frame->nb_samples; 
    float *left =(float *)_frame->data[0];
    float *right=(float *)_frame->data[1];
    float *out=*outptr;
    for(int i=0;i<nbSample;i++)
    {
        out[0]=*(left++);
        out[1]=*(right++);
        out+=2;
    }
   *outptr=out;
   (*nbOut)+=nbSample*2;
   return true;
}
/**
 * \fn decodeToS16Planar
 * @param outptr
 * @param nbOut
 * @return
 */
bool ADM_AudiocoderLavcodec::decodeToS16Planar(float **outptr,uint32_t *nbOut)
{
    // Interleave
    int nbSample=_frame->nb_samples;
    float scale=1./(float)(1LL<<15LL);
    float *p=*outptr;
    for(int c=0;c<channels;c++)
    {
        float *o=p+c;
        int16_t *d=(int16_t *)_frame->data[c];
        for(int i=0;i<nbSample;i++)
        {
            *o=((float)d[i])*scale;
            o+=channels;
        }
    }
    (*nbOut) +=nbSample*channels;
    (*outptr)+=nbSample*channels;
    return true;
}
/**
 * \fn decodeToS32Planar
 * @param outptr
 * @param nbOut
 * @return 
 */
bool ADM_AudiocoderLavcodec::decodeToS32Planar(float **outptr,uint32_t *nbOut)
{
     // Interleave
    int nbSample=  _frame->nb_samples; 
    float scale=1./(float)(1LL<<31LL);
    float *p=*outptr;
    for(int c=0;c<channels;c++)
    {
        float *o=p+c;
        int32_t *d=(int32_t *)_frame->data[c];
        
        for(int i=0;i<nbSample;i++)
        {
            *o=((float)d[i])*scale;
            o+=channels;
        }
    }
   (*nbOut) +=nbSample*channels;
   (*outptr)+=nbSample*channels;
   return true;
}

/**
    \fn decodeToS32
*/

bool ADM_AudiocoderLavcodec::decodeToS32(float **outptr,uint32_t *nbOut)
{
    int nbSample=  _frame->nb_samples*channels; 
    float scale=1./(float)(1LL<<31LL);
    float *p=*outptr;
    int32_t *q=(int32_t *)_frame->data[0];
    for(int c=0;c<nbSample;c++)
    {
            *p=(float)(*q)*scale;
            p++;q++;
    }
   (*nbOut) +=nbSample;
   (*outptr)+=nbSample;
    return true;
}
/**
    \fn decodeToFloatPlanar
*/

bool ADM_AudiocoderLavcodec::decodeToFloatPlanar(float **outptr,uint32_t *nbOut)
{
    switch(channels)
    {
        case 1:
            return decodeToFloat(outptr,nbOut);
            break;
        case 2:
            return decodeToFloatPlanarStereo(outptr,nbOut);
            break;
        default:
            break;
    }
    // Interleave
    int nbSample=  _frame->nb_samples; 
    for(int i=0;i<nbSample;i++)
    {
        for(int c=0;c<channels;c++)
        {
           float *p=(float *)_frame->data[c];
           (*outptr)[c]=p[i]; // we can do better here
        }
        (*outptr)+=channels;
    }
   (*nbOut)+=nbSample*channels;
   return true;
}
/**
    \fn run

*/
uint8_t ADM_AudiocoderLavcodec::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
    *nbOut=0;
    // Shrink
    if(_head && (_tail+nbIn)*3>ADMWA_BUF*2)
    {
        memmove(_buffer,_buffer+_head,_tail-_head);
        _tail-=_head;
        _head=0;
    }
    //
    ADM_assert(nbIn+_tail<=ADMWA_BUF);
    memcpy(_buffer+_tail,inptr,nbIn);
    _tail+=nbIn;

    AVPacket pkt;
    av_init_packet(&pkt);
    int nbChunk,res=0;
    bool eof=false;
    while(_tail-_head>=_blockalign && !eof)
    {
        nbChunk=(_tail-_head)/_blockalign;
        pkt.size=nbChunk*_blockalign;
        pkt.data=_buffer+_head;

        res=avcodec_send_packet(_context, &pkt);
        if(res==AVERROR(EAGAIN)) res=0; // we are about to read output anyway
        // Regardless of the outcome, always consume the data.
        _head+=nbChunk*_blockalign;

        while(!res)
        {
            res=avcodec_receive_frame(_context, _frame);
            if(res==AVERROR(EAGAIN)) break; // we need to send more input
            if(res==AVERROR_EOF)
            {
                eof=true;
                break;
            }
            if(res<0)
            {
                char er[AV_ERROR_MAX_STRING_SIZE]={0};
                av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, res);
                ADM_warning("[ADM_ad_lav] decoding error %d: %s\n",res,er);
                break;
            }

            if(_context->sample_rate!=outputFrequency)
            {
                if(!frequencyChecked)
                {
                    ADM_warning("Output frequency %d does not match input frequency %d. Implicit SBR?\n",
                        _context->sample_rate,outputFrequency);
                }
                frequencyChecked=true;
                reconfigureNeeded=true;
            }
            if(_context->channels!=channels)
            {
                if(!nbChannelsChecked)
                {
                    ADM_warning("Decoder and demuxer disagree about # of channels: %d vs %u\n",
                        _context->channels,channels);
                }
                nbChannelsChecked=true;
                reconfigureNeeded=true;
            }
            if(reconfigureNeeded && _context->channels == channels && _context->sample_rate == outputFrequency)
            {
                ADM_info("Output frequency and # of channels match again, cancelling the reconfigure request\n");
                reconfigureNeeded = false;
            }

            bool invalid=false;
            int  toCheck=1;
            if(_context->sample_fmt==AV_SAMPLE_FMT_FLTP ||
               _context->sample_fmt==AV_SAMPLE_FMT_S16P ||
               _context->sample_fmt==AV_SAMPLE_FMT_S32P)
                toCheck=channels;

            for(int i=0;i<toCheck;i++)
            {
                if(!_frame->data[i]) // no data inside, abort
                {
                    invalid=true;
                    break;
                }
            }
            if(reconfigureNeeded || invalid)
            {
                if(_frame->nb_samples && _context->sample_rate)
                {
                    uint32_t fakeSamples=_frame->nb_samples;
                    if(_context->sample_rate!=outputFrequency)
                    {
                        float f=fakeSamples;
                        f*=outputFrequency;
                        f/=_context->sample_rate;
                        f+=0.49;
                        fakeSamples=f;
                    }
                    memset(outptr,0,fakeSamples*channels*sizeof(float));
                    outptr+=fakeSamples*channels;
                    (*nbOut)+=fakeSamples*channels;
                }
                continue;
            }

            switch(_context->sample_fmt)
            {
                case AV_SAMPLE_FMT_FLT:     decodeToFloat(&outptr,nbOut);break;
                case AV_SAMPLE_FMT_FLTP:    decodeToFloatPlanar(&outptr,nbOut);break;
                case AV_SAMPLE_FMT_S16P:    decodeToS16Planar(&outptr,nbOut);break;
                case AV_SAMPLE_FMT_S32P:    decodeToS32Planar(&outptr,nbOut);break;
                case AV_SAMPLE_FMT_S32:     decodeToS32(&outptr,nbOut);break;
                default:
                    ADM_info("Decoder created using ??? %d...\n",_context->sample_fmt);
                    ADM_assert(0);
                    break;
            }
        }
    }

    setChannelMapping();

    return 1;
}

/**
    \fn setChannelMapping
*/
bool ADM_AudiocoderLavcodec::setChannelMapping(void)
{
    memset(channelMapping,0,sizeof(CHANNEL_TYPE) * MAX_CHANNELS);

    CHANNEL_TYPE *p_ch_type = channelMapping;
    if(!_context->channel_layout)
        _context->channel_layout=av_get_default_channel_layout(channels);
#define HAVE(chan) (_context->channel_layout & AV_CH_ ##chan)
#define MAPIT(chan) *(p_ch_type++)=ADM_CH_ ##chan;
#define DOIT(x,y) if HAVE(x) MAPIT(y)
    DOIT(FRONT_LEFT,FRONT_LEFT)
    DOIT(FRONT_RIGHT,FRONT_RIGHT)
    DOIT(FRONT_CENTER,FRONT_CENTER)
    DOIT(LOW_FREQUENCY,LFE)
    if(HAVE(SIDE_LEFT) && !HAVE(BACK_LEFT))
        MAPIT(REAR_LEFT) // see https://trac.ffmpeg.org/ticket/3160
    if(HAVE(SIDE_RIGHT) && !HAVE(BACK_RIGHT))
        MAPIT(REAR_RIGHT)
    DOIT(BACK_LEFT,REAR_LEFT)
    DOIT(BACK_RIGHT,REAR_RIGHT)
    if(HAVE(SIDE_LEFT) && HAVE(BACK_LEFT))
        MAPIT(SIDE_LEFT)
    if(HAVE(SIDE_RIGHT) && HAVE(BACK_RIGHT))
        MAPIT(SIDE_RIGHT)

    return true;
}

/**
    \fn getOutputFrequency
    \brief return sampling rate as seen by libavcodec
*/
uint32_t ADM_AudiocoderLavcodec::getOutputFrequency(void)
{
    ADM_assert(_context);
    uint32_t freq=_context->sample_rate;
    return freq;
}
/**
    \fn getOutputChannels
    \brief return number of channels as seen by libavcodec
*/
uint32_t ADM_AudiocoderLavcodec::getOutputChannels(void)
{
    ADM_assert(_context);
    uint32_t chan=_context->channels;
    return chan;
}
/**
    \fn reconfigureCompleted
    \brief sync internal sampling rate and number of channels with context
*/
bool ADM_AudiocoderLavcodec::reconfigureCompleted(void)
{
    outputFrequency=_context->sample_rate;
    if(false==updateChannels(_context->channels))
        return false;
    channels=_context->channels;
    setChannelMapping();
    reconfigureNeeded=false;
    frequencyChecked=false;
    nbChannelsChecked=false;
    return true;
}
//--- EOF



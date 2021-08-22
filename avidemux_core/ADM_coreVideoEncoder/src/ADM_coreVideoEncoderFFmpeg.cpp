/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------

    copyright            : (C) 2002/2009 by mean
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
#include "ADM_vidMisc.h"
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "prefs.h"
#include "ADM_coreUtils.h"

extern "C"
{
char *av_strdup(const char *s);
void *av_malloc(size_t size) ;
}
#if 1
    #define aprintf(...) {}
#else
    #define aprintf printf
#endif

#define LAVS(x) Settings.lavcSettings.x

/**
    \fn ADM_coreVideoEncoderFFmpeg
    \brief Constructor

*/

ADM_coreVideoEncoderFFmpeg::ADM_coreVideoEncoderFFmpeg(ADM_coreVideoFilter *src,FFcodecSettings *set,bool globalHeader)
                    : ADM_coreVideoEncoder(src)
{
uint32_t w,h;
_hasSettings=false;

    if(set) {
        memcpy(&Settings,set,sizeof(*set));
        _hasSettings=true;
    }
    _options=NULL;
    targetColorSpace=ADM_COLOR_YV12;
    w=getWidth();
    h=getHeight();

    image=new ADMImageDefault(w,h);
    _frame=av_frame_alloc();
    ADM_assert(_frame);
    _frame->pts = AV_NOPTS_VALUE;
    _frame->width=w;
    _frame->height=h;
    _pkt = av_packet_alloc();
    ADM_assert(_pkt);
    rgbByteBuffer.setSize((w+7)*(h+7)*4);
    colorSpace=NULL;
    pass=0;
    statFileName=NULL;
    statFile=NULL;
    _globalHeader=globalHeader;
    _isMT=false;
    timeScalerNum=0;
    timeScalerDen=0;

    uint64_t inc=source->getInfo()->frameIncrement;
    if(_hasSettings && LAVS(max_b_frames))
        encoderDelay=inc*2;
    else
        encoderDelay=0;
    ADM_info("[Lavcodec] Using a video encoder delay of %d ms\n",(int)(encoderDelay/1000));
    lastLavPts = AV_NOPTS_VALUE;
    encoderState = ADM_ENCODER_STATE_FEEDING;
}
/**
    \fn ADM_coreVideoEncoderFFmpeg
    \brief Destructor
*/
ADM_coreVideoEncoderFFmpeg::~ADM_coreVideoEncoderFFmpeg()
{
    if (_context)
    {
        if (_isMT )
        {
          printf ("[lavc] killing threads\n");
          _isMT = false;
        }
        char *stats = _context->stats_in;
        avcodec_free_context(&_context);
        av_freep(&stats);
    }
    if (_options)
    {
        av_dict_free(&_options);
        _options=NULL;
    }
    av_frame_free(&_frame);
    av_packet_free(&_pkt);

    if(colorSpace)
    {
        delete colorSpace;
        colorSpace=NULL;
    }
    if(statFile)
    {
        printf("[ffMpeg4Encoder] Closing stat file\n");
        fclose(statFile);
        statFile=NULL;
    }
    ADM_dealloc(statFileName);
    statFileName=NULL;
}
/**
    \fn prolog
*/

bool             ADM_coreVideoEncoderFFmpeg::prolog(ADMImage *img)
{
    int w=getWidth();

  switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:    _frame->linesize[0] = img->GetPitch(PLANAR_Y);
                                _frame->linesize[1] = img->GetPitch(PLANAR_U);
                                _frame->linesize[2] = img->GetPitch(PLANAR_V);
                                _frame->format=AV_PIX_FMT_YUV420P;
                                _context->pix_fmt =AV_PIX_FMT_YUV420P;break;
        case ADM_COLOR_YUV422P: w = ADM_IMAGE_ALIGN(w);
                                _frame->linesize[0] = w;
                                _frame->linesize[1] = w>>1;
                                _frame->linesize[2] = w>>1;
                                _frame->format=AV_PIX_FMT_YUV422P;
                                _context->pix_fmt =AV_PIX_FMT_YUV422P;break;
        case ADM_COLOR_RGB32A : _frame->linesize[0] = ADM_IMAGE_ALIGN(w*4);
                                _frame->linesize[1] = 0;//w >> 1;
                                _frame->linesize[2] = 0;//w >> 1;
                                _frame->format=AV_PIX_FMT_RGB32;
                                _context->pix_fmt =AV_PIX_FMT_RGB32;break;
        case ADM_COLOR_RGB24:   _frame->linesize[0] = ADM_IMAGE_ALIGN(w*3);
                                _frame->linesize[1] = 0;
                                _frame->linesize[2] = 0;
                                _frame->format = AV_PIX_FMT_RGB24;
                                _context->pix_fmt = AV_PIX_FMT_RGB24;break;
        default: ADM_assert(0);

    }
    return true;
}
/**
    \fn ADM_coreVideoEncoderFFmpeg
*/
int64_t          ADM_coreVideoEncoderFFmpeg::timingToLav(uint64_t val)
{
    double q=(double)val;
    q/=1000.;
    q*=timeScalerDen;
    q/=timeScalerNum;
    q/=1000.;
    q+=0.49;
    int64_t v=floor(q);
#if 0
    printf("Lav in=%llu, scale=%d/%d,",val,timeScalerNum,timeScalerDen);
    printf(" q=%lf,out PTS=%lld\n",q,v);
#endif
    return v;
}
/**
    \fn lavToTiming
*/
uint64_t         ADM_coreVideoEncoderFFmpeg::lavToTiming(int64_t val)
{
    double v=val;
    v*=timeScalerNum;
    v/=timeScalerDen;
    v*=1000.*1000.;
    v+=0.49;
    return floor(v);
}

/**
    \fn pre-encoder

*/
bool             ADM_coreVideoEncoderFFmpeg::preEncode(void)
{
    if(encoderState != ADM_ENCODER_STATE_FEEDING)
        return false;

    uint32_t nb;
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[ff] Cannot get next image\n");
        encoderState = ADM_ENCODER_STATE_START_FLUSHING;
        return false;
    }
    prolog(image);

    uint64_t p=image->Pts;
    queueOfDts.push_back(p);
    aprintf("Incoming frame PTS=%" PRIu64", delay=%" PRIu64"\n",p,getEncoderDelay());
    p+=getEncoderDelay();
    _frame->pts= timingToLav(p);    //
    if(_frame->pts!=AV_NOPTS_VALUE && lastLavPts!=AV_NOPTS_VALUE && _frame->pts==lastLavPts)
    {
        ADM_warning("Lav PTS collision at frame %" PRIu32", lav PTS=%" PRId64", time %s\n",nb,_frame->pts,ADM_us2plain(p));
        _frame->pts++;
    }
    lastLavPts=_frame->pts;

    ADM_timeMapping map; // Store real PTS <->lav value mapping
    map.realTS=p;
    map.internalTS=_frame->pts;
    mapper.push_back(map);

    aprintf("Codec> real pts=%" PRIu64", internal pts=%" PRId64"\n",p,_frame->pts);
    //printf("--->>[PTS] :%"PRIu64", raw %"PRIu64" num:%"PRIu32" den:%"PRIu32"\n",_frame->pts,image->Pts,_context->time_base.num,_context->time_base.den);
    //
    int w=getWidth();
    int h=getHeight();
    switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:
                _frame->data[0] = image->GetWritePtr(PLANAR_Y);
                _frame->data[1] = image->GetWritePtr(PLANAR_U);
                _frame->data[2] = image->GetWritePtr(PLANAR_V);
                break;

        case ADM_COLOR_YUV422P:
        {
                if(!colorSpace->convertImage(image,rgbByteBuffer.at(0)))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                uint8_t *p = rgbByteBuffer.at(0);
                uint32_t off = ADM_IMAGE_ALIGN(w) * ADM_IMAGE_ALIGN(h);
                _frame->data[0] = p;
                p += off;
                _frame->data[1] = p;
                p += off >> 1;
                _frame->data[2] = p;
                break;
        }
        case ADM_COLOR_RGB32A:
        case ADM_COLOR_RGB24:
        {
                if(!colorSpace->convertImage(image,rgbByteBuffer.at(0)))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                _frame->data[0] = rgbByteBuffer.at(0);
                _frame->data[2] = NULL;
                _frame->data[1] = NULL;
                break;
        }
        default:
                ADM_assert(0);
    }
    return true;
}

/**
    \fn configure-context
    \brief To be overriden in classes which need to preform operations to the context prior to opening the codec.
*/
bool             ADM_coreVideoEncoderFFmpeg::configureContext(void)
{
    return true;
}

static int printLavError(int er)
{
    if(er>=0)
        return 0;
    char msg[AV_ERROR_MAX_STRING_SIZE]={0};
    av_make_error_string(msg, AV_ERROR_MAX_STRING_SIZE, er);
    ADM_warning("Error %d encoding video (%s)\n",er,msg);
    return er;
}

/**
 * \fn encodeWrapper
 */
int ADM_coreVideoEncoderFFmpeg::encodeWrapper(AVFrame *in,ADMBitstream *out)
{
    int r = 0;
    switch(encoderState)
    {
        case ADM_ENCODER_STATE_FEEDING:
            r = avcodec_send_frame(_context,in);
            break;
        case ADM_ENCODER_STATE_START_FLUSHING:
            r = avcodec_send_frame(_context,NULL);
            encoderState = ADM_ENCODER_STATE_FLUSHING;
            break;
        case ADM_ENCODER_STATE_FLUSHING:
            break;
        case ADM_ENCODER_STATE_FLUSHED:
            return 0;
        default:
            ADM_assert(0);
            return 0;
    }
    if(r<0)
        return printLavError(r);

    r = avcodec_receive_packet(_context, _pkt);

    if(r < 0)
    {
        av_packet_unref(_pkt);
        if(r == AVERROR(EAGAIN))
        {
            ADM_info("Encoder needs more input to produce data.\n");
            return 0;
        }
        if(r == AVERROR_EOF)
        {
            encoderState = ADM_ENCODER_STATE_FLUSHED;
            ADM_info("End of stream.\n");
            return 0;
        }
        return printLavError(r);
    }

    ADM_assert(out->bufferSize >= _pkt->size);
    memcpy(out->data, _pkt->data, _pkt->size);
    lavPtsFromPacket = _pkt->pts;
    out->flags = (_pkt->flags & AV_PKT_FLAG_KEY)? AVI_KEY_FRAME : AVI_P_FRAME;
    out->out_quantizer = (int)floor(_frame->quality / (float) FF_QP2LAMBDA); // fallback

    int sideDataSize;
    uint8_t *sideData = av_packet_get_side_data(_pkt, AV_PKT_DATA_QUALITY_STATS, &sideDataSize);
    if(sideData && sideDataSize > 5)
    {
        int quality = 0;
        memcpy(&quality,sideData,4);
        out->out_quantizer = (int)floor(quality / (float) FF_QP2LAMBDA);

        AVPictureType pict_type = (AVPictureType)sideData[4];
        switch(pict_type)
        {
            case AV_PICTURE_TYPE_I:
                out->flags = AVI_KEY_FRAME;
                break;
            case AV_PICTURE_TYPE_B:
                out->flags = AVI_B_FRAME;
                break;
            default: break;
        }
        aprintf("[ADM_coreVideoEncoderFFmpeg::encodeWrapper] Out Quant : %d, pic type %d (%s), keyf %d\n",out->out_quantizer,pict_type,
                (out->flags == AVI_P_FRAME)? "P" : (out->flags == AVI_B_FRAME)? "B" : (out->flags == AVI_KEY_FRAME)? "I" : "?",
                (_pkt->flags & AV_PKT_FLAG_KEY)? 1 : 0);
    }
    r = _pkt->size;
    av_packet_unref(_pkt);
    return r;
}
/**
 * 
 * @param codecId
 * @return 
 */
bool ADM_coreVideoEncoderFFmpeg::setupInternal(AVCodec *codec)
{
    int res;
    _context = avcodec_alloc_context3 (codec);
    ADM_assert (_context);
    _context->width = getWidth();
    _context->height = getHeight();
    _context->strict_std_compliance = -1;

    if(_globalHeader)
    {
                ADM_info("Codec configured to use global header\n");
                _context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    prolog(image);

    FilterInfo *info=source->getInfo();
    int n = info->timeBaseNum & 0x7FFFFFFF;
    int d = info->timeBaseDen & 0x7FFFFFFF;
    ADM_assert(n);
    ADM_assert(d);
    if(isStdFrameRate(d,n))
    {
        _context->time_base.num = _context->framerate.den = n;
        _context->time_base.den = _context->framerate.num = d;
    }else
    {
        int maxClockFreq = 0x7FFFFFFF;
        switch(codec->id)
        {
            case AV_CODEC_ID_MPEG4:
                maxClockFreq = 0xFFFF;
                break;
            case AV_CODEC_ID_MPEG2VIDEO:
                maxClockFreq = 90000;
                break;
            default:break;
        }
        usSecondsToFrac(info->frameIncrement,&n,&d,maxClockFreq);
        _context->time_base.num = _context->framerate.den = n;
        _context->time_base.den = _context->framerate.num = d;
        if(codec->id == AV_CODEC_ID_MPEG2VIDEO && !isStdFrameRate(d,n))
        {
            ADM_error("Non-standard frame rate %d/%d is not supported for mpeg2video.\n",d,n);
            return false;
        }
    }
    timeScalerNum=_context->time_base.num;
    timeScalerDen=_context->time_base.den;
    printf("[ff] Time base: %d/%d, frame rate: %d/%d\n",
        _context->time_base.num, _context->time_base.den,
        _context->framerate.num, _context->framerate.den);
   if(_hasSettings && LAVS(MultiThreaded))
    {
        encoderMT();
    }

   if(!configureContext()) {
     return false;
   }
   ADM_info("Opening context\n");
   if(_options)
        res=avcodec_open2(_context, codec, &_options);
   else
        res=avcodec_open2(_context, codec, NULL);
   if(res<0)
    {   ADM_info("[ff] Opening context failed\n");
        return false;
    }

    // Now allocate colorspace
    int w,h;
    w=info->width;
    h=info->height;
    if(targetColorSpace!=ADM_COLOR_YV12)
    {
        colorSpace=new ADMColorScalerSimple(w,h,ADM_COLOR_YV12,targetColorSpace);
        if(!colorSpace)
        {
            printf("[ADM_jpegEncoder] Cannot allocate colorspace\n");
            return false;
        }
    }
    return true;
}
/**
    \fn setup
    \brief put flags before calling setup!
*/
bool ADM_coreVideoEncoderFFmpeg::setup(AVCodecID codecId)
{
   
    AVCodec *codec=avcodec_find_encoder(codecId);
    if(!codec)
    {
        printf("[ff] Cannot find codec\n");
        return false;
    }
    return setupInternal(codec);
}

/**
    \fn setup
    \brief put flags before calling setup!
*/
bool ADM_coreVideoEncoderFFmpeg::setupByName(const char *name)
{
    AVCodec *codec=avcodec_find_encoder_by_name(name);
    if(!codec)
    {
        ADM_warning("[ff] Cannot find codec with name %s\n",name);
        return false;
    }
    return setupInternal(codec);
}
/**
    \fn getExtraData
    \brief

*/
bool             ADM_coreVideoEncoderFFmpeg::getExtraData(uint32_t *l,uint8_t **d)
{
    *l=_context->extradata_size;
    *d=_context->extradata;
    return true;

}

/**
    \fn loadStatFile
    \brief load the stat file from pass 1
*/
bool ADM_coreVideoEncoderFFmpeg::loadStatFile(const char *file)
{
    ADM_info("Loading stat file %s\n",file);
    FILE *_statfile = ADM_fopen (file, "rb");

    if (!_statfile)
    {
        ADM_error ("Cannot open stat file. Does it exist?\n");
        return false;
    }

    fseek (_statfile, 0, SEEK_END);
    uint64_t statSize = ftello (_statfile);
    if (statSize >= INT_MAX - 32) // enforce the av_malloc limit
    {
        ADM_error ("Stat file too large.\n");
        fclose(_statfile);
        return false;
    }
    fseek (_statfile, 0, SEEK_SET);
    _context->stats_in = (char *) av_malloc(statSize+1);
    _context->stats_in[statSize] = 0;
    if (!fread (_context->stats_in, statSize, 1, _statfile))
    {
        ADM_error ("Cannot read stat file.\n");
        fclose(_statfile);
        return false;
    }
    fclose(_statfile);

    int i;
    char *p=_context->stats_in;
    for(i=-1; p; i++)
    {
        p= strchr(p+1, ';');
    }
    ADM_info ("Stat file loaded ok, %d frames found.\n",i);
    return true;
}
/**
        \fn postEncode
        \brief update bitstream info from output of lavcodec
*/
bool ADM_coreVideoEncoderFFmpeg::postEncode(ADMBitstream *out, uint32_t size)
{
    out->len=size;

    // Update PTS/Dts
    if(!_context->max_b_frames)
    {
            if(mapper.size())
                mapper.erase(mapper.begin());
            if(queueOfDts.size())
            {
                out->dts=out->pts=queueOfDts[0];
                queueOfDts.erase(queueOfDts.begin());
            }else
            {
                out->dts=out->pts=lastDts+ source->getInfo()->frameIncrement;
                return false; // probably empty now
            }

    } else
    {
        if(lavPtsFromPacket==AV_NOPTS_VALUE)
            return false;
        if(!getRealPtsFromInternal(lavPtsFromPacket,&(out->dts),&(out->pts)))
            return false;
    }
    // update lastDts
    lastDts=out->dts;

    aprintf("Codec>Out pts=%" PRIu64" us, out Dts=%" PRIu64"\n",out->pts,out->dts);

    // Update stats
    if(Settings.params.mode==COMPRESS_2PASS   || Settings.params.mode==COMPRESS_2PASS_BITRATE)
    {
        if(pass==1)
            if (_context->stats_out)
                fprintf (statFile, "%s", _context->stats_out);
    }
    return true;
}

/**
    \fn presetContext
    \brief put sensible values into context
*/
bool ADM_coreVideoEncoderFFmpeg::presetContext(FFcodecSettings *set)
{
	  //_context->gop_size = 250;

#define SETX(x) _context->x=set->lavcSettings.x; printf("[LAVCODEC]"#x" : %d\n",set->lavcSettings.x);

      SETX (qmin);
      SETX (qmax);
      SETX (max_b_frames);
      SETX (mpeg_quant);
      SETX (max_qdiff);
      SETX (gop_size);

#undef SETX

#define SETX(x)  _context->x=set->lavcSettings.x; printf("[LAVCODEC]"#x" : %f\n",set->lavcSettings.x);
#define SETX_COND(x)  if(set->lavcSettings.is_##x) {_context->x=set->lavcSettings.x; printf("[LAVCODEC]"#x" : %f\n",set->lavcSettings.x);} else  \
									{printf("[LAVCODEC]"#x" No activated\n");}
      SETX_COND (lumi_masking);
      SETX_COND (dark_masking);
      SETX (qcompress);
      SETX (qblur);
      SETX_COND (temporal_cplx_masking);
      SETX_COND (spatial_cplx_masking);

#undef SETX
#undef SETX_COND

    switch (set->lavcSettings.mb_eval)
	{
        case 0:
          _context->mb_decision = FF_MB_DECISION_SIMPLE;
          break;
        case 1:
          _context->mb_decision = FF_MB_DECISION_BITS;
          break;
        case 2:
          _context->mb_decision = FF_MB_DECISION_RD;
          break;
        default:
          ADM_assert (0);
	}

#define SETX(x) if(set->lavcSettings.x){ _context->flags|=AV_CODEC_FLAG##x;printf("[LAVCODEC]"#x" is set\n");}
      SETX (_4MV);
      SETX (_QPEL);
      if(set->lavcSettings._TRELLIS_QUANT) _context->trellis=1;
      //SETX(_HQ);
      //SETX (_NORMALIZE_AQP);

    if(set->lavcSettings.widescreen)
    {
        float f=getHeight();
        f*=16.;
        f/=9.;
        f+=0.49;
        int num=1,den=1;
        av_reduce(&num, &den, (uint32_t)f, getWidth(), 65535);
        _context->sample_aspect_ratio.num = num;
        _context->sample_aspect_ratio.den = den;
        printf("[LAVCODEC] 16/9 display aspect ratio is set, pixel aspect = %d:%d\n",num,den);
    }
#undef SETX
  _context->bit_rate_tolerance = 8000000;
  _context->b_quant_factor = 1.25;
  _context->b_frame_strategy = 0;
  _context->b_quant_offset = 1.25;
  _context->rtp_payload_size = 0;
  _context->strict_std_compliance = 0;
  _context->i_quant_factor = 0.8;
  _context->i_quant_offset = 0.0;
//  _context->rc_qsquish = 1.0;
//  _context->rc_qmod_amp = 0;
//  _context->rc_qmod_freq = 0;
//  _context->rc_eq = av_strdup("tex^qComp");
  _context->rc_max_rate = 000;
  _context->rc_min_rate = 000;
  _context->rc_buffer_size = 000;
//  _context->rc_buffer_aggressivity = 1.0;
//  _context->rc_initial_cplx = 0;
  _context->dct_algo = 0;
  _context->idct_algo = 0;
  _context->p_masking = 0.0;

  prolog(image);
  return true;
}

/**
    \fn setLogFile
*/
 bool         ADM_coreVideoEncoderFFmpeg::setPassAndLogFile(int pass,const char *name)
{
    if(!pass || pass >2) return false;
    if(!name) return false;
    this->pass=pass;
    statFileName=ADM_strdup(name);
    return true;
}
/**
    \fn setupPass
    \brief Setup in case of multipass

*/
bool ADM_coreVideoEncoderFFmpeg::setupPass(void)
{
    int averageBitrate; // Fixme

    // Compute average bitrate

        if(Settings.params.mode==COMPRESS_2PASS_BITRATE) averageBitrate=Settings.params.avg_bitrate*1000;
            else
            {
                uint64_t duration=source->getInfo()->totalDuration; // in us
                uint32_t avg;
                if(false==ADM_computeAverageBitrateFromDuration(duration, Settings.params.finalsize,
                                &avg))
                {
                    printf("[ffMpeg4] No source duration!\n");
                    return false;
                }
                averageBitrate=(uint32_t)avg*1000; // convert from kb/s to b/s
            }

        printf("[ffmpeg4] Average bitrate =%" PRIu32" kb/s\n",averageBitrate/1000);
        _context->bit_rate=averageBitrate;
        switch(pass)
        {
                case 1:
                    printf("[ffMpeg4] Setup-ing Pass 1\n");
                    _context->flags |= AV_CODEC_FLAG_PASS1;
                    // Open stat file
                    statFile=ADM_fopen(statFileName,"wt");
                    if(!statFile)
                    {
                        printf("[ffmpeg] Cannot open statfile %s for writing\n",statFileName);
                        return false;
                    }
                    break;
                case 2:
                    printf("[ffMpeg4] Setup-ing Pass 2\n");
                    _context->flags |= AV_CODEC_FLAG_PASS2;
                    if(false==loadStatFile(statFileName))
                    {
                        printf("[ffmpeg4] Cannot load stat file\n");
                        return false;
                    }
                    break;
                default:
                        printf("[ffmpeg] Pass=0, fail\n");
                        return false;
                    break;

        }
        return true;
}
/**
    \fn encoderMT
    \brief handle multithreaded encoding
*/
bool ADM_coreVideoEncoderFFmpeg::encoderMT (void)
{

  uint32_t threads =    LAVS(MultiThreaded);
  switch(threads)
  {
    case 99:threads = ADM_cpu_num_processors();break;
    case 1: threads=0;
    break;
  }
  if (threads)
  {
      printf ("[lavc] Enabling MT encoder with %u threads\n", threads);
      _context->thread_count=threads;
      _isMT = 1;
  }
  return true;
}


// EOF

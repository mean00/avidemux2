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
#define __STDC_CONSTANT_MACROS
#include "ADM_default.h"
#include "ADM_coreVideoEncoderFFmpeg.h"
#include "prefs.h"
#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME
#define TIME_TENTH_MILLISEC
#define aprintf(...) {}
/**
    \fn ADM_coreVideoEncoderFFmpeg
    \brief Constructor

*/

ADM_coreVideoEncoderFFmpeg::ADM_coreVideoEncoderFFmpeg(ADM_coreVideoFilter *src,FFcodecSetting *set) 
                    : ADM_coreVideoEncoder(src)
{
uint32_t w,h;
    if(set) memcpy(&Settings,set,sizeof(*set));
    targetColorSpace=ADM_COLOR_YV12;
    w=getWidth();
    h=getHeight();

    image=new ADMImage(w,h);
    _context = avcodec_alloc_context2 (CODEC_TYPE_VIDEO);
    ADM_assert (_context);
    memset (&_frame, 0, sizeof (_frame));
    _frame.pts = AV_NOPTS_VALUE;
    _context->width = w;
    _context->height = h;
    _context->strict_std_compliance = -1;
    rgbBuffer=new uint8_t [(w+7)*(h+7)*4];
    colorSpace=NULL;
    pass=0;
    statFileName=NULL;
    statFile=NULL;
    _isMT=false;

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
          avcodec_thread_free (_context);
          _isMT = false;
        }
        avcodec_close (_context);
        ADM_dealloc (_context);
        _context = NULL;
    }
    if(colorSpace)
    {
        delete colorSpace;
        colorSpace=NULL;
    }   
    if(rgbBuffer)
    {
        delete [] rgbBuffer;
        rgbBuffer=NULL;
    }
    if(statFile)
    {
        printf("[ffMpeg4Encoder] Closing stat file\n");
        fclose(statFile);
        statFile=NULL;
    }
    if(statFileName) ADM_dealloc(statFileName);
    statFileName=NULL;
}
/**
    \fn prolog
*/

bool             ADM_coreVideoEncoderFFmpeg::prolog(void)
{
    int w=getWidth();
    int h=getHeight();

  switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:    _frame.linesize[0] = w; 
                                _frame.linesize[1] = w>>1; 
                                _frame.linesize[2] = w>>1; 
                                _context->pix_fmt =PIX_FMT_YUV420P;break;
        case ADM_COLOR_YUV422P: _frame.linesize[0] = w; 
                                _frame.linesize[1] = w>>1;
                                _frame.linesize[2] = w>>1;
                                _context->pix_fmt =PIX_FMT_YUV422P;break;
        case ADM_COLOR_RGB32A : _frame.linesize[0] = w*4;
                                _frame.linesize[1] = 0;//w >> 1;
                                _frame.linesize[2] = 0;//w >> 1;
                                _context->pix_fmt =PIX_FMT_RGB32;break;
        default: ADM_assert(0);

    }
    
    // Eval fps
    uint64_t f=source->getInfo()->frameIncrement;
    // Let's put 100 us as time  base
    _context->time_base.den=10000LL;
#ifdef TIME_TENTH_MILLISEC
    _context->time_base.num=1;
#else
    if(f>=100)
        _context->time_base.num=f/100;
    else
        _context->time_base.num=1;
#endif
    //printf("[Time base] %d/%d\n", _context->time_base.num,_context->time_base.den);
    return true;
}
/**
    \fn pre-encoder
    
*/
bool             ADM_coreVideoEncoderFFmpeg::preEncode(void)
{

    uint8_t *from;
    if(source->getNextFrame(image)==false)
    {
        printf("[ff] Cannot get next image\n");
        return false;
    }
    prolog();
    // put a time stamp...
    if(image->Pts==ADM_NO_PTS) 
    {
        ADM_assert(0);
        _frame.pts=AV_NOPTS_VALUE;
    }
    else
    {
        float f=image->Pts,n=_context->time_base.num,d=_context->time_base.den;
#ifdef TIME_TENTH_MILLISEC
        f=f/100;
#else
        f=f/100;
        f=f/n;
#endif
        _frame.pts=f;

        printf("*** PTS:%d time_base :%d/%d\n",_frame.pts,_context->time_base.num,_context->time_base.den);
    }
    //
    //printf("[PTS] :%"LU" num:%"LU" den:%"LU"\n",_frame.pts,_context->time_base.num,_context->time_base.den);
    //
    switch(targetColorSpace)
    {
        case ADM_COLOR_YV12:      
                _frame.data[0] = image->GetWritePtr(PLANAR_Y);
                _frame.data[2] = image->GetWritePtr(PLANAR_U);
                _frame.data[1] = image->GetWritePtr(PLANAR_V);
                break;

        case ADM_COLOR_YUV422P:
        {
              int w=getWidth();
              int h=getHeight();

                if(!colorSpace->convert(image->data,rgbBuffer))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                _frame.data[0] = rgbBuffer;
                _frame.data[2] = rgbBuffer+(w*h);
                _frame.data[1] = rgbBuffer+(w*h*3)/2;
                break;
        }
        case ADM_COLOR_RGB32A:
                if(!colorSpace->convert(image->data,rgbBuffer))
                {
                    printf("[ADM_jpegEncoder::encode] Colorconversion failed\n");
                    return false;
                }
                _frame.data[0] = rgbBuffer;
                _frame.data[2] = NULL;
                _frame.data[1] = NULL;
                break;
        default:
                ADM_assert(0);
    }
    return true;
}
/**
    \fn setup
    \brief put flags before calling setup!
*/
bool ADM_coreVideoEncoderFFmpeg::setup(CodecID codecId)
{
    int res;
    AVCodec *codec=avcodec_find_encoder(codecId);
    if(!codec) 
    {
        printf("[ff] Cannot find codec\n");
        return false;
    }
   prolog();
   encoderMT();
   res=avcodec_open(_context, codec); 
   if(res<0) 
    {   printf("[ff] Cannot open codec\n");
        return false;
    }
   
    // Now allocate colorspace
    int w,h;
    FilterInfo *info=source->getInfo();
    w=info->width;
    h=info->height;
    if(targetColorSpace!=ADM_COLOR_YV12)
    {
        colorSpace=new ADMColorspace(w,h,ADM_COLOR_YV12,targetColorSpace);
        if(!colorSpace)
        {
            printf("[ADM_jpegEncoder] Cannot allocate colorspace\n");
            return false;
        }
    }
    return true;
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
  printf("[FFmpeg] Loading stat file :%s\n",file);
  FILE *_statfile = fopen (file, "rb");
  int statSize;

  if (!_statfile)
    {
      printf ("[ffmpeg] internal file does not exists ?\n");
      return false;
    }

  fseek (_statfile, 0, SEEK_END);
  statSize = ftello (_statfile);
  fseek (_statfile, 0, SEEK_SET);
  _context->stats_in = (char *) ADM_alloc (statSize + 1);
  _context->stats_in[statSize] = 0;
  fread (_context->stats_in, statSize, 1, _statfile);
  fclose(_statfile);


    int i;
    char *p=_context->stats_in;
   for(i=-1; p; i++){
            p= strchr(p+1, ';');
        }
  printf("[FFmpeg] stat file loaded ok, %d frames found\n",i);
  return true;
}
/**
        \fn postEncode
        \brief update bitstream info from output of lavcodec
*/
bool ADM_coreVideoEncoderFFmpeg::postEncode(ADMBitstream *out, uint32_t size)
{
    int pict_type=FF_P_TYPE;
    int keyframe=false;
    if(_context->coded_frame)
    {
        pict_type=_context->coded_frame->pict_type;
        keyframe=_context->coded_frame->key_frame;
    }
    aprintf("[ffMpeg4] Out Quant :%d, pic type %d keyf %d\n",out->out_quantizer,pict_type,keyframe);
    out->len=size;
    out->flags=0;
    if(keyframe) 
        out->flags=AVI_KEY_FRAME;
    else if(pict_type==FF_B_TYPE)
            out->flags=AVI_B_FRAME;
    out->pts=out->dts=image->Pts;
    // Update PTS/Dts
    out->pts=_frame.reordered_opaque;
    out->dts=-1; // FIXME
    

    // Update quant
    if(!_context->coded_frame->quality)
      out->out_quantizer=(int) floor (_frame.quality / (float) FF_QP2LAMBDA);
    else
      out->out_quantizer =(int) floor (_context->coded_frame->quality / (float) FF_QP2LAMBDA);

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
bool ADM_coreVideoEncoderFFmpeg::presetContext(FFcodecSetting *set)
{
	  _context->gop_size = 250;
	
#define SETX(x) _context->x=set->x; printf("[LAVCODEC]"#x" : %d\n",set->x);
#define SETX_COND(x) if(set->is_##x) {_context->x=set->x; printf("[LAVCODEC]"#x" : %d\n",set->x);} else\
		{ printf(#x" is not activated\n");}
      SETX (me_method);
      SETX (qmin);
      SETX (qmax);
      SETX (max_b_frames);
      SETX (mpeg_quant);
      SETX (max_qdiff);
      SETX_COND (luma_elim_threshold);
      SETX_COND (chroma_elim_threshold);

#undef SETX
#undef SETX_COND

#define SETX(x)  _context->x=set->x; printf("[LAVCODEC]"#x" : %f\n",set->x);
#define SETX_COND(x)  if(set->is_##x) {_context->x=set->x; printf("[LAVCODEC]"#x" : %f\n",set->x);} else  \
									{printf("[LAVCODEC]"#x" No activated\n");}
      SETX_COND (lumi_masking);
      SETX_COND (dark_masking);
      SETX (qcompress);
      SETX (qblur);
      SETX_COND (temporal_cplx_masking);
      SETX_COND (spatial_cplx_masking);

#undef SETX
#undef SETX_COND

#define SETX(x) if(set->x){ _context->flags|=CODEC_FLAG##x;printf("[LAVCODEC]"#x" is set\n");}
      SETX (_GMC);


    switch (set->mb_eval)
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
      
      SETX (_4MV);
      SETX (_QPEL);
      if(set->_TRELLIS_QUANT) _context->trellis=1;
      //SETX(_HQ);
      //SETX (_NORMALIZE_AQP);

      if (set->widescreen)
        {
          _context->sample_aspect_ratio.num = 16;
          _context->sample_aspect_ratio.den = 9;
          printf ("[LAVCODEC]16/9 aspect ratio is set.\n");

        }
#undef SETX
    
  _context->bit_rate_tolerance = 8000000;
  _context->b_quant_factor = 1.25;
  _context->rc_strategy = 2;
  _context->b_frame_strategy = 0;
  _context->b_quant_offset = 1.25;
  _context->rtp_payload_size = 0;
  _context->strict_std_compliance = 0;
  _context->i_quant_factor = 0.8;
  _context->i_quant_offset = 0.0;
  _context->rc_qsquish = 1.0;
  _context->rc_qmod_amp = 0;
  _context->rc_qmod_freq = 0;
  _context->rc_eq = const_cast < char *>("tex^qComp");
  _context->rc_max_rate = 000;
  _context->rc_min_rate = 000;
  _context->rc_buffer_size = 000;
  _context->rc_buffer_aggressivity = 1.0;
  _context->rc_initial_cplx = 0;
  _context->dct_algo = 0;
  _context->idct_algo = 0;
  _context->p_masking = 0.0;

  // Set frame rate den/num
  prolog();
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
                float f;
                if(!duration) 
                {
                    printf("[ffMpeg4] No source duration!\n");
                    return false;
                }
                f=Settings.params.finalsize; 
                f=f*1024*1024*8; // in bits
                f*=1000*1000;
                f/=duration;
                averageBitrate=(uint32_t)f;
            }

        printf("[ffmpeg4] Average bitrate =%"LU" kb/s\n",averageBitrate/1000);
        _context->bit_rate=averageBitrate;
        switch(pass)
        {
                case 1:
                    printf("[ffMpeg4] Setup-ing Pass 1\n");
                    _context->flags |= CODEC_FLAG_PASS1;
                    // Open stat file
                    statFile=fopen(statFileName,"wt");
                    if(!statFile)
                    {
                        printf("[ffmpeg] Cannot open statfile %s for writing\n",statFileName);
                        return false;
                    }
                    break;
                case 2:
                    printf("[ffMpeg4] Setup-ing Pass 2\n");
                    _context->flags |= CODEC_FLAG_PASS2;
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
  uint32_t threads = 0;

  prefs->get(FEATURE_THREADING_LAVC, &threads);

  if (threads == 0)
	  threads = ADM_cpu_num_processors();

  if (threads == 1)
	  threads = 0;

  if (threads)
  {
      printf ("[lavc] Enabling MT encoder with %u threads\n", threads);

      if (avcodec_thread_init (_context, threads) == -1)
	      printf ("[lavc] Failed!!\n");
	  else
          _isMT = 1;
  }
  return true;
}


// EOF

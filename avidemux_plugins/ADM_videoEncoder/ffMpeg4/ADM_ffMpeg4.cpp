/***************************************************************************
                          \fn ADM_ffMpeg4
                          \brief Front end for libavcodec Mpeg4 asp encoder
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
#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "ADM_ffMpeg4.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

static FFcodecSetting Settings=
{
    {
    COMPRESS_CQ, //COMPRESSION_MODE  mode;
    2,              // uint32_t          qz;           /// Quantizer
    1500,           //uint32_t          bitrate;      /// In kb/s 
    700,            //uint32_t          finalsize;    /// In ?
    1500,           //uint32_t          avg_bitrate;  /// avg_bitrate is in kb/s!!
    ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_GLOBAL+ADM_ENC_CAP_SAME
    },
          ME_EPZS,			//     ME
          0,				//          GMC     
          1,				// 4MV
          0,				//           _QPEL;   
          1,				//           _TREILLIS_QUANT
          2,				//           qmin;
          31,				//          qmax;
          3,				//           max_qdiff;
          2,				//           max_b_frames;
          0,				//          mpeg_quant;
          1,				//
          -2,				//                 luma_elim_threshold;
          1,				//
          -5,				// chroma_elim_threshold;
          0.05,				//lumi_masking;
          1,				// is lumi
          0.01,				//dark_masking; 
          1,				// is dark
          0.5,				// qcompress amount of qscale change between easy & hard scenes (0.0-1.0
          0.5,				// qblur;    amount of qscale smoothing over time (0.0-1.0) 
          0,				// min bitrate in kB/S
          0,				// max bitrate
          0,				// default matrix
          0,				// no gop size
          NULL,
          NULL,
          0,				// interlaced
          0,				// WLA: bottom-field-first
          0,				// wide screen
          2,				// mb eval = distortion
          8000,				// vratetol 8Meg
          0,				// is temporal
          0.0,				// temporal masking
          0,				// is spatial
          0.0,				// spatial masking
          0,				// NAQ
          0				// DUMMY 
};
/**
        \fn ADM_ffMpeg4Encoder
*/
ADM_ffMpeg4Encoder::ADM_ffMpeg4Encoder(ADM_coreVideoFilter *src) : ADM_coreVideoEncoderFFmpeg(src)
{
    printf("[ffMpeg4Encoder] Creating.\n");
    pass=0;
    statFileName=NULL;
    statFile=NULL;

}
/**
    \fn setupPass
    \brief Setup in case of multipass

*/
bool ADM_ffMpeg4Encoder::setupPass(void)
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
    \fn setup
*/
bool ADM_ffMpeg4Encoder::setup(void)
{
    
    switch(Settings.params.mode)
    {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
           if(false==setupPass())
            {
                printf("[ffmpeg] Multipass setup failed\n");
                return false;
            }
            break;
      case COMPRESS_SAME:
      case COMPRESS_CQ:
            _context->flags |= CODEC_FLAG_QSCALE;
  	    _context->bit_rate = 0;
            break;
      case COMPRESS_CBR:
              _context->bit_rate=Settings.params.bitrate*1000; // kb->b;
            break;
     default:
            return false;
    }
    presetContext(&Settings);
    if(false== ADM_coreVideoEncoderFFmpeg::setup(CODEC_ID_MPEG4))
        return false;

    printf("[ffMpeg] Setup ok\n");
    return true;
}


/** 
    \fn ~ADM_ffMpeg4Encoder
*/
ADM_ffMpeg4Encoder::~ADM_ffMpeg4Encoder()
{
    printf("[ffMpeg4Encoder] Destroying.\n");
    if(statFile)
    {
        fclose(statFile);
        statFile=NULL;
    }
    if(statFileName) ADM_dealloc(statFileName);
    statFileName=NULL;
    
}

/**
    \fn encode
*/
bool         ADM_ffMpeg4Encoder::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, NULL)) <= 0)
        {
            printf("[ffmpeg4] Error %d encoding video\n",sz);
            return false;
        }
        printf("[ffmpeg4] Popping delayed bframes (%d)\n",sz);
        goto link;
    }
    q=image->_Qp;
    
    if(!q) q=2;
    switch(Settings.params.mode)
    {
      case COMPRESS_SAME:
                // Keep same frame type & same Qz as the incoming frame...
            _frame.quality = (int) floor (FF_QP2LAMBDA * q+ 0.5);

            if(image->flags & AVI_KEY_FRAME)    _frame.pict_type=FF_I_TYPE;
            else if(image->flags & AVI_B_FRAME) _frame.pict_type=FF_B_TYPE;
            else                                _frame.pict_type=FF_P_TYPE;

            break;
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
            switch(pass)
            {
                case 1: 
                        break;
                case 2: 
                        break; // Get Qz for this frame...
            }
      case COMPRESS_CQ:
            _frame.quality = (int) floor (FF_QP2LAMBDA * Settings.params.qz+ 0.5);
            break;
      case COMPRESS_CBR:
            break;
     default:
            printf("[ffMpeg4] Unsupported encoding mode\n");
            return false;
    }
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame.quality, _frame.quality/ FF_QP2LAMBDA,q);     
    
    _frame.reordered_opaque=image->Pts;
    if ((sz = avcodec_encode_video (_context, out->data, out->bufferSize, &_frame)) < 0)
    {
        printf("[ffmpeg4] Error %d encoding video\n",sz);
        return false;
    }
    
    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    postEncode(out,sz);
   
    return true;
}
/**
        \fn postEncode
        \brief update bitstream info from output of lavcodec
*/
bool ADM_ffMpeg4Encoder::postEncode(ADMBitstream *out, uint32_t size)
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
bool ADM_ffMpeg4Encoder::presetContext(FFcodecSetting *set)
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
 bool         ADM_ffMpeg4Encoder::setPassAndLogFile(int pass,const char *name)
{
    if(!pass || pass >2) return false;
    if(!name) return false;
    this->pass=pass;
    statFileName=ADM_strdup(name);
    return true;
}
/**
    \fn isDualPass

*/
bool         ADM_ffMpeg4Encoder::isDualPass(void) 
{
    if(Settings.params.mode==COMPRESS_2PASS || Settings.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
    \fn loadStatFile
    \brief load the stat file from pass 1
*/
bool ADM_ffMpeg4Encoder::loadStatFile(const char *file)
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
  printf("[FFmpeg] stat file loaded ok\n");
  return true;
}
/**
    \fn jpegConfigure
    \brief UI configuration for jpeg encoder
*/

bool         ffMpeg4Configure(void)
{         
diaMenuEntry meE[]={
  {1,QT_TR_NOOP("None")},
  {2,QT_TR_NOOP("Full")},
  {3,QT_TR_NOOP("Log")},
  {4,QT_TR_NOOP("Phods")},
  {5,QT_TR_NOOP("EPZS")},
  {6,QT_TR_NOOP("X1")}
};       

diaMenuEntry qzE[]={
  {0,QT_TR_NOOP("H.263")},
  {1,QT_TR_NOOP("MPEG")}
};       

diaMenuEntry rdE[]={
  {0,QT_TR_NOOP("MB comparison")},
  {1,QT_TR_NOOP("Fewest bits (vhq)")},
  {2,QT_TR_NOOP("Rate distortion")}
};     

        FFcodecSetting *conf=&Settings;

uint32_t me=(uint32_t)conf->me_method;  
#define PX(x) &(conf->x)

         diaElemBitrate   bitrate(&(Settings.params),NULL);
         diaElemMenu      meM(&me,QT_TR_NOOP("Matrices"),4,meE);
         diaElemUInteger  qminM(PX(qmin),QT_TR_NOOP("Mi_n. quantizer:"),1,31);
         diaElemUInteger  qmaxM(PX(qmax),QT_TR_NOOP("Ma_x. quantizer:"),1,31);
         diaElemUInteger  qdiffM(PX(max_qdiff),QT_TR_NOOP("Max. quantizer _difference:"),1,31);
         
         diaElemToggle    fourMv(PX(_4MV),QT_TR_NOOP("4_MV"));
         diaElemToggle    trellis(PX(_TRELLIS_QUANT),QT_TR_NOOP("_Trellis quantization"));
         
         diaElemToggle    qpel(PX(_QPEL),QT_TR_NOOP("_Quarter pixel"));
         diaElemToggle    gmc(PX(_GMC),QT_TR_NOOP("_GMC"));

         
         diaElemUInteger  max_b_frames(PX(max_b_frames),QT_TR_NOOP("_Number of B frames:"),0,32);
         diaElemMenu     qzM(PX(mpeg_quant),QT_TR_NOOP("_Quantization type:"),2,qzE);
         
         diaElemMenu     rdM(PX(mb_eval),QT_TR_NOOP("_Macroblock decision:"),3,rdE);
         
         diaElemUInteger filetol(PX(vratetol),QT_TR_NOOP("_Filesize tolerance (kb):"),0,100000);
         
         diaElemFloat    qzComp(PX(qcompress),QT_TR_NOOP("_Quantizer compression:"),0,1);
         diaElemFloat    qzBlur(PX(qblur),QT_TR_NOOP("Quantizer _blur:"),0,1);
         
         
          /* First Tab : encoding mode */
        diaElem *diamode[]={&bitrate};
        diaElemTabs tabMode(QT_TR_NOOP("User Interface"),1,diamode);
        
        /* 2nd Tab : ME */
        diaElemFrame frameMe(QT_TR_NOOP("Advanced Simple Profile"));
        
        frameMe.swallow(&max_b_frames);
        frameMe.swallow(&qpel);
        frameMe.swallow(&gmc);
        
        diaElem *diaME[]={&fourMv,&frameMe};
        diaElemTabs tabME(QT_TR_NOOP("Motion Estimation"),2,diaME);
        /* 3nd Tab : Qz */
        
         diaElem *diaQze[]={&qzM,&rdM,&qminM,&qmaxM,&qdiffM,&trellis};
        diaElemTabs tabQz(QT_TR_NOOP("Quantization"),6,diaQze);
        
        /* 4th Tab : RControl */
        
         diaElem *diaRC[]={&filetol,&qzComp,&qzBlur};
        diaElemTabs tabRC(QT_TR_NOOP("Rate Control"),3,diaRC);
        
         diaElemTabs *tabs[]={&tabMode,&tabME,&tabQz,&tabRC};
        if( diaFactoryRunTabs(QT_TR_NOOP("libavcodec MPEG-4 configuration"),4,tabs))
        {
          conf->me_method=(Motion_Est_ID)me;
          return true;
        }
         return false;
}
// EOF

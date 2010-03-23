/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean/gruntster
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
#include "ADM_x264.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C" 
{
static void        logger( void *cooki, int i_level, const char *psz, va_list list)
{
    static char buffer[2048];
    vsnprintf(buffer,2048,psz,list);
    printf(">>%s\n",buffer);
}
}
#define MMSET(x) memset(&(x),0,sizeof(x))

x264_encoder x264Settings=
{
    {
    COMPRESS_CQ, //COMPRESSION_MODE  mode;
    15,              // uint32_t          qz;           /// Quantizer
    1500,           //uint32_t          bitrate;      /// In kb/s 
    700,            //uint32_t          finalsize;    /// In ?
    1500,           //uint32_t          avg_bitrate;  /// avg_bitrate is in kb/s!!
    ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+0*COMPRESS_AQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL+ADM_ENC_CAP_SAME
    },
    2, // uint32_t MaxRefFrames;
    20, //uint32_t MinIdr;
    50, //uint32_t MaxIdr;
    2,  // threads
   true, //bool _8x8;
   true, //bool _8x8P;
   true, //bool _8x8B;
   true, //bool _4x4;
   true, //bool _8x8I;
   true, //bool _4x4I;
   2, //uint32_t MaxBFrame;
   30, //uint32_t profile;
   true, //bool CABAC;
   true, //bool Trellis;     
    
};
/**
        \fn x264Encoder
*/
x264Encoder::x264Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[x264] Creating.\n");
    handle=NULL;
    extraData=NULL;
    extraDataLen=0;
    seiUserDataLen=0;
    seiUserData=NULL;
    pic=NULL;
    
}
/**
    \fn setup
*/
bool x264Encoder::setup(void)
{
  ADM_info("x264, setting up");
  image=new ADMImage(getWidth(),getHeight());
  MMSET(param);
  x264_param_default( &param);
  param.pf_log=logger;
  param.i_threads = x264Settings.threads;
  param.i_width = getWidth();
  param.i_height = getHeight();
  param.i_csp = X264_CSP_I420;
 
    //Framerate
    int n,d;    
    uint64_t f=source->getInfo()->frameIncrement;
    usSecondsToFrac(f,&n,&d);
    param.i_fps_num = n;
    param.i_fps_den = d;


#define MKPARAM(x,y) {param.x = x264Settings.y;printf("[x264] "#x" = %d\n",param.x);}
#define MKPARAMF(x,y) {param.x = (float)x264Settings.y / 100; printf("[x264] "#x" = %.2f\n",param.x);}

#if 0
  if (zparam->AR_AsInput) {
    param.vui.i_sar_width = video_body->getPARWidth();
    param.vui.i_sar_height = video_body->getPARHeight();
  } else {
    MKPARAM(vui.i_sar_width , AR_Num);	// FIXME
    MKPARAM(vui.i_sar_height, AR_Den);
  }
  if(zparam->idc)
  {
    MKPARAM(i_level_idc,idc);
    printf("[x264] *** Forcing level = %d\n",param.i_level_idc);
  }
  // KeyframeBoost ?
  // BframeReduction ?
  // PartitionDecision ?
  MKPARAMF(rc.f_qcompress,BitrateVariability);

  // update for Sadarax dialog
  MKPARAM(rc.i_vbv_max_bitrate,vbv_max_bitrate);
  MKPARAM(rc.i_vbv_buffer_size,vbv_buffer_size);
  MKPARAMF(rc.f_vbv_buffer_init,vbv_buffer_init);
  
  MKPARAM (analyse.b_fast_pskip,fastPSkip);
  MKPARAM (analyse.b_dct_decimate,DCTDecimate);
  MKPARAM (b_interlaced,interlaced);
      
  //
  MKPARAM(analyse.i_direct_mv_pred,DirectMode+1);
  MKPARAM(rc.i_qp_min,MinQp);
  MKPARAM(rc.i_qp_max,MaxQp);
  MKPARAM(rc.i_qp_step,QpStep);
  MKPARAM(i_scenecut_threshold,SceneCut);
  MKPARAM(i_bframe_bias,Bias);
  MKPARAM( b_bframe_pyramid,BasReference );
  MKPARAM(analyse. b_bidir_me,BidirME );
  MKPARAM( b_bframe_adaptive, Adaptative);
  MKPARAM( analyse.b_weighted_bipred, Weighted);
  MKPARAM(analyse.i_subpel_refine,PartitionDecision+1);
#endif
  MKPARAM(i_frame_reference,MaxRefFrames);
  
  MKPARAM(i_keyint_min,MinIdr);
  MKPARAM(i_keyint_max,MaxIdr);
  MKPARAM(i_bframe,MaxBFrame);

  MKPARAM( b_cabac , CABAC);
  MKPARAM( analyse.i_trellis, Trellis);

#if 0  
#define MIN_RDO 6
  if(zparam->PartitionDecision+1>=MIN_RDO)
  {
      int rank,parity;
      rank=((zparam->PartitionDecision+1-MIN_RDO)>>1)+MIN_RDO;
      parity=(zparam->PartitionDecision+1-MIN_RDO)&1;
      
      param.analyse.i_subpel_refine=rank;
      param.analyse.b_bframe_rdo=parity;
  }
  MKPARAM(analyse.b_chroma_me,ChromaME);
  MKPARAM(b_deblocking_filter,DeblockingFilter);
  MKPARAM(i_deblocking_filter_alphac0,Strength );
  MKPARAM(i_deblocking_filter_beta, Threshold);
  
  MKPARAM(analyse.i_me_method,Method);
  MKPARAM(analyse.i_me_range,Range);
  MKPARAM(i_bframe_bias,Bias);
  MKPARAM( b_bframe_pyramid,BasReference );
  MKPARAM(analyse. b_bidir_me,BidirME );
  MKPARAM( b_bframe_adaptive, Adaptative);
  MKPARAM( analyse.b_weighted_bipred, Weighted);
#endif
//  MKPARAM(PartitionDecision,Method);
  MKPARAM(analyse.b_transform_8x8,_8x8);
  
#define MES(x,y) if(x264Settings.x) {param.analyse.inter |=X264_ANALYSE_##y;printf("[x264] "#x" is on\n");}
  param.analyse.inter=0;
  MES(  _8x8P,  PSUB16x16);
  MES(  _8x8B,  BSUB16x16);
  MES(  _4x4,   PSUB8x8);
  MES(  _8x8I,  I8x8);
  MES(  _4x4I,  I4x4);


  param.i_log_level=X264_LOG_DEBUG; //INFO;
 
  
  if(globalHeader)
      param.b_repeat_headers=0;
  else
      param.b_repeat_headers=1;

  handle = x264_encoder_open (&param);
  if (!handle)
  {
    ADM_error("Cannot initialize x264\n");
    return 0;
  }

  pic=new x264_picture_t;
  x264_picture_alloc( pic, X264_CSP_I420, getWidth(),getHeight());
  ADM_info("x264, setup ok\n");
  if (globalHeader)
    return createHeader ();
  return true;
}
/**
    \fn encodeNals
*/
int x264Encoder::encodeNals(uint8_t *buf, int size, x264_nal_t *nals, int nalCount, bool skipSei)
{
    uint8_t *p = buf;
    int i;

    if (seiUserDataLen > 0 && nalCount > 0)
        {
        memcpy(p, seiUserData, seiUserDataLen);
        p += seiUserDataLen;
        seiUserDataLen = 0;
    }

    for (i = 0; i < nalCount; i++)
        {
        if (skipSei && nals[i].i_type == NAL_SEI)
                {
            seiUserDataLen = nals[i].i_payload;
            seiUserData = new uint8_t[seiUserDataLen];
            memcpy(seiUserData, nals[i].p_payload, nals[i].i_payload);
            continue;
        }

        memcpy(p, nals[i].p_payload, nals[i].i_payload);
        p += nals[i].i_payload;
    }

    return p - buf;
}

/**
        \fn createHeader
        \brief create esds header, needed for mp4/mov
*/
#define MAX_HEADER_X264 1024	// should be enough
#define H264_NAL_TYPE_SEI       0x6
#define H264_NAL_TYPE_SEQ_PARAM 0x7
#define H264_NAL_TYPE_PIC_PARAM 0x8

bool x264Encoder::createHeader (void)
{

  x264_nal_t *nal;
  int        nalCount;

    extraDataLen = x264_encoder_headers(handle, &nal, &nalCount);
    extraData = new uint8_t[extraDataLen];
    extraDataLen = encodeNals(extraData, extraDataLen, nal, nalCount, true);

  return 1;
}
/** 
    \fn ~x264Encoder
*/
x264Encoder::~x264Encoder()
{
    ADM_info("[x264] Destroying.\n");
    if (handle)
    {
      x264_encoder_close (handle);
      handle = NULL;
    }
    if (pic)
    {
      // picture_clean ?
      x264_picture_clean(pic);
      delete pic;
      pic = NULL;
    }
    if(extraData)
    {
        delete [] extraData;
        extraData=NULL;
    }

  if(seiUserData)
  {
	  delete [] seiUserData;
	  seiUserData=NULL;
  }
}

/**
    \fn encode
*/
bool         x264Encoder::encode (ADMBitstream * out)
{
    // 1 fetch a frame...
    uint32_t nb;
    // update
again:    
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[x264] Cannot get next image\n");
        return false;
    }
#if 0
    // Store Pts/DTS
    ADM_timeMapping map; // Store real PTS <->lav value mapping
    map.realTS=image->Pts+getEncoderDelay();
    aprintf("Pushing fn=%d Time=%"LLU"\n",frameNum,map.realTS);
   
    map.internalTS=frameNum++;
    mapper.push_back(map);
    queueOfDts.push_back(image->Pts);
#endif
    // 2-preamble
    if(false==preAmble(image))
    {
        ADM_warning("[x264] preAmble failed\n");
        return false;
    }
    //
      x264_nal_t *    nal;
      int    nbNal = 0;
      x264_picture_t    pic_out;

      out->flags = 0;
      
     
      int er=x264_encoder_encode (handle, &nal, &nbNal, pic, &pic_out);
      if(er<0)
        {
          ADM_error ("[x264] Error encoding %d\n",er);
          return false;
        }



    // 3-encode
    if(false==postAmble(out,nbNal,nal,&pic_out))
    {
        ADM_warning("[x264] postAmble failed\n");
        return false;     
    }
    return true;
}

/**
    \fn isDualPass

*/
bool         x264Encoder::isDualPass(void) 
{
    if(x264Settings.params.mode==COMPRESS_2PASS || x264Settings.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
        \fn preAmble
        \fn prepare a frame to be encoded
*/
bool  x264Encoder::preAmble (ADMImage * in)
{
      pic->img.i_csp = X264_CSP_I420;
      pic->img.i_plane = 3;
      pic->img.plane[0] = YPLANE(in);
      pic->img.plane[2] = UPLANE(in);
      pic->img.plane[1] = VPLANE(in);
      pic->img.i_stride[0] = getWidth();
      pic->img.i_stride[1] = getWidth()/2;
      pic->img.i_stride[2] = getWidth()/2;
      pic->i_type = X264_TYPE_AUTO;
      pic->i_pts = in->Pts;
  return 1;
}
/**
    \fn postAmble
    \brief update after a frame has been succesfully encoded
*/
bool x264Encoder::postAmble (ADMBitstream * out,uint32_t nbNals,x264_nal_t *nal,x264_picture_t *picout)
{
        int size = encodeNals(out->data, out->bufferSize, nal, nbNals, false);

        if (size < 0)
        {
                ADM_error("[x264] Error encoding NALs\n");
                return false;
        }
        out->len=size;
        out->pts =  picout->i_pts+getEncoderDelay();	
        switch (picout->i_type)
        {
        case X264_TYPE_IDR:
          out->flags = AVI_KEY_FRAME;
          /* First frame ?*/
#if 0
          if(!param.b_repeat_headers && _seiUserData && !pic_out.i_pts)
          {
              // Put our SEI front...
              // first a temp location...
              uint8_t tmpBuffer[size];
              memcpy(tmpBuffer,out->data,size);
              // Put back out SEI and add Size
              dout[0]=(_seiUserDataLen>>24)&0xff;
              dout[1]=(_seiUserDataLen>>16)&0xff;
              dout[2]=(_seiUserDataLen>>8)&0xff;
              dout[3]=(_seiUserDataLen>>0)&0xff;
              memcpy(dout+4,_seiUserData,_seiUserDataLen);
              memcpy(dout+4+_seiUserDataLen,tmpBuffer,size);
              size+=4+_seiUserDataLen;
              out->len = size; // update total size
          }
#endif
          break;
        case X264_TYPE_I:
          out->flags = AVI_P_FRAME;
          break;
        case X264_TYPE_P:
          out->flags = AVI_P_FRAME;
          break;
        case X264_TYPE_B:
        case X264_TYPE_BREF:
          out->flags = AVI_B_FRAME;
          break;
        default:
          ADM_error ("[x264] Unknown image type: %d\n", picout->i_type);
          //ADM_assert(0);
        }
        out->out_quantizer = picout->i_qpplus1;
        return true;
}

// EOF


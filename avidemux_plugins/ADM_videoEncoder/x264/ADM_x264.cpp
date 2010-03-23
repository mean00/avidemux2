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


#define MMSET(x) memset(&(x),0,sizeof(x))

x264_encoder x264Settings;
#if 0
=
{
    {
    COMPRESS_CQ, //COMPRESSION_MODE  mode;
    2,              // uint32_t          qz;           /// Quantizer
    1500,           //uint32_t          bitrate;      /// In kb/s 
    700,            //uint32_t          finalsize;    /// In ?
    1500,           //uint32_t          avg_bitrate;  /// avg_bitrate is in kb/s!!
    ADM_ENC_CAP_CBR+ADM_ENC_CAP_CQ+ADM_ENC_CAP_2PASS+ADM_ENC_CAP_2PASS_BR+ADM_ENC_CAP_GLOBAL+ADM_ENC_CAP_SAME
    },
            XVID_PROFILE_AS_L4, // Profile
            3, // rdMode
            3, // MotionEstimation
            0, // cqmMode
            0, // arMode
            2, // MaxBframe
            200, // MaxKeyInterval
            
            99, // nbThreads
            true, // rdOnBframe
            true, //bool:hqAcPred
            true, //bool:optimizeChrome
            true, // Trellis
    
};
#endif
/**
        \fn x264Encoder
*/
x264Encoder::x264Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    printf("[x264] Creating.\n");
    handle=NULL;
}
/**
    \fn setup
*/
bool x264Encoder::setup(void)
{
  ADM_info("x264, setting up");
  MMSET(param);

  param.i_threads = 2;
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
  
  param.i_frame_reference = 1;
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
  MKPARAM(i_frame_reference,MaxRefFrames);
  MKPARAM(i_scenecut_threshold,SceneCut);
  MKPARAM(i_keyint_min,MinIdr);
  MKPARAM(i_keyint_max,MaxIdr);
  MKPARAM(i_bframe,MaxBFrame);
  MKPARAM(i_bframe_bias,Bias);
  MKPARAM( b_bframe_pyramid,BasReference );
  MKPARAM(analyse. b_bidir_me,BidirME );
  MKPARAM( b_bframe_adaptive, Adaptative);
  MKPARAM( analyse.b_weighted_bipred, Weighted);
  MKPARAM( b_cabac , CABAC);
  MKPARAM( analyse.i_trellis, Trellis);
  MKPARAM(analyse.i_subpel_refine,PartitionDecision+1);
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
//  MKPARAM(PartitionDecision,Method);
  MKPARAM(analyse.b_transform_8x8,_8x8);
  MKPARAM(analyse.b_mixed_references,MixedRefs);
  MKPARAM(analyse.i_noise_reduction,NoiseReduction);
  
#define MES(x,y) if(zparam->x) {param.analyse.inter |=X264_ANALYSE_##y;printf("[x264] "#x" is on\n");}
  param.analyse.inter=0;
  MES(  _8x8P,  PSUB16x16);
  MES(  _8x8B,  BSUB16x16);
  MES(  _4x4,   PSUB8x8);
  MES(  _8x8I,  I8x8);
  MES(  _4x4I,  I4x4);
#endif

  param.i_log_level=X264_LOG_INFO;
 
  
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

  pic =  new    x264_picture_t;
  ADM_info("x264, setup ok\n");
  if (globalHeader)
    return createHeader ();
  return true;
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
#if 0
  x264_nal_t *
    nal;
  int
    nal_count;
  uint32_t
    offset = 0;;
  uint8_t    buffer[MAX_HEADER_X264];
  uint8_t    picParam[MAX_HEADER_X264];
  uint8_t    seqParam[MAX_HEADER_X264];
  uint8_t    sei[MAX_HEADER_X264];
  int	     picParamLen = 0, seqParamLen = 0,seiParamLen=0, len;
  int
    sz;

  // 1024 bytes should be enough
  extraData = new uint8_t[MAX_HEADER_X264];
  extraSize = 0;

  x264_t *x264=(x264_t *)_handle;
  //x264->i_frame=0; // force write_sei_params
  if (x264_encoder_headers (x264, &nal, &nal_count))
    {
      printf ("[x264] Cannot create header\n");
      return 0;
    }
  printf ("[x264] Nb nal: %d\n", nal_count);

  // Now encode them
  for (int i = 0; i < nal_count; i++)
    {
	  switch(nal[i].i_type)
	  {
	  case H264_NAL_TYPE_SEQ_PARAM:
		  	sz = x264_nal_encode (seqParam, &seqParamLen, 0, &nal[i]);
		  	break;
	  case H264_NAL_TYPE_PIC_PARAM:
		  	sz = x264_nal_encode (picParam, &picParamLen, 0, &nal[i]);
		  	break;
	  case H264_NAL_TYPE_SEI:
		  	sz = x264_nal_encode (sei, &seiParamLen, 0, &nal[i]);
		  	break;
	  default:
		  printf ("[x264] ?? type %d in nal %d\n", nal[i].i_type, i);
		  sz = x264_nal_encode (buffer, &len, 0, &nal[i]);
	 }
    if (sz <= 0)
	{
	  printf ("[x264] Cannot encode nal header %d\n", i);
	  return 0;
	}
    }
  // Now that we got all the nals encoded, time to build the avcC atom
  // Check we have everything we want
  if (!picParamLen || !seqParamLen)
    {
      printf ("[x264] Seqparam or PicParam not found\n");
      return 0;
    }



  // Fill header
  extraData[0] = 1;		// Version
  extraData[1] = seqParam[1];	//0x42; // AVCProfileIndication
  extraData[2] = seqParam[2];	//0x00; // profile_compatibility
  extraData[3] = seqParam[3];	//0x0D; // AVCLevelIndication

  extraData[4] = 0xFC + 3;	// lengthSizeMinusOne 
  extraData[5] = 0xE0 + 1;	// nonReferenceDegredationPriorityLow        

  offset = 6;



  extraData[offset] = seqParamLen >> 8;
  extraData[offset + 1] = seqParamLen & 0xff;
  offset += 2;
  memcpy (extraData + offset, seqParam, seqParamLen);
  offset += seqParamLen;

  
  extraData[offset] = 1;	// numOfPictureParameterSets
  offset++;
  extraData[offset] = (picParamLen) >> 8;
  extraData[offset + 1] = (picParamLen) & 0xff;
  offset += 2;
  memcpy (extraData + offset, picParam, picParamLen);
  offset += picParamLen;

  // Where x264 stores all its header, save it for later use
  if(seiParamLen) 
  {
	  	_seiUserDataLen=seiParamLen;
	  	_seiUserData=new uint8_t[_seiUserDataLen];
	  	memcpy(_seiUserData,sei,_seiUserDataLen);
  }

  extraSize = offset;

  ADM_assert (offset < MAX_HEADER_X264);
#endif
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
      delete	pic;
      pic = NULL;
    }
#if 0
  if(_seiUserData)
  {
	  delete [] _seiUserData;
	  _seiUserData=NULL;
  }
#endif
   
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

  uint32_t      size = 0, thisnal = 0;
  uint8_t *     dout = out->data;
  int           sizemax = 0;
#if 0
  for (uint32_t i = 0; i < nbNals; i++)
    {
      sizemax = 0xfffffff;;
      if (!param.b_repeat_headers)
    	  size += 4;
      thisnal =	x264_nal_encode (dout + size, &sizemax, param.b_repeat_headers, &nal[i]);
      if (!param.b_repeat_headers)
      {			// Need to put size (assuming nal_size=4)
    	  dout[size + 0 - 4] = (thisnal >> 24) & 0xff;
    	  dout[size + 1 - 4] = (thisnal >> 16) & 0xff;
    	  dout[size + 2 - 4] = (thisnal >> 8) & 0xff;
    	  dout[size + 3 - 4] = (thisnal >> 0) & 0xff;
      }
      size += thisnal;
    }
#endif

  out->len = size;
  out->pts =  picout->i_pts+getEncoderDelay();	
//    if(admParam.BasReference)
//    {
//  printf("%u +=%u\n",out->ptsFrame,admParam.MaxBFrame);
//      out->ptsFrame+=admParam.MaxBFrame;
//    }
  //printf("Frame :%lld \n",pic_out.i_pts);
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
      printf ("[x264] Unknown image type: %d\n", picout->i_type);
      //ADM_assert(0);
    }
    
    //
  out->out_quantizer = picout->i_qpplus1;
  return 1;
}
// EOF


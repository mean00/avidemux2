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
#define avsnprintf(...) {}
#else
#define aprintf printf
#define avsnprintf vsnprintf
#endif

#define MMSET(x) memset(&(x),0,sizeof(x))

x264_encoder x264Settings=
{
   { // General
    {
    COMPRESS_AQ, //COMPRESSION_MODE  mode;
    20,              // uint32_t          qz;           /// Quantizer
    1500,           //uint32_t          bitrate;      /// In kb/s 
    700,            //uint32_t          finalsize;    /// In ?
    1500,           //uint32_t          avg_bitrate;  /// avg_bitrate is in kb/s!!
        ADM_ENC_CAP_CBR+
        ADM_ENC_CAP_CQ+
        ADM_ENC_CAP_AQ+
        0*ADM_ENC_CAP_2PASS+
        0*ADM_ENC_CAP_2PASS_BR+
        ADM_ENC_CAP_GLOBAL+
        0*ADM_ENC_CAP_SAME
    },
    99, // Threads : auto
    },
    31, // Level
    {1,1}, // Sar width/height
    2, //uint32_t MaxRefFrames;
    100, // uint32_t MinIdr;
    500, // uint32_t MaxIdr;
    2, //uint32_t MaxBFrame;
    0, //uint32_t i_bframe_adaptative;
    0, //uint32_t i_bframe_bias;
    0, //uint32_t i_bframe_pyramid;
    0, //bool b_deblocking_filter;
    0, //int32_t i_deblocking_filter_alphac0;
    0, //int32_t i_deblocking_filter_beta;
    true, //bool cabac;
    false, //bool interlaced;
    true, //    bool b_8x8;
    false, //    bool b_i4x4;
    false, //    bool b_i8x8;
    false, //    bool b_p8x8;
    false, //    bool b_p16x16;
    false, //    bool b_b16x16;
    0, //    uint32_t weighted_pred;
    0, //    bool weighted_bipred;
    0, //    uint32_t direct_mv_pred;
    0, //    uint32_t chroma_offset;
    0, //    uint32_t me_method;
    7, //    uint32_t subpel_refine;
    false, //    bool chroma_me;
    false, //    bool mixed_references;
    1, //    uint32_t trellis;
    true, //    bool fast_pskip;
    false, //    bool dct_decimate;
    0,//    uint32_t noise_reduction;
    true,//    bool psy;
//    
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
    this->globalHeader=globalHeader;
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
        delete [] seiUserData;
        seiUserData=NULL;
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
    aprintf("x264 Incoming : %"LLU"us \n",image->Pts);
    
    // 2-preamble
    if(false==preAmble(image))
    {
        ADM_warning("[x264] preAmble failed\n");
        return false;
    }
    //
      x264_nal_t        *nal;
      int               nbNal = 0;
      x264_picture_t    pic_out;

      out->flags = 0;
      
        int er=x264_encoder_encode (handle, &nal, &nbNal, &pic, &pic_out);
        if(er<0)
        {
          ADM_error ("[x264] Error encoding %d\n",er);
          return false;
        }
        if(!nbNal)
        {
            ADM_info("[x264] Null frame\n");
            goto again;
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
    if(x264Settings.general.params.mode==COMPRESS_2PASS || x264Settings.general.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
        \fn preAmble
        \fn prepare a frame to be encoded
*/
bool  x264Encoder::preAmble (ADMImage * in)
{
    MMSET(pic);
      pic.img.i_csp = X264_CSP_I420;
      pic.img.i_plane = 3;
      pic.img.plane[0] = YPLANE(in);
      pic.img.plane[2] = UPLANE(in);
      pic.img.plane[1] = VPLANE(in);
      pic.img.i_stride[0] = getWidth();
      pic.img.i_stride[1] = getWidth()/2;
      pic.img.i_stride[2] = getWidth()/2;
      pic.i_type = X264_TYPE_AUTO;
      pic.i_pts = in->Pts;
  return true;
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
        out->dts =  picout->i_dts+getEncoderDelay();
        aprintf("encoder delay=%d, pic out dts=%d picout pts=%d\n",getEncoderDelay(),picout->i_dts,picout->i_pts);
        aprintf("pts = %"LLU", dts=%"LLU", pts+delay=%"LLU" delta=%"LLU"\n",picout->i_pts,out->dts,out->pts,
                    out->pts-out->dts);
        if(out->dts>out->pts)
        {
            ADM_warning("DTS > PTS, that can happen when there are holes in the source (%"LLU"/%"LLU")\n",
                        out->dts,out->pts);
            if(picout->i_type!=X264_TYPE_B && picout->i_type!=X264_TYPE_BREF)
            {
                ADM_warning("It is not a bframe, expect problems\n");
                ADM_warning("It is not a bframe, expect problems\n");
            }
            out->dts=out->pts;
        }
        switch (picout->i_type)
        {
        case X264_TYPE_IDR:
          out->flags = AVI_KEY_FRAME;
          /* First Idr ?*/
          if(!param.b_repeat_headers && seiUserData && firstIdr==true)
          {
              // Put our SEI front...
              // first a temp location...
              firstIdr=false;
              uint8_t *tmpBuffer=new uint8_t[size];
              memcpy(tmpBuffer,out->data,size);
              uint8_t *dout=out->data;
              // Put back out SEI and add Size
              dout[0]=(seiUserDataLen>>24)&0xff;
              dout[1]=(seiUserDataLen>>16)&0xff;
              dout[2]=(seiUserDataLen>>8)&0xff;
              dout[3]=(seiUserDataLen>>0)&0xff;
              memcpy(dout+4,seiUserData,seiUserDataLen);
              memcpy(dout+4+seiUserDataLen,tmpBuffer,size);
              size+=4+seiUserDataLen;
              out->len = size; // update total size
              delete [] tmpBuffer;
          }
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

extern bool x264_ui(x264_encoder *settings);
bool         x264Configure(void)
{
    return x264_ui(&x264Settings);
}
// EOF


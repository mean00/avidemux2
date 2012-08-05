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

bool  x264_encoder_jserialize(const char *file, const x264_encoder *key);

x264_encoder x264Settings = X264_DEFAULT_CONF;

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
    passNumber=0;
    logFile=NULL;
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
  if(logFile)
  {
        ADM_dealloc(logFile);
        logFile=NULL;
  }
}
/**
    \fn setPassAndLogFile
*/
bool         x264Encoder::setPassAndLogFile(int pass,const char *name)
{
    ADM_info("Initializing pass %d, log file =%s\n",pass,name);
    logFile=ADM_strdup(name);
    passNumber=pass;
    return true;

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
    bool gotFrame=true;
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[x264] Cannot get next image\n");
        gotFrame=false;
        //return false;
    }else
    {
        //printf("[PPPP] x264 Incoming : %"PRIu64"us \n",image->Pts);    
        // 2-preamble   
        if(false==preAmble(image))
        {
            ADM_warning("[x264] preAmble failed\n");
            return false;
        }
    }
    //
      x264_nal_t        *nal;
      int               nbNal = 0;
      x264_picture_t    pic_out;

      out->flags = 0;
      
        int er;
        if(false==gotFrame)     
        {
            ADM_info("Flushing delayed frames\n");
            er=x264_encoder_encode (handle, &nal, &nbNal, NULL, &pic_out);
            if(er<=0)
            {
                ADM_info ("End of flush\n");
                return false;
            }
        }else 
        {
            er=x264_encoder_encode (handle, &nal, &nbNal, &pic, &pic_out);
            if(er<0)
            {
              ADM_error ("[x264] Error encoding %d\n",er);
              return false;
            }
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
      pic.img.i_stride[0] = in->GetPitch(PLANAR_Y);
      pic.img.i_stride[1] = in->GetPitch(PLANAR_U);
      pic.img.i_stride[2] = in->GetPitch(PLANAR_V);
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
        aprintf("pts = %"PRIu64", dts=%"PRIu64", pts+delay=%"PRIu64" delta=%"PRIu64"\n",picout->i_pts,out->dts,out->pts,
                    out->pts-out->dts);
        if(out->dts>out->pts)
        {
            ADM_warning("DTS > PTS, that can happen when there are holes in the source (%"PRIu64"/%"PRIu64")\n",
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
        //printf("[OOOO] x264 Outgoing : %"PRIu64"us \n",out->dts);    
        out->out_quantizer = picout->i_qpplus1;
        return true;
}

extern bool x264_ui(x264_encoder *settings);
bool         x264Configure(void)
{
bool r;
    r=x264_ui(&x264Settings);
    return r;
}
// EOF


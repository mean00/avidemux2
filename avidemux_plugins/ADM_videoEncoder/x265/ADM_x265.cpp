/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 HEVC asp encoder
                             -------------------
    
    copyright            : (C) 2002/2014 by mean/gruntster
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
#include "ADM_x265.h"
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

bool  x265_settings_jserialize(const char *file, const x265_settings *key);

x265_settings x265Settings = X265_DEFAULT_CONF;

/**
        \fn x265Encoder
*/
x265Encoder::x265Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("[x265] Creating with globalHeader=%d\n",globalHeader);
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
int x265Encoder::encodeNals(uint8_t *buf, int size, x265_nal *nals, int nalCount, bool skipSei)
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
        if (skipSei && (nals[i].type == NAL_UNIT_PREFIX_SEI || nals[i].type == NAL_UNIT_SUFFIX_SEI))
                {
            seiUserDataLen = nals[i].sizeBytes;
            seiUserData = new uint8_t[seiUserDataLen];
            memcpy(seiUserData, nals[i].payload, nals[i].sizeBytes);
            continue;
        }

        memcpy(p, nals[i].payload, nals[i].sizeBytes);
        p += nals[i].sizeBytes;
    }

    return p - buf;
}

/**
        \fn createHeader
        \brief create esds header, needed for mp4/mov
*/
bool x265Encoder::createHeader (void)
{

  x265_nal *nal;
  uint32_t nalCount;

    extraDataLen = x265_encoder_headers(handle, &nal, &nalCount);
    extraData = new uint8_t[extraDataLen];
    extraDataLen = encodeNals(extraData, extraDataLen, nal, nalCount, true);

  return 1;
}
/** 
    \fn ~x265Encoder
*/
x265Encoder::~x265Encoder()
{
    ADM_info("[x265] Destroying.\n");
    if (handle)
    {
      x265_encoder_close (handle);
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
#if _WIN32
extern std::string utf8StringToAnsi(const char *utf8String);
#endif
bool         x265Encoder::setPassAndLogFile(int pass,const char *name)
{
    ADM_info("Initializing pass %d, log file =%s\n",pass,name);
#if defined(_WIN32) && !defined(X265_USE_UTF8)
    std::string ansi=utf8StringToAnsi(name);
    logFile=ADM_strdup(ansi.c_str());
#else
    logFile=ADM_strdup(name);
#endif
    ADM_info("Creating system file =<%s>\n",logFile);
    passNumber=pass;
    return true;

}
/**
    \fn encode
*/
bool         x265Encoder::encode (ADMBitstream * out)
{
    // 1 fetch a frame...
    uint32_t nb;

    // update
again:    
    bool gotFrame=true;
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[x265] Cannot get next image\n");
        gotFrame=false;
        //return false;
    }else
    {
        //printf("[PPPP] x265 Incoming : %" PRIu64"us \n",image->Pts);    
        // 2-preamble   
        if(false==preAmble(image))
        {
            ADM_warning("[x265] preAmble failed\n");
            return false;
        }
    }
    //
      x265_nal          *nal;
      uint32_t          nbNal = 0;
      x265_picture      pic_out;

      out->flags = 0;
      
        int er;
        if(false==gotFrame)     
        {
            ADM_info("Flushing delayed frames\n");
            er=x265_encoder_encode (handle, &nal, &nbNal, NULL, &pic_out);
            if(er<=0)
            {
                ADM_info ("End of flush\n");
                return false;
            }
        }else 
        {
            er=x265_encoder_encode (handle, &nal, &nbNal, &pic, &pic_out);
            if(er<0)
            {
              ADM_error ("[x265] Error encoding %d\n",er);
              return false;
            }
        }
        if(!nbNal)
        {
            ADM_info("[x265] Null frame\n");
            goto again;
        }


    // 3-encode
    if(false==postAmble(out,nbNal,nal,&pic_out))
    {
        ADM_warning("[x265] postAmble failed\n");
        return false;     
    }
    return true;
}

/**
    \fn isDualPass

*/
bool         x265Encoder::isDualPass(void) 
{
    if(x265Settings.general.params.mode==COMPRESS_2PASS || x265Settings.general.params.mode==COMPRESS_2PASS_BITRATE ) return true;
    return false;

}

/**
        \fn preAmble
        \fn prepare a frame to be encoded
*/
bool  x265Encoder::preAmble (ADMImage * in)
{
    MMSET(pic);
      pic.colorSpace = X265_CSP_I420;
      pic.planes[0] = YPLANE(in);
      pic.planes[2] = UPLANE(in);
      pic.planes[1] = VPLANE(in);
      pic.stride[0] = in->GetPitch(PLANAR_Y);
      pic.stride[1] = in->GetPitch(PLANAR_U);
      pic.stride[2] = in->GetPitch(PLANAR_V);
      pic.sliceType = X265_TYPE_AUTO;
      pic.pts = in->Pts;
      pic.bitDepth = 8;
  return true;
}
/**
    \fn postAmble
    \brief update after a frame has been succesfully encoded
*/
bool x265Encoder::postAmble (ADMBitstream * out,uint32_t nbNals,x265_nal *nal,x265_picture *picout)
{
        int size = encodeNals(out->data, out->bufferSize, nal, nbNals, false);

        if (size < 0)
        {
                ADM_error("[x265] Error encoding NALs\n");
                return false;
        }
        out->len=size;
        aprintf("--PostAmble--\n");
        // Make sure PTS & DTS > 0
        int64_t finalDts=picout->dts+(int64_t)getEncoderDelay();
        if(finalDts<0)
        {
            out->dts=0; 
            ADM_warning("Final DTS <0, fixing rounding error\n");        
        }else
        {
                out->dts =  finalDts;
        }
         int64_t finalPts=picout->pts+(int64_t)getEncoderDelay();
         if(finalPts<0)
        {
            out->pts=0; 
            ADM_warning("Final PTS <0, fixing rounding error\n");
        }else
        {
                out->pts =  finalPts;
        }
        //------
        aprintf("encoder delay=%d, pic out dts=%d picout pts=%d\n",getEncoderDelay(),picout->i_dts,picout->i_pts);
        aprintf("pts = %" PRIu64", dts=%" PRIu64", pts+delay=%" PRIu64" delta=%" PRIu64"\n",picout->i_pts,out->dts,out->pts,
                    out->pts-out->dts);
        if(out->dts>out->pts)
        {
            ADM_warning("DTS > PTS, that can happen when there are holes in the source (%" PRIu64"/%" PRIu64")\n",
                        out->dts,out->pts);
            if(picout->sliceType!=X265_TYPE_B && picout->sliceType!=X265_TYPE_BREF)
            {
                ADM_warning("It is not a bframe, expect problems\n");
                ADM_warning("It is not a bframe, expect problems\n");
            }
            out->dts=out->pts;
        }
        switch (picout->sliceType)
        {
        case X265_TYPE_IDR:
          out->flags = AVI_KEY_FRAME;
          /* First Idr ?*/
          if(!param.bRepeatHeaders && seiUserData && firstIdr==true)
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
        case X265_TYPE_I:
          out->flags = AVI_P_FRAME;
          break;
        case X265_TYPE_P:
          out->flags = AVI_P_FRAME;
          break;
        case X265_TYPE_B:
        case X265_TYPE_BREF:
          out->flags = AVI_B_FRAME;
          break;
        default:
          ADM_error ("[x265] Unknown image type: %d\n", picout->sliceType);
          //ADM_assert(0);
        }
        //printf("[OOOO] x265 Outgoing : %" PRIu64"us \n",out->dts);    
        out->out_quantizer = picout->forceqp;
        return true;
}

extern bool x265_ui(x265_settings *settings);
bool         x265Configure(void)
{
bool r;
    r=x265_ui(&x265Settings);
    return r;
}
// EOF


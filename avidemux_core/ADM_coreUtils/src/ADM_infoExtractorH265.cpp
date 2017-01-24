/***************************************************************************
                          ADM_infoextractor
                             -------------------
           - extract additionnal info from header (mp4/h263)                  
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _MSC_VER
#	include <malloc.h>
#endif

#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"

#define aprintf(...) {}
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h265_tag.h"


extern "C"
{
#include "libavcodec/parser.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/ff_spsinfo.h"

}



/**
      \fn extractH265FrameType
      \brief return frametype in flags (KEY_FRAME or 0). 
             To be used only with  mkv/mp4 nal type (i.e. no startcode)
                    but 4 bytes NALU
      
*/
uint8_t extractH265FrameType (uint32_t nalSize, uint8_t * buffer, uint32_t len,  uint32_t * flags)
{
  uint8_t *head = buffer, *tail = buffer + len;
  uint8_t stream;

  uint32_t val, hnt;
  nalSize=4;
// Check for short nalSize, i.e. size coded on 3 bytes
  {
      uint32_t length =(head[0] << 24) + (head[1] << 16) + (head[2] << 8) + (head[3]);
      if(length>len)
      {
          nalSize=3;
      }
  }
  uint32_t recovery=0xff;
  *flags=0;
  while (head + nalSize < tail)
    {

      uint32_t length =(head[0] << 16) + (head[1] << 8) + (head[2] << 0) ;
      if(nalSize==4)
          length=(length<<8)+head[3];
      if (length > len)// || length < 2)
      {
          ADM_warning ("Warning , incomplete nal (%u/%u),(%0x/%0x)\n", length, len, length, len);
          *flags = 0;
          return 0;
        }
      head += nalSize;		// Skip nal lenth
      
      stream = ((*head)>>1) & 0x3F;
      

      switch (stream)
        {
            case H265_NAL_PREFIX_SEI:
            case H265_NAL_SUFIX_SEI:
                //getRecoveryFromSei(length-1, head+1,&recovery);
                break;
            case H265_NAL_SPS:
            case H265_NAL_PPS: 
            case H265_NAL_AU_DELIMITER:
            case H265_NAL_FD:
                    break;
            case H265_NAL_IDR_W:
            case H265_NAL_IDR_N:
              *flags = AVI_KEY_FRAME;
              return 1;
              break;
            /*case NAL_NON_IDR:
              getNalType(head+1,head+length,flags,recovery);
              return 1;
              break;*/
            default:
              ADM_warning ("unknown nal ??0x%x\n", stream);
              break;
         }
      
        head+=length;
    }
  ADM_warning ("No stream\n");
  return 0;
}


/**
        \fn extractSPSInfo_mp4Header
        \brief Only works for mp4 style headers i.e. begins by 0x01
*/
bool extractSPSInfoH265_mp4Header (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
    bool r=false;
    bool closeCodec=false;

    // duplicate
    int myLen=len+FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t *myData=new uint8_t[myLen];
    memset(myData,0x2,myLen);
    memcpy(myData,data,len);

    // 1-Create parser
    AVCodecParserContext *parser=av_parser_init(AV_CODEC_ID_HEVC);
    AVCodecContext *ctx=NULL;
    AVCodec *codec=NULL;
    uint8_t *d=NULL;

    if(!parser)
    {
        ADM_error("cannot create h264 parser\n");
        goto theEnd;
    }
    ADM_info("Parser created\n");
    codec=avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if(!codec)
    {
        ADM_error("cannot create h264 codec\n");
        goto theEnd;
    }
    ADM_info("Codec created\n");
    ctx=avcodec_alloc_context3(codec);
   if (avcodec_open2(ctx, codec, NULL) < 0)
   {
        ADM_error("cannot create h264 context\n");
        goto theEnd;
    }

    ADM_info("Context created\n");
    //2- Parse, let's add SPS prefix + Filler postfix to make life easier for libavcodec parser
    ctx->extradata=myData;
    ctx->extradata_size=len;
     {
         uint8_t *outptr=NULL;
         int outsize=0;

         int used=av_parser_parse2(parser, ctx, &outptr, &outsize, d, 0, 0, 0,0);
         printf("Used bytes %d/%d (+5)\n",used,len);
         if(!used)
         {
             ADM_warning("Failed to extract SPS info\n");
           //  goto theEnd;
         }
    }
    ADM_info("Width  : %d\n",ctx->width);
    ADM_info("Height : %d\n",ctx->height);
    {
        spsinfo->width=ctx->width;
        spsinfo->height=ctx->height;
        spsinfo->fps1000=(ctx->framerate.num * 1000)/ctx->framerate.den;
        r=true;
     }
    // cleanup
theEnd:
    if(ctx)
    {
        avcodec_close(ctx);

        av_free(ctx);
    }
    if(parser)
        av_parser_close(parser);

    delete [] myData;

    return r;
}
/**
        \fn extractSPSInfo2
        \brief Same as extractSPSInfo, but using libavcodec
*/
static bool extractSPSInfoH265_lavcodec (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
    ADM_info("Incoming SPS info\n");
    mixDump(data,len);

    return    extractSPSInfoH265_mp4Header(data,len,spsinfo) ;

}
/**
 * 
 * @param data
 * @param len
 * @param spsinfo
 * @return 
 */
bool extractSPSInfoH265 (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
#define DPY(x) ADM_info(#x":%d\n",(int)spsinfo->x);
#if 1

        bool l=extractSPSInfoH265_lavcodec(data,len,spsinfo);
        if(l)
        {
            DPY(width);
            DPY(height);
            DPY(fps1000);
            DPY(hasStructInfo);
            DPY(CpbDpbToSkip);
            DPY(darNum);
            DPY(darDen);
        }else
        {
            ADM_info("Failed\n.");
        }
        return l;

#else
        bool i=extractSPSInfo_internal(data,len,spsinfo);
        DPY(width);
        DPY(height);
        DPY(fps1000);
        DPY(hasStructInfo);
        DPY(CpbDpbToSkip);
        DPY(darNum);
        DPY(darDen);
        return i;
#endif
}
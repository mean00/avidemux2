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
extern  HEVCSPS *ff_hevc_parser_get_sps(AVCodecParserContext *parser);
extern  HEVCPPS *ff_hevc_parser_get_pps(AVCodecParserContext *parser);
extern  HEVCVPS *ff_hevc_parser_get_vps(AVCodecParserContext *parser);
   

}
extern bool ADM_SPSannexBToMP4(uint32_t dataLen,uint8_t *incoming,
                                    uint32_t *outLen, uint8_t *outData);


/**
 */
class H265Parser
{
public : 
    H265Parser  (int len,uint8_t *data);
    ~H265Parser();
    bool parseAnnexB(ADM_SPSinfoH265 *spsinfo);
    bool parseMpeg4(ADM_SPSinfoH265 *spsinfo);    
    bool init();

protected:
    int myLen,originalLength;
    uint8_t *myData;
    AVCodecParserContext *parser;
    AVCodecContext *ctx;
    AVCodec *codec;
};

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

H265Parser::H265Parser  (int len,uint8_t *data)
{
    originalLength=len;
    myLen=len+FF_INPUT_BUFFER_PADDING_SIZE;
    myData=new uint8_t[myLen];
    memset(myData,0x2,myLen);
    memcpy(myData,data,len);
    parser=NULL;
    ctx=NULL;
    codec=NULL;
}
H265Parser::~H265Parser()
{
    if(myData)
    {
        delete [] myData;
        myData=NULL;
    }
    //-
    if(ctx)
    {
        avcodec_close(ctx);
        av_free(ctx);
        ctx=NULL;
    }    
    if(parser)
    {
        av_parser_close(parser);
        parser=NULL;
    }
    
}

bool H265Parser::init()
{
     parser=av_parser_init(AV_CODEC_ID_HEVC);
    if(!parser)
    {
        ADM_error("cannot create h265 parser\n");
        return false;;
    }
    ADM_info("Parser created\n");
    codec=avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if(!codec)
    {
        ADM_error("cannot create h265 codec\n");
        return false;;
    }
    ADM_info("Codec created\n");
    ctx=avcodec_alloc_context3(codec);
   if (avcodec_open2(ctx, codec, NULL) < 0)
   {
        ADM_error("cannot create h265 context\n");
        return false;;
    }
    parser->flags|=PARSER_FLAG_COMPLETE_FRAMES;
    return true;
}
/**
 * 
 * @return 
 */
bool H265Parser::parseMpeg4(ADM_SPSinfoH265 *spsinfo)
{
    uint8_t *outptr=NULL;
    int outsize=0;

    ctx->extradata=myData;
    ctx->extradata_size=myLen;
    int used=av_parser_parse2(parser, ctx, &outptr, &outsize, NULL, 0, 0, 0,0);
    printf("Used bytes %d, total = %d, outsize=%d (+5)\n",used,myLen,outsize);
    if(!used)
    {
        ADM_warning("Failed to extract SPS info\n");
        return false;
    }
    return  true;
}
/**
 * 
 * @return 
 */
bool H265Parser::parseAnnexB(ADM_SPSinfoH265 *spsinfo)
{
    
    uint8_t *start=myData;
    int   toConsume=myLen;
#if 1    
    uint8_t *p= myData+originalLength;
    *p++=0x00;
    *p++=0x00;
    *p++=0x00;
    *p++=0x01;
    *p++=21<<1;  // fake NAL_H265_CRA_NUT
    *p++=0X01;  
    *p++=0xac;  
    *p++=0xe1;      
    *p++=0X22;      
    *p++=0X22;  
#endif
    ctx->flags |= PARSER_FLAG_COMPLETE_FRAMES;
    mixDump(myData,myLen);
    while(toConsume>5)
     {
         ADM_info("Left in buffer %d\n",toConsume);
         uint8_t *outptr=NULL;
         int outsize=0;
         int used=av_parser_parse2(parser, ctx, &outptr, &outsize,
                     start, toConsume,
                     0,0,0); // pos, dts,...    
         printf("Used bytes %d, total = %d, outsize=%d (+5)\n",used,toConsume,outsize);
         if(used>0)
         {
             toConsume-=used;
             start+=used;
             continue;
         }
         break;
    }
    // Ok, let's see if we get a valid sps
   HEVCSPS *sps = ff_hevc_parser_get_sps(parser);
   HEVCVPS *vps = ff_hevc_parser_get_vps(parser);
   HEVCPPS *pps = ff_hevc_parser_get_pps(parser);
   if(sps)
   {
        printf("Coded width=%d x %d\n",sps->output_width,sps->output_height);
        spsinfo->width=sps->output_width;
        spsinfo->height=sps->output_height;
        spsinfo->fps1000=23976;
        spsinfo->sps=*sps;
        if(vps)
        {
            spsinfo->vps=*vps;
            printf("VPS timescale =%d\n",(int)vps->vps_time_scale);
            printf("VPS num unit in tick =%d\n",(int)vps->vps_num_units_in_tick);
            spsinfo->fps1000=(1000*vps->vps_time_scale)/vps->vps_num_units_in_tick;
        }
        if(pps)
            spsinfo->pps=*pps;
        return true;
   }
    return false;
}

/**
 * 
 * @param data
 * @param len
 * @param spsinfo
 * @return 
 */
bool extractSPSInfoH265 (uint8_t * data, uint32_t len, ADM_SPSinfoH265 *spsinfo)
{
    
    bool annexB=false;
    switch(data[0])
    {
    case 1: ADM_info("Mp4 \n");
            break;
    case 0: 
            annexB=true;
            ADM_info("Annex B \n");
            break;
    default:
            ADM_warning("Format not recognized\n");
            return false;
            break;            
    }        
      
    H265Parser parser(len,data);
    if(!parser.init())
    {
        ADM_info("Cannot initialize parser\n");
        return false;
    }
    bool r;
    if(!annexB)
        r=parser.parseMpeg4(spsinfo);
    else
        r=parser.parseAnnexB(spsinfo);
    return r;
}
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



extern "C"
{
#include "libavcodec/parser.h"
#include "libavcodec/hevc.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/ff_spsinfo.h"
}

extern "C"
{
extern  HEVCSPS *ff_hevc_parser_get_sps(AVCodecParserContext *parser);
extern  HEVCPPS *ff_hevc_parser_get_pps(AVCodecParserContext *parser);
extern  HEVCVPS *ff_hevc_parser_get_vps(AVCodecParserContext *parser);
   

}

#include "../include/ADM_h265_tag.h"

extern bool ADM_SPSannexBToMP4(uint32_t dataLen,uint8_t *incoming,
                                    uint32_t *outLen, uint8_t *outData);
extern bool ADM_findMpegStartCode (uint8_t * start, uint8_t * end,
                uint8_t * outstartcode, uint32_t * offset);


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

static int bitsNeeded(int v)
{
    int b=1;
    while(v)
    {
        v>>=1;
        b++;
    }
    return b;
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
   spsinfo-> num_extra_slice_header_bits=0;
   if(sps)
   {
        printf("Coded width=%d x %d\n",sps->output_width,sps->output_height);
        spsinfo->width=sps->output_width;
        spsinfo->height=sps->output_height;
        spsinfo->fps1000=23976;
        spsinfo->dependent_slice_segments_enabled_flag=0;
        spsinfo->address_coding_length=bitsNeeded(sps->ctb_width*sps->ctb_height);
        printf("VPS = %d  x %d ** %d\n",sps->ctb_width,sps->ctb_height, sps->ctb_size);
        if(vps)
        {
            printf("VPS timescale =%d\n",(int)vps->vps_time_scale);
            printf("VPS num unit in tick =%d\n",(int)vps->vps_num_units_in_tick);
            if(vps->vps_time_scale && vps->vps_num_units_in_tick)
                spsinfo->fps1000=(1000*vps->vps_time_scale)/vps->vps_num_units_in_tick;
            else
            {
                ADM_warning("No framerate information, hardcoding to 50 fps\n");
                spsinfo->fps1000=50*1000;
            }
        }
        if(pps)
        {
            spsinfo-> num_extra_slice_header_bits=pps-> num_extra_slice_header_bits;
            spsinfo->dependent_slice_segments_enabled_flag=pps->dependent_slice_segments_enabled_flag;
            
        }
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

/**
    \fn ADM_findNalu
    \brief lookup for a specific NALU in the given buffer
*/
NALU_descriptor *ADM_findNaluH265(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc)
{
    for(int i=0;i<maxNalu;i++)
    {
            if(((desc[i].nalu>>1)&0x3f) == (nalu&0x3f))
                return desc+i;
    }
    return NULL;
}

/**
    \fn ADM_splitNalu
    \brief split a nalu annexb size into a list of nalu descriptor
*/
int ADM_splitNaluH265(uint8_t *start, uint8_t *end, uint32_t maxNalu,NALU_descriptor *desc)
{
bool first=true;
uint8_t *head=start;
uint32_t offset;
uint8_t startCode,oldStartCode=0xff;
int index=0;
      while(true==SearchStartCode(head,end,&startCode,&offset))
      {
            if(true==first)
            {
                head+=offset;
                first=false;
                oldStartCode=startCode;
                continue;
            }
        if(index>=maxNalu) return 0;
        desc[index].start=head;
        desc[index].size=offset-5; // FOR h265 assume long start code
        desc[index].nalu=oldStartCode;
        index++;
        head+=offset;
        oldStartCode=startCode;
      }
    // leftover
    desc[index].start=head;
    desc[index].size=(uint32_t)(end-head);
    desc[index].nalu=oldStartCode;
    index++;
    return index;
}
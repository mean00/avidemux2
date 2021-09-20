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
#include "libavcodec/hevc_ps.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/ff_spsinfo.h"
#include "libavutil/mem.h"
}

extern "C"
{
extern const HEVCSPS *ff_hevc_parser_get_sps(AVCodecParserContext *parser);
extern const HEVCPPS *ff_hevc_parser_get_pps(AVCodecParserContext *parser);
extern const HEVCVPS *ff_hevc_parser_get_vps(AVCodecParserContext *parser);
}

#include "../include/ADM_h265_tag.h"

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
#define NAL_H265_CRA_NUT_LENGTH 10
    myLen=len+AV_INPUT_BUFFER_PADDING_SIZE+NAL_H265_CRA_NUT_LENGTH;
    myData=new uint8_t[myLen];
    memset(myData,0,myLen);
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
 *  \fn bitsNeeded
 */
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
 *  \fn spsInfoFromParserContext
 */
static bool spsInfoFromParserContext(AVCodecParserContext *parser, ADM_SPSinfoH265 *spsinfo)
{
    // Ok, let's see if we get a valid sps
    const HEVCSPS *sps = ff_hevc_parser_get_sps(parser);
    const HEVCVPS *vps = ff_hevc_parser_get_vps(parser);
    const HEVCPPS *pps = ff_hevc_parser_get_pps(parser);
    spsinfo-> num_extra_slice_header_bits=0;
    spsinfo->output_flag_present_flag=false;
    if(sps)
    {
        const HEVCWindow *ow=&sps->output_window;
        printf("Coded dimensions = %d x %d\n",sps->width-ow->left_offset-ow->right_offset,sps->height-ow->top_offset-ow->bottom_offset);
        spsinfo->width=sps->width-ow->left_offset-ow->right_offset;
        spsinfo->height=sps->height-ow->top_offset-ow->bottom_offset;
        spsinfo->fps1000=23976;
        spsinfo->log2_max_poc_lsb=sps->log2_max_poc_lsb;
        spsinfo->separate_colour_plane_flag=sps->separate_colour_plane_flag;
        spsinfo->dependent_slice_segments_enabled_flag=0;
        spsinfo->address_coding_length=bitsNeeded(sps->ctb_width*sps->ctb_height);
        printf("VPS = %d  x %d ** %d\n",sps->ctb_width,sps->ctb_height, sps->ctb_size);
        uint32_t timeBaseNum=0;
        uint32_t timeBaseDen=0;
        if(vps && vps->vps_timing_info_present_flag)
        {
            printf("VPS timescale = %u\n",vps->vps_time_scale);
            printf("VPS num unit in tick = %u\n",vps->vps_num_units_in_tick);
            timeBaseNum=vps->vps_num_units_in_tick;
            timeBaseDen=vps->vps_time_scale;
        }else if(sps->vui.vui_timing_info_present_flag)
        {
            printf("VUI timescale = %u\n",sps->vui.vui_time_scale);
            printf("VUI num unit in tick = %u\n",sps->vui.vui_num_units_in_tick);
            timeBaseNum=sps->vui.vui_num_units_in_tick;
            timeBaseDen=sps->vui.vui_time_scale;
        }
        if(!timeBaseNum || !timeBaseDen)
        {
            ADM_warning("No framerate information, hardcoding to 50 fps\n");
            spsinfo->fps1000=50*1000;
        }else
        {
            double f=1000.;
            f*=timeBaseDen;
            f/=timeBaseNum;
            f+=0.49;
            spsinfo->fps1000=(int)f;
        }
        if(pps)
        {
            spsinfo-> num_extra_slice_header_bits=pps-> num_extra_slice_header_bits;
            spsinfo->dependent_slice_segments_enabled_flag=pps->dependent_slice_segments_enabled_flag;
            spsinfo->output_flag_present_flag=pps->output_flag_present_flag;
        }
        if(sps->vui.frame_field_info_present_flag)
            spsinfo->field_info_present=true;
        else
            printf("No field info present\n");
        return true;
    }
    return false;
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
    // no use to evaluate the return value, it will be always 0
    av_parser_parse2(parser, ctx, &outptr, &outsize, NULL, 0, 0, 0,0);
    return spsInfoFromParserContext(parser,spsinfo);
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
    //mixDump(myData,myLen);
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
    return spsInfoFromParserContext(parser,spsinfo);
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
    \fn ADM_findNaluH265
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
    \fn writeBE32
*/
static void writeBE32(uint8_t *p, uint32_t size)
{
    p[0]=size>>24;
    p[1]=(size>>16)&0xff;
    p[2]=(size>>8)&0xff;
    p[3]=(size>>0)&0xff;
}

/**
    \fn ADM_convertFromAnnexBToMP4H265
    \brief convert annexB startcode (00 00 00 0 xx) to NALU
*/
int ADM_convertFromAnnexBToMP4H265(uint8_t *inData, uint32_t inSize, uint8_t *outData, uint32_t outMaxSize)
{
    uint8_t *tgt=outData;
    NALU_descriptor desc[MAX_NALU_PER_CHUNK+1];
    int nbNalu=ADM_splitNalu(inData,inData+inSize,MAX_NALU_PER_CHUNK,desc);
    const int nalHeaderSize=4;
    int outputSize=0;

    for(int i=0;i<nbNalu;i++)
    {
        NALU_descriptor *d=desc+i;
        aprintf("%d/%d : Nalu :0x%x size=%d\n",i,nbNalu,d->nalu,d->size);
        switch((d->nalu>>1)&0x3f)
        {
            case NAL_H265_FD_NUT:
            case NAL_H265_AUD:
                break;
            default:
                writeBE32(tgt,1+d->size);
                tgt[nalHeaderSize]=d->nalu;
                memcpy(tgt+1+nalHeaderSize,d->start,d->size);
                tgt+=d->size+1+nalHeaderSize;
                break;
        }
        outputSize=tgt-outData;
        ADM_assert(outputSize<outMaxSize);
    }
    return outputSize;
}

/**
    \fn nalTypeToString
*/
static const char *nalTypeToString(int type)
{
    int nb=sizeof(nalDesc)/sizeof(NAL_DESC);
    for(int i=0; i<nb; i++)
    {
        if(nalDesc[i].value == type)
            return nalDesc[i].name;
    }
    return "Unknown";
}

/**
    \fn decodeSliceHeaderH265
*/
static bool decodeSliceHeaderH265(uint8_t *head, uint8_t *tail, uint32_t *flags, ADM_SPSinfoH265 *info, int *poc)
{
    if(head+2>=tail)
        return false;
    uint8_t nal=(*head>>1) & 0x3F;
    head+=2; // skip NALU type, layer ID and temporal ID
    switch(nal)
    {
        case NAL_H265_TRAIL_N:
        case NAL_H265_TRAIL_R:
        case NAL_H265_TSA_N:
        case NAL_H265_TSA_R:
        case NAL_H265_STSA_N:
        case NAL_H265_STSA_R:
        case NAL_H265_BLA_W_LP:
        case NAL_H265_BLA_W_RADL:
        case NAL_H265_BLA_N_LP:
        case NAL_H265_IDR_W_RADL:
        case NAL_H265_IDR_N_LP:
        case NAL_H265_CRA_NUT:
        case NAL_H265_RADL_N:
        case NAL_H265_RADL_R:
        case NAL_H265_RASL_N:
        case NAL_H265_RASL_R:
            break;
        default:
            ADM_warning("Unsupported NAL type %d (%s)\n",nal,nalTypeToString(nal));
            return false;
    }
    uint8_t *out=(uint8_t *)malloc(tail-head+AV_INPUT_BUFFER_PADDING_SIZE);
    if(!out) return false;
    memset(out,0,tail-head+AV_INPUT_BUFFER_PADDING_SIZE);
    int size=ADM_unescapeH264(tail-head,head,out);
    getBits bits(size,out);
    int firstSliceInPic=bits.get(1);
    bool keyframe = (nal >= NAL_H265_BLA_W_LP) && (nal <= NAL_H265_IRAP_VCL23);
    if(keyframe)
        bits.get(1); // no_output_of_prior_pics_flag
    bits.getUEG(); // PPS id
    if(!firstSliceInPic)
    {
        if(info->dependent_slice_segments_enabled_flag && bits.get(1))
        {
            ADM_warning("Dependent slice segments not handled.\n");
            free(out);
            return false;
        }
        bits.get(info->address_coding_length); // slice segment address
    }
    for(int i=0; i < info->num_extra_slice_header_bits; i++)
        bits.skip(1);
    int sliceType=bits.getUEG();
    switch(sliceType)
    {
        case 0:
            *flags=AVI_B_FRAME;
            break;
        case 1:
            *flags=AVI_P_FRAME;
            break;
        case 2:
            *flags = keyframe ? AVI_KEY_FRAME : AVI_P_FRAME; // non-keyframe intra
            break;
        default:
            ADM_warning("Unknown slice type %d\n",sliceType);
            free(out);
            return false;
    }
    if(*flags!=AVI_KEY_FRAME && keyframe)
    {
        ADM_warning("Slice type mismatch, NAL says keyframe, header says %s\n",(*flags==AVI_B_FRAME)? "B" : "P");
    }
    if(info->output_flag_present_flag)
        bits.get(1); // pic_output_flag
    if(info->separate_colour_plane_flag)
        bits.get(2); // colour_plane_id
    if((nal == NAL_H265_IDR_W_RADL) || (nal == NAL_H265_IDR_N_LP))
    {
        if(*flags!=AVI_KEY_FRAME)
        {
            ADM_warning("Slice type mismatch, NAL says IDR, header says %s\n",(*flags==AVI_B_FRAME)? "B" : "P");
            free(out);
            return false;
        }else
        {
            *flags |= AVI_IDR_FRAME;
        }
        *poc=0;
        free(out);
        return true;
    }
    int pocLsb = bits.get(info->log2_max_poc_lsb);
    const int maxPocLsb = 1 << info->log2_max_poc_lsb;

    int lastPoc = 0;
    if(*poc > INT_MIN) lastPoc = *poc;
    int lastPocLsb = lastPoc % maxPocLsb;
    int lastPocMsb = lastPoc - lastPocLsb;
    int pocMsb = lastPocMsb;

    if(pocLsb < lastPocLsb && lastPocLsb-pocLsb >= maxPocLsb/2)
        pocMsb = lastPocMsb + maxPocLsb;
    else if(pocLsb > lastPocLsb && pocLsb-lastPocLsb > maxPocLsb/2)
        pocMsb = lastPocMsb - maxPocLsb;

    if(nal == NAL_H265_BLA_W_LP || nal == NAL_H265_BLA_W_RADL || nal == NAL_H265_BLA_N_LP)
        pocMsb = 0;
    *poc = pocMsb + pocLsb;
    free(out);
    return true;
}

/**
 *  \fn extractH265FrameType
 *  \brief Parse access unit using given NALU length size nalSize (0: autodetect), find a slice and decode the header.
 *         The caller must set *poc to previous POC if applicable, else to INT_MIN
 */
bool extractH265FrameType(uint8_t *buffer, uint32_t len, uint32_t nalSize, ADM_SPSinfoH265 *spsinfo, uint32_t *flags, int *poc)
{
    if(!spsinfo || !flags || !poc)
        return false;

    uint8_t *head = buffer;
    uint8_t *tail = buffer + len;
    uint8_t nalType;

    uint32_t i, length = 0;
    if(!nalSize || nalSize > 4)
    { // Try to detect number of bytes used to code NAL length. Shaky.
        nalSize = 4;
        for(i = 0; i < nalSize; i++)
        {
            length = (length << 8) + head[i];
            if(i && length > len)
            {
                nalSize = i;
                break;
            }
        }
    }
    *flags = 0;

    while(head + nalSize < tail)
    {
        length = 0;
        for(int i=0; i < nalSize; i++)
            length = (length << 8) + head[i];
        if(!length)
        {
            ADM_warning("Zero length NAL unit?\n");
            return false;
        }
        if(length > len)
        {
            ADM_warning("Incomplete NAL unit: need %u, got %u\n",length,len);
            return false;
        }
        head += nalSize;
        len = (len > nalSize)? len - nalSize : 0;
        if(*(head) & 0x80)
        {
            ADM_warning("Invalid NAL header, skipping.\n");
            head += length;
            len = (len > length)? len - length : 0;
            continue;
        }
        nalType = (*(head)>>1) & 0x3F;
        switch(nalType)
        {
            case NAL_H265_TRAIL_N:
            case NAL_H265_TRAIL_R:
            case NAL_H265_TSA_N:
            case NAL_H265_TSA_R:
            case NAL_H265_STSA_N:
            case NAL_H265_STSA_R:
            case NAL_H265_BLA_W_LP:
            case NAL_H265_BLA_W_RADL:
            case NAL_H265_BLA_N_LP:
            case NAL_H265_IDR_W_RADL:
            case NAL_H265_IDR_N_LP:
            case NAL_H265_CRA_NUT:
            case NAL_H265_RADL_N:
            case NAL_H265_RADL_R:
            case NAL_H265_RASL_N:
            case NAL_H265_RASL_R:
            {
#define SLICE_HEADER_SAMPLE_SIZE 32
                if(length > SLICE_HEADER_SAMPLE_SIZE)
                    length = SLICE_HEADER_SAMPLE_SIZE;
                return decodeSliceHeaderH265(head, head+length, flags, spsinfo, poc);
            }
            default:
                ADM_info("Skipping NALU of type %d (%s)\n",nalType,nalTypeToString(nalType));
                break;
        }
        head+=length;
        len = (len > length)? len - length : 0;
    }
    ADM_warning("No picture slice found in the buffer.\n");
    return false;
}

/**
 *  \fn extractH265FrameType_startCode
 *  \brief Parse access unit in AnnexB type stream, find a slice and decode the header
 *         The caller must set *poc to previous POC if applicable, else to a negative value
 */
bool extractH265FrameType_startCode(uint8_t *buffer, uint32_t len, ADM_SPSinfoH265 *spsinfo, uint32_t *flags, int *poc)
{
    if(!spsinfo || !flags || !poc)
        return false;

    uint8_t *head = buffer, *tail = buffer + len;
    uint32_t hnt = 0xFFFFFFFF;
    int nalType = -1, counter = 0, length = 0;
    bool last = false;

    *flags = 0;

    while(head + 2 < tail)
    {
        // Search startcode
        hnt = (hnt << 8) + head[0];
        if((hnt & 0xFFFFFF) != 1)
        {
            head++;
            if(head + 2 < tail)
                continue;
            if(!counter) break;
            last = true;
            length = head - buffer + 2;
        }
        int prevNaluType = -1;
        if(!last)
        {
            head++;
            counter++;
            if(counter > 1)
                length = head - buffer - 3; // 3 bytes start code length no matter zero-prefixed or not
            prevNaluType = (*head>>1) & 0x3F;
            if(!length)
            {
                buffer = head;
                nalType = prevNaluType;
                continue;
            }
        }
        switch(nalType)
        {
            case NAL_H265_TRAIL_N:
            case NAL_H265_TRAIL_R:
            case NAL_H265_TSA_N:
            case NAL_H265_TSA_R:
            case NAL_H265_STSA_N:
            case NAL_H265_STSA_R:
            case NAL_H265_BLA_W_LP:
            case NAL_H265_BLA_W_RADL:
            case NAL_H265_BLA_N_LP:
            case NAL_H265_IDR_W_RADL:
            case NAL_H265_IDR_N_LP:
            case NAL_H265_CRA_NUT:
            case NAL_H265_RADL_N:
            case NAL_H265_RADL_R:
            case NAL_H265_RASL_N:
            case NAL_H265_RASL_R:
            {
                if(length > SLICE_HEADER_SAMPLE_SIZE)
                    length = SLICE_HEADER_SAMPLE_SIZE;
                ADM_info("Trying to decode slice header, NALU %d (%s)\n",nalType,nalTypeToString(nalType));
                return decodeSliceHeaderH265(buffer, buffer+length, flags, spsinfo, poc);
            }
            default:
                ADM_info("Skipping NALU of type %d (%s)\n",nalType,nalTypeToString(nalType));
                break;
        }
        buffer = head;
        nalType = prevNaluType;
    }
    ADM_warning("No picture slice found in the buffer.\n");
    return false;
}

/**
 *  \fn ADM_getNalSizeH265
 *  \brief extract NALU length size from hvcC header
 */
uint32_t ADM_getNalSizeH265(uint8_t *extra, uint32_t len)
{
    if(len < 24)
    {
        ADM_warning("Invalid HEVC extradata length %u\n",len);
        return 0;
    }
    if(extra[0] != 1)
    {
        ADM_warning("Invalid HEVC extradata.\n");
        return 0;
    }
    return (extra[21] & 3) + 1;
}
// EOF

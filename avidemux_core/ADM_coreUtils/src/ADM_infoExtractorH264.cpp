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

extern "C"
{
#include "libavcodec/parser.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/ff_spsinfo.h"
extern int ff_h264_info(AVCodecParserContext *parser,ffSpsInfo *ndo);
}

#include "ADM_Video.h"

#include "fourcc.h"
//#include "ADM_mp4.h"

#define aprintf(...) {}
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"


//#define ANNEX_B_DEBUG

#if defined(ANNEX_B_DEBUG)
#define aprintf ADM_info
#define check isNalValid
#else
#define aprintf(...) {}
#define check(...) {}
#endif

#define MAX_NALU_PER_CHUNK 20

bool ADM_findMpegStartCode (uint8_t * start, uint8_t * end,
			    uint8_t * outstartcode, uint32_t * offset);
/**
    \fn unescapeH264
    \brief Remove escape stuff

*/
uint32_t ADM_unescapeH264 (uint32_t len, uint8_t * in, uint8_t * out)
{
  uint32_t outlen = 0;
  uint8_t *tail = in + len;
  if (len < 3)
    return 0;
  while (in < tail - 3)
    {
      if (!in[0] && !in[1] && in[2] == 3)
	{
	  out[0] = 0;
	  out[1] = 0;
	  out += 2;
	  outlen += 2;
	  in += 3;
	}
      *out++ = *in++;
      outlen++;
    }
  // copy last bytes
  uint32_t left = tail - in;
  memcpy (out, in, left);
  outlen += left;
  return outlen;

}
/**
    \fn hrd
    \brief decode hdr_parameters
*/
static int hrd(getBits &bits)
{
    //ADM_warning("hdr not implemented\n");
    int count = bits.getUEG(); 
    
	bits.get(4);	// bitRateScale
    bits.get(4);	// scale

    for(int i = 0; i <= count; i++)
    {
        bits.getUEG();	// bitrate
        bits.getUEG();  // sizeMinus1
        bits.get(1);	// cbrFlag
    }

    bits.get(5);	// initialRemovalDelay

    int removalDelay=bits.get(5);
    int outputDelay=bits.get(5);

    bits.get(5);	// timeOffset

    return removalDelay+outputDelay+2 ;
}
/**
        \fn extractVUIInfo
*/
static uint8_t  extractVUIInfo (getBits &bits, ADM_SPSInfo *spsinfo)
{
  if (bits.get(1))
    {
      unsigned int aspect_ratio_information = bits.get( 8);

      if (aspect_ratio_information == 255)
	{
	  spsinfo->darNum = bits.get( 16);
	  spsinfo->darDen = bits.get( 16);
	}
      else if (aspect_ratio_information <
	       sizeof (pixel_aspect) / sizeof (*pixel_aspect))
	{
	  spsinfo->darNum = pixel_aspect[aspect_ratio_information].num;
	  spsinfo->darDen = pixel_aspect[aspect_ratio_information].den;
	}
    }

  if (bits.get(1))		// overscan
    bits.get(1);

  if (bits.get(1))		// vsp_color
    {
      bits.get( 4);

      if (bits.get(1))
	{
        bits.get( 8);
        bits.get( 8);
        bits.get( 8);
	}
    }

  if (bits.get(1))		// chroma
    {
      bits.getUEG();
      bits.getUEG();
    }

  if (bits.get(1))		// timing
    {
      uint32_t timeinc_unit = bits.get( 32);
      uint32_t timeinc_resolution = bits.get( 32);
      uint32_t fixed_fps = bits.get(1);
      ADM_info("Time unit =%d/%d\n",(int)timeinc_unit,(int)timeinc_resolution);
      if (timeinc_unit > 0 && timeinc_resolution > 0)
        spsinfo->fps1000 =  (uint32_t) (((float) timeinc_resolution / (float) timeinc_unit) *    1000);
/* No!
      if (fixed_fps)
	*fps1000 /= 2;
*/
    }
    uint32_t f=0;
    spsinfo->CpbDpbToSkip=0;
    if(bits.get(1)) // nal_hrd_param
    {
        f++;
        spsinfo->CpbDpbToSkip+=hrd(bits);
    }
    if(bits.get(1)) // vcl_hrd_param
    {
        f++;
        spsinfo->CpbDpbToSkip+=hrd(bits);
    }
    if(f) bits.get(1); // low delay flag
    
    
    spsinfo->hasStructInfo=bits.get(1) | spsinfo->CpbDpbToSkip; 
    // Has struct info in SEI, see D2.2

  return 1;
}
/**
        \fn decodeOnMatrix
        \brief borrowed from ffmpeg
*/
bool decodeOneMatrix(int count,getBits &bits)
{
        if(!bits.get(1)) 
        {
                return true;
        }
        
        int i, last = 8, next = 8;
        for(i=0;i<count;i++)
        {
         if(next)
            next = (last + bits.getUEG()) & 0xff;
         if(!i && !next)
         { /* matrix not written, we use the preset one */
            return true;
         }
         //last = factors[scan[i]] = next ? next : last;
    }
    return true;

}
/**
        \fn decodeScalingMatrices
*/
bool          decodeScalingMatrices(getBits &bits)
{
        decodeOneMatrix(16,bits);
        decodeOneMatrix(16,bits);
        decodeOneMatrix(16,bits);
        decodeOneMatrix(16,bits);
        decodeOneMatrix(16,bits);
        decodeOneMatrix(16,bits);
        decodeOneMatrix(64,bits);
        decodeOneMatrix(64,bits);
        return true;
}

/**
    \fn extractSPSInfo
    \brief Extract info from H264 SPS
    See 7.3.2.1 of 14496-10
*/
uint8_t extractSPSInfo_internal (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
   
  uint32_t profile, constraint, level, pic_order_cnt_type, w, h, mbh,
    frame_mbs_only;
  uint32_t frame_cropping_flag;
  uint32_t chroma_format_idc = 1; // this defaults to 1 when it's missing
  uint32_t separate_colour_plane_flag = 0;
  uint32_t chroma_array_type = 0;
  uint8_t *buf = new uint8_t[len];
  uint32_t outlen;
  uint32_t id, dum;

    ADM_assert(spsinfo);
    memset(spsinfo,0,sizeof(*spsinfo));


  outlen = ADM_unescapeH264 (len, data, buf);
  getBits bits(outlen,buf);
  delete [] buf;

  profile = bits.get(8);
  constraint = bits.get( 8) >> 5;
  level = bits.get( 8);
  id = bits.getUEG();	// Seq parameter set id           
  printf ("[H264]Profile : %u, Level :%u, SPSid:%u\n", profile, level, id);
  if (profile >= 100)		// ?? Borrowed from H264.C/FFMPEG
    {
      printf ("[H264]Warning : High profile\n");
      chroma_format_idc = bits.getUEG();
      if (chroma_format_idc == 3)	//chroma_format_idc
        separate_colour_plane_flag = bits.get(1);		//residual_color_transform_flag
      bits.getUEG();	//bit_depth_luma_minus8
      bits.getUEG();	//bit_depth_chroma_minus8
      bits.get(1);		// Transform bypass
      if (bits.get(1))	// Scaling matrix
	  {
	    printf ("[H264] Scaling matrix present\n");
          decodeScalingMatrices(bits);
	  }
    }

	if ( separate_colour_plane_flag == 0 )
		chroma_array_type = chroma_format_idc;

  dum = bits.getUEG();	// log2_max_frame_num_minus4
  printf ("[H264]Log2maxFrame-4:%u\n", dum);
  pic_order_cnt_type = bits.getUEG();
  printf ("[H264]Pic Order Cnt Type:%u\n", pic_order_cnt_type);
  if (!pic_order_cnt_type)	// pic_order_cnt_type
    {
      dum = bits.getUEG();	//log2_max_pic_order_cnt_lsb_minus4
      printf ("[H264]Log2maxPix-4:%u\n", dum);
    }
  else
    {
      if (pic_order_cnt_type == 1)
	{
	  bits.get(1);	//delta_pic_order_always_zero_flag
	  bits.getSEG();	//offset_for_non_ref_pic
	  bits.getSEG();	// offset_for_top_to_bottom_field
	  int i = bits.getUEG() ;	//num_ref_frames_in_pic_order_cnt_cycle

	  for (int j = 0; j < i; j++)
	    {
	      bits.getSEG();
	    }
	}
      else if (pic_order_cnt_type != 2)
	{
	  printf ("Error in SPS\n");
	  return 0;
	}
    }
  dum = bits.getUEG();	//num_ref_frames
  printf ("[H264] # of ref frames : %u\n", dum);
  bits.get(1);		// gaps_in_frame_num_value_allowed_flag
  w = bits.getUEG() + 1;	//pic_width_in_mbs_minus1

  mbh = bits.getUEG() + 1;
  frame_mbs_only = bits.get(1);
  h = (2 - frame_mbs_only) * mbh;	//pic_height_in_mbs_minus1

  printf ("[H264] Width in mb -1  :%d\n", w);
  printf ("[H264] Height in mb -1 :%d\n", h);

  spsinfo->width = w * 16;
  spsinfo->height = h * 16;

  if (!frame_mbs_only)
    bits.get(1);

  bits.get(1);

	frame_cropping_flag = bits.get(1);
 	if (frame_cropping_flag)
	{
		// The tests could probably be done more simply but the following is per the spec.
		uint32_t cl, cr, ct, cb;
		int cux = 1; // x units
		int cuy = 2 - frame_mbs_only; // y units
		if ( chroma_array_type > 0 ) {
			switch( chroma_format_idc ) {
			case 1:
				cux = 2;
				cuy = 2 * ( 2 - frame_mbs_only );
				break;
			case 2:
				cux = 2;
				cuy = 1 * ( 2 - frame_mbs_only );
				break;
			case 3:
				cux = 1; 
				cuy = 1 * ( 2 - frame_mbs_only );
				break;
			}
		}
		cl = bits.getUEG() * cux;
		cr = bits.getUEG() * cux;
		ct = bits.getUEG() * cuy;
		cb = bits.getUEG() * cuy;
		spsinfo->width -= cl; // reduce dims based on crop values
		spsinfo->width -= cr;
		spsinfo->height -= ct;
		spsinfo->height -= cb;
		printf ("[H264] Has cropping of l:%d  r:%d  t:%d  b:%d\n", cl, cr, ct, cb);
    }

  if (bits.get(1))
    {
      extractVUIInfo (bits, spsinfo);
//      printf ("[H264] Fps %" LU ", a.r. %" LU ",%" LU "\n", spsinfo->fps1000, spsinfo->darNum,  spsinfo->darDen);
    }
  else
    {
      printf ("[H264] Unknown FPS, setting 25\n");
      spsinfo->fps1000 = 25000;
    }
  return 1;
}
/**
    \fn getRecoveryFromSei
    \brief We dont unescape here, very unlikely needed as we only decode recovery which is small
*/
static bool getRecoveryFromSei(uint32_t nalSize, uint8_t *org,uint32_t *recoveryLength)
{
    
    uint8_t *payloadBuffer=(uint8_t *)malloc(nalSize+16);
    uint8_t *payload=payloadBuffer;
    bool r=false;
    nalSize=ADM_unescapeH264(nalSize,org,payload);
    uint8_t *tail=payload+nalSize;
    *recoveryLength=16;
    while( payload<tail)
    {
                uint32_t sei_type=0,sei_size=0;
                while(payload[0]==0xff) {sei_type+=0xff;payload++;};
                sei_type+=payload[0];payload++;
                while(payload[0]==0xff) {sei_size+=0xff;payload++;};
                sei_size+=payload[0];payload++;
                aprintf("  [SEI] Type : 0x%x size:%d\n",sei_type,sei_size);
                if(payload+sei_size>tail) break;
                switch(sei_type) // Recovery point
                {
                       case 6:
                        {
                            getBits bits(sei_size,payload);
                            payload+=sei_size;
                            *recoveryLength=bits.getUEG();
                            r=true;
                            break;
                        }
                        default:
                            payload+=sei_size;
                            break;
                }
    }
    free(payloadBuffer);
    return r;
}
/**
    \fn getNalType
    \brief Return the slice type. The stream is escaped by the function. If recovery==0 
            I as considered IDR else as P.
*/
static bool getNalType (uint8_t * head, uint8_t * tail, uint32_t * flags,int recovery)
{
    uint8_t *out=(uint8_t *)malloc(tail-head);
    int size=ADM_unescapeH264(tail-head,head,out);
   
    getBits bits(size,out);
    uint32_t sliceType;
    *flags = 0;
  
    bits.getUEG();               // first mb in slice
    sliceType = bits.getUEG31(); // get_ue_golomb_31??
    if (sliceType > 9)
    {
      ADM_warning ("Weird Slice %d\n", sliceType);
      free(out);
      return false;
    }
    if (sliceType > 4)
        sliceType -= 5;

    switch(sliceType)
    {
        case 3:
        case 0: *flags=  AVI_P_FRAME;break;
        case 1: *flags = AVI_B_FRAME;break;
        case 2: case 4:
                if(!recovery) *flags=AVI_KEY_FRAME;
                    else      *flags=AVI_P_FRAME;
                break;
        
    }

      free(out);
    return true;
}

/**
      \fn extractH264FrameType
      \brief return frametype in flags (KEY_FRAME or 0). 
             To be used only with  mkv/mp4 nal type (i.e. no startcode)
                    but 4 bytes NALU
      
*/
uint8_t extractH264FrameType (uint32_t nalSize, uint8_t * buffer, uint32_t len,  uint32_t * flags)
{
  uint8_t *head = buffer, *tail = buffer + len;
  uint8_t stream;

  uint32_t val, hnt;
  nalSize=4;
// Check for short nalSize, i.e. size coded on 3 bytes
  {
      uint32_t length =(head[0] << 24) + (head[1] << 16) + (head[2] << 8) + (head[3]);
      if(length>len) nalSize=3;      
  }
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
      stream = *(head) & 0x1F;
      uint32_t recovery;

      switch (stream)
        {
            case NAL_SEI:
                getRecoveryFromSei(length-1, head+1,&recovery);
                break;
            case NAL_SPS:
            case NAL_PPS: 
            case NAL_AU_DELIMITER:
            case NAL_FILLER:
                    break;
            case NAL_IDR:
              *flags = AVI_KEY_FRAME;
              return 1;
              break;
            case NAL_NON_IDR:
              getNalType(head+1,head+length,flags,recovery);
              return 1;
              break;
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
      \fn extractH264FrameType_startCode
      \brief return frametype in flags (KEY_FRAME or 0). 
      To be used only with  avi / mpeg TS nal type 
        (i.e. with startcode 00 00 00 01)     
*/
uint8_t extractH264FrameType_startCode(uint32_t nalSize, uint8_t * buffer,uint32_t len, uint32_t * flags)
{
  uint8_t *head = buffer, *tail = buffer + len;
  uint8_t stream;
  uint32_t val, hnt;

// FIXME :  no startcode only !

  while (head + 4 < tail)
    {
      // Search startcode
      hnt = (head[0] << 24) + (head[1] << 16) + (head[2] << 8) + (head[3]);
      head += 4;
      while ((hnt != 1) && head < tail)
        {

          hnt <<= 8;
          val = *head++;
          hnt += val;
        }
      if (head >= tail)
	break;
      stream = *(head++) & 0x1f;
      switch (stream)
	{
	case NAL_IDR:
	  *flags = AVI_KEY_FRAME;
	  // printf("IDR\n");
	  return 1;
	  break;
	case NAL_NON_IDR:
	   getNalType (head,tail, flags,16); // No recovery here
	  return 1;
	  break;
    case NAL_SPS:case NAL_PPS: case NAL_FILLER: case NAL_AU_DELIMITER: break;
	default:
	  ADM_warning ("??0x%x\n", stream);
	  continue;
	}
    }
  printf ("No stream\n");
  return 0;
}
/**
        \fn extractSPSInfo2
        \brief Same as extractSPSInfo, but using libavcodec
*/
uint8_t extractSPSInfo_lavcodec (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
    static const uint8_t sig[5]={0,0,0,1,0};
    uint32_t myLen=len+5+5+FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t *myData = new uint8_t[myLen];
    bool r=false;

    memset(myData,myLen,1);
    mixDump(data,len);
    // put dummy SPS tag + dummy filler at the end
    // so that lavcodec can split it
    memcpy(myData,sig,5);
    memcpy(myData+5,data,len);
    memcpy(myData+5+len,sig,5);
    myData[4]=NAL_SPS;
    myData[5+len+4]=NAL_FILLER;
    
    
    // 1-Create parser
    AVCodecParserContext *parser=av_parser_init(CODEC_ID_H264);
    AVCodecContext *ctx=NULL; 
    AVCodec *codec=NULL;
    uint8_t *d=NULL;
  
    if(!parser)
    {
        ADM_error("cannot create h264 parser\n");
        goto theEnd;
    }
    ADM_info("Parser created\n");
    codec=avcodec_find_decoder(CODEC_ID_H264);
    if(!codec)
    {
        ADM_error("cannot create h264 codec\n");
        goto theEnd;
    }
    ADM_info("Codec created\n");
    ctx=avcodec_alloc_context();
   if (avcodec_open(ctx, codec) < 0)  
   {
        ADM_error("cannot create h264 context\n");
        goto theEnd;
    }
    ADM_info("Context created\n");
    //2- Parse, let's add SPS prefix + Filler postfix to make life easier for libavcodec parser
    ctx->extradata=myData;
    ctx->extradata_size=myLen;
     {
         uint8_t *outptr=NULL;
         int outsize=0;

         av_parser_parse2(parser, ctx, &outptr, &outsize, d, 0, 0, 0,0);
    }
    ADM_info("Width  : %d\n",ctx->width);
    ADM_info("Height : %d\n",ctx->height);
    {
        ffSpsInfo nfo;
        if(!ff_h264_info(parser,&nfo))
        {
            ADM_error("Cannot get sps info from lavcodec\n");
            r=false;
            goto theEnd;
        }
        ADM_info("Width2 : %d\n",nfo.width);
        ADM_info("Height2: %d\n",nfo.height);
        #define CPY(x) spsinfo->x=nfo.x
        CPY(width);
        CPY(height);
        CPY(fps1000);
        CPY(hasStructInfo);
        CPY(CpbDpbToSkip);
        CPY(darNum);
        CPY(darDen);
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

uint8_t extractSPSInfo (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
#define DPY(x) ADM_info(#x":%d\n",(int)spsinfo->x);
#if 1        
        
        bool l=extractSPSInfo_lavcodec(data,len,spsinfo);
        DPY(width);
        DPY(height);
        DPY(fps1000);
        DPY(hasStructInfo);
        DPY(CpbDpbToSkip);
        DPY(darNum);
        DPY(darDen);
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
/**
    \fn    packNalu
    \brief convert mpeg type NALU into mp4 header type nalu
*/
static void packNalu(int idx,NALU_descriptor *nalus,uint32_t *len,uint8_t **data)
{
    NALU_descriptor *n=nalus+idx;
    uint32_t size=n->size;
    uint8_t *p=new uint8_t[size+1];
    *data=p;
    p[0]=n->nalu;
    *len=1+ADM_unescapeH264(n->size, n->start,p+1);
}
/**
    \fn ADM_getH264SpsPpsFromExtraData
    \brief Returns a copy of PPS/SPS extracted from extrdata
*/
bool ADM_getH264SpsPpsFromExtraData(uint32_t extraLen,uint8_t *extra,
                                    uint32_t *spsLen,uint8_t **spsData,
                                    uint32_t *ppsLen,uint8_t **ppsData)
{
  
            if(extraLen<7) // not mov/mp4 formatted...
            {
                ADM_error("Wrong extra data for h264\n");
                return false;
            }
            //
            if(extra[0]==1) // MP4-type extradata
            {
                ADM_info("MP4 style PPS/SPS\n");

                int nbSps=extra[5]&0x1f;
                if(nbSps!=1)    
                {
                    ADM_error("More or less than 1 sps\n");
                    return false;
                    
                }
                uint8_t *c=extra+6;
                *spsLen= (c[0]<<8)+(c[1]);
                *spsData=c+2;
                c+=2+*spsLen;
                int nbPps=c[0]&0x1f;
                if(nbPps!=1)
                {
                    ADM_error("More or less than 1 pps\n");
                    return false;        
                }
                c++;
                *ppsLen= (c[0]<<8)+(c[1]);
                *ppsData=c+2;
                // Duplicate
                uint8_t *y=new uint8_t [*spsLen];
                memcpy(y,*spsData,*spsLen);
                *spsData=y;
                y=new uint8_t [*ppsLen];
                memcpy(y,*ppsData,*ppsLen);
                *ppsData=y;
                ADM_info("Got extradata, ppslen=%d, spslen=%d\n",(int)(*ppsLen),(int)*spsLen);
                return true;

            }else
            if(!extra[0] && !extra[1])
                if(extra[2]==1 || (!extra[2] && extra[3]==1)) // 00 00 01 type extradata
            {
                ADM_info("Startcoded PPS/SPS\n");
                #define NALU_COUNT 10
                NALU_descriptor nalus[NALU_COUNT];      
                int nbNalus=ADM_splitNalu(extra, extraLen+extra, NALU_COUNT,nalus);
                //int ADM_findNalu(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc);
                if(nbNalus<2)
                {
                    ADM_error("Not enough nalus in extradata (%s)\n",nbNalus);
                    return false;
                }
                int spsIndex=ADM_findNalu(NAL_SPS,nbNalus,nalus);
                int ppsIndex=ADM_findNalu(NAL_PPS,nbNalus,nalus);
                if(-1==spsIndex || -1 == ppsIndex)
                {
                    ADM_error("Cant find sps/pps in nalus\n");
                    return false;
                }
                packNalu(spsIndex,nalus,spsLen,spsData);
                packNalu(ppsIndex,nalus,ppsLen,ppsData);
                return true;
            }
    return false;
}
/**
    \fn ADM_splitNalu
    \brief split a nalu annexb size into a list of nalu descriptor
*/
int ADM_splitNalu(uint8_t *start, uint8_t *end, uint32_t maxNalu,NALU_descriptor *desc)
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
        ADM_assert(index<maxNalu);
        desc[index].start=head;
        desc[index].size=offset-START_CODE_LEN;
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
/**
    \fn ADM_findNalu
    \brief lookup for a specific NALU in the given buffer
*/
int ADM_findNalu(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc)
{
    for(int i=0;i<maxNalu;i++)
    {
            if((desc[i].nalu&0x1f) == (nalu&0x1f))
                return i;
    }
    return -1;
}

static void writeBE32(uint8_t *p, uint32_t size)
{
    p[0]=size>>24;
    p[1]=(size>>16)&0xff;
    p[2]=(size>>8)&0xff;
    p[3]=(size>>0)&0xff;
}
/**
    \fn ADM_convertFromAnnexBToMP4   
    \brief convert annexB startcode (00 00 00 0 xx) to NALU
*/
int ADM_convertFromAnnexBToMP4(uint8_t *inData,uint32_t inSize,
                                                      uint8_t *outData,uint32_t outMaxSize)
{
    uint8_t *tgt=outData;
    NALU_descriptor desc[MAX_NALU_PER_CHUNK];
    int nbNalu=ADM_splitNalu(inData,inData+inSize,
                        MAX_NALU_PER_CHUNK,desc);
    int nalHeaderSize=4;
    int outputSize=0;


    for(int i=0;i<nbNalu;i++)
    {
        NALU_descriptor *d=desc+i;
        aprintf("%d/%d : Nalu :0x%x size=%d\n",i,nbNalu,d->nalu,d->size);
        switch(d->nalu&0x1f)
        {
            case NAL_FILLER: break;
            case NAL_AU_DELIMITER: break; 
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
//EOF

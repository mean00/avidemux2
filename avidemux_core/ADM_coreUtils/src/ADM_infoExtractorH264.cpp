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
#    include <malloc.h>
#endif

#include "ADM_default.h"

extern "C"
{
#include "libavcodec/parser.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/ff_spsinfo.h"
#include "libavutil/mem.h"
extern int ff_h264_info(AVCodecParserContext *parser, int ticksPerFrame, ffSpsInfo *ndo);
}

#include "ADM_Video.h"

#include "fourcc.h"
//#include "ADM_mp4.h"

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

#if 0
#define seiprintf ADM_info
#else
#define seiprintf(...) {}
#endif

extern bool ADM_findAnnexBStartCode(uint8_t *start, uint8_t *end, uint8_t *outstartcode,
                uint32_t *offset, bool *fivebytes);

/**
    \fn ADM_getH264SpsPpsFromExtraData
    \brief Returns a copy of PPS/SPS extracted from extrdata
*/
bool ADM_SPSannexBToMP4(uint32_t dataLen,uint8_t *incoming,
                                    uint32_t *outLen, uint8_t *outData)
{
    int p;
    if(dataLen>200)
    {
        ADM_warning("SPS TOO LONG\n");
        return false;
    }
    // 01
    // 4d 40 1f ff
    // e1 00 1b 67
    // 4d 40 1f f6
    // 02 80 2d

    outData[0]=1;
    outData[1]=0x4d;
    outData[2]=0x40;
    outData[3]=0x1f;
    outData[4]=0xff;
    outData[5]=0xe1; // 1 stream
    outData[6]=0; // Len MSB
    outData[7]=0; // Len LSB, should fit in 1 byte...
    outData[8]=0x67; // Len LSB, should fit in 1 byte...
#if 1
    memcpy(outData+9,incoming,dataLen);
    p=dataLen;
#else
    p=ADM_unescapeH264(dataLen,incoming,outData+9) ;
#endif
    outData[7]=(p+1); // Len LSB, should fit in 1 byte...
    outData[6]=(p+1)>>8; // MSB : And no, does not always fit
    *outLen= p+9;
    return true;
}

/**
    \fn ADM_getNalSizeH264
    \brief extract NALU length size from avcC header
*/
uint32_t ADM_getNalSizeH264(uint8_t *extra, uint32_t len)
{
    if(len < 9)
    {
        ADM_warning("Invalid H.264 extradata length %u\n",len);
        return 0;
    }
    if(extra[0] != 1)
    {
        ADM_warning("Invalid H.264 extradata\n");
        return 0;
    }
    return (extra[4] & 3) + 1;
}

/**
    \fn ADM_escapeH264
    \brief Add escape stuff

*/
uint32_t ADM_escapeH264 (uint32_t len, uint8_t * in, uint8_t * out)
{
  uint32_t outlen = 0;
  uint8_t *tail = in + len;
  if (len < 2)
    return 0;
  while (in < tail-1 )
    {
      if (!in[0] && !in[1])
    {
      out[0] = 0;
      out[1] = 0;
          out[2] = 3;
      out += 3;
      outlen += 3;
      in += 2;
          continue;
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
    \fn unescapeH264
    \brief Remove escape stuff

*/
uint32_t ADM_unescapeH264 (uint32_t len, uint8_t * in, uint8_t * out)
{
  if (len < 3)
    return 0;
  uint8_t *offset = in;
  uint8_t *firstOut=out;
  uint32_t outlen = 0;
  uint8_t *tail = in + len;
  uint8_t *border=tail-3;

  while (in < border)
  {
        if(in[1]) // cannot be 00 00 nor the next one
        {
          in += 2;
          continue;
        }
        if(!in[0] && !in[1] && in[2] == 3)
        {
          uint32_t copy = in - offset + 2;
          memcpy(out, offset, copy);
          out += copy;
          in += 3;
          offset = in;
          continue;
        }
        in++;
  }
  outlen=(int)(out-firstOut);
  // copy last bytes
  uint32_t left = tail - offset;
  memcpy (out, offset, left);
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

    bits.get(4);    // bitRateScale
    bits.get(4);    // scale

    for(int i = 0; i <= count; i++)
    {
        bits.getUEG();    // bitrate
        bits.getUEG();  // sizeMinus1
        bits.get(1);    // cbrFlag
    }

    bits.get(5);    // initialRemovalDelay

    int removalDelay=bits.get(5);
    int outputDelay=bits.get(5);

    bits.get(5);    // timeOffset

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

  if (bits.get(1))        // overscan
    bits.get(1);

  if (bits.get(1))        // vsp_color
    {
      bits.get( 4);

      if (bits.get(1))
    {
        bits.get( 8);
        bits.get( 8);
        bits.get( 8);
    }
    }

  if (bits.get(1))        // chroma
    {
      bits.getUEG();
      bits.getUEG();
    }

  if (bits.get(1))        // timing
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
  id = bits.getUEG();    // Seq parameter set id
  printf ("[H264]Profile : %u, Level :%u, SPSid:%u\n", profile, level, id);
  if (profile >= 100)        // ?? Borrowed from H264.C/FFMPEG
    {
      printf ("[H264]Warning : High profile\n");
      chroma_format_idc = bits.getUEG();
      if (chroma_format_idc == 3)    //chroma_format_idc
        separate_colour_plane_flag = bits.get(1);        //residual_color_transform_flag
      bits.getUEG();    //bit_depth_luma_minus8
      bits.getUEG();    //bit_depth_chroma_minus8
      bits.get(1);        // Transform bypass
      if (bits.get(1))    // Scaling matrix
      {
        printf ("[H264] Scaling matrix present\n");
          decodeScalingMatrices(bits);
      }
    }

    if ( separate_colour_plane_flag == 0 )
        chroma_array_type = chroma_format_idc;

  dum = bits.getUEG();    // log2_max_frame_num_minus4
  printf ("[H264]Log2maxFrame-4:%u\n", dum);
  pic_order_cnt_type = bits.getUEG();
  printf ("[H264]Pic Order Cnt Type:%u\n", pic_order_cnt_type);
  if (!pic_order_cnt_type)    // pic_order_cnt_type
    {
      dum = bits.getUEG();    //log2_max_pic_order_cnt_lsb_minus4
      printf ("[H264]Log2maxPix-4:%u\n", dum);
    }
  else
    {
      if (pic_order_cnt_type == 1)
    {
      bits.get(1);    //delta_pic_order_always_zero_flag
      bits.getSEG();    //offset_for_non_ref_pic
      bits.getSEG();    // offset_for_top_to_bottom_field
      int i = bits.getUEG() ;    //num_ref_frames_in_pic_order_cnt_cycle

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
  dum = bits.getUEG();    //num_ref_frames
  printf ("[H264] # of ref frames : %u\n", dum);
  bits.get(1);        // gaps_in_frame_num_value_allowed_flag
  w = bits.getUEG() + 1;    //pic_width_in_mbs_minus1

  mbh = bits.getUEG() + 1;
  frame_mbs_only = bits.get(1);
  h = (2 - frame_mbs_only) * mbh;    //pic_height_in_mbs_minus1

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
    \fn getInfoFromSei
    \brief Get SEI type, decode recovery point
    \return 0: failure, 1: recovery, 2: unregistered user data, 3: both
*/
enum {
    ADM_H264_SEI_TYPE_OTHER = 0,
    ADM_H264_SEI_TYPE_USER_DATA_UNREGISTERED = 1,
    ADM_H264_SEI_TYPE_RECOVERY_POINT = 2
};

static int getInfoFromSei(uint32_t nalSize, uint8_t *org, uint32_t *recoveryLength, uint32_t *unregistered)
{
    int originalNalSize=nalSize+16;
    uint8_t *payloadBuffer=(uint8_t *)malloc(originalNalSize+AV_INPUT_BUFFER_PADDING_SIZE);
    memset(payloadBuffer,0,originalNalSize+AV_INPUT_BUFFER_PADDING_SIZE);
    uint8_t *payload=payloadBuffer;
    int r=ADM_H264_SEI_TYPE_OTHER;
    nalSize=ADM_unescapeH264(nalSize,org,payload);
    if(nalSize>originalNalSize)
    {
        ADM_warning("NAL is way too big : %d, while we expected %d at most\n",nalSize,originalNalSize);
        free(payloadBuffer);
        return r;
    }

    uint8_t *tail=payload+nalSize;

    while(payload+2<tail)
    {
        uint32_t sei_type=0,sei_size=0;
        while(payload[0]==0xff)
        {
            sei_type+=0xff;payload++;
            if(payload+2>=tail)
            {
                seiprintf("Not enough data.\n");
                goto abtSei;
            }
        }
        sei_type+=payload[0];payload++;
        if(payload>=tail)
        {
            seiprintf("No data left after decoding SEI type.\n");
            goto abtSei;
        }
        while(payload[0]==0xff)
        {
            sei_size+=0xff;payload++;
            if(payload+1>=tail)
            {
                seiprintf("Not enough data left after decoding SEI size.\n");
                goto abtSei;
            }
        }
        sei_size+=payload[0];payload++;
        seiprintf("Type: %u size: %u remaining: %d\n",sei_type,sei_size,tail-payload);
        if(payload+sei_size>tail)
        {
            seiprintf("Not enough data.\n");
            break;
        }
        switch(sei_type)
        {
            case 5: // Unregistered user data
            {
                if(!unregistered) break;
                if(sei_size<16)
                {
                    ADM_info("User data too short: %u\n",sei_size);
                    break;
                }
                char *udata=(char *)malloc(16+sei_size+1);
                getBits bits(sei_size,payload);
                for(uint32_t i=0; i<sei_size; i++)
                    udata[i]=bits.get(8);
                udata[sei_size]=0;
                int build;
                if(1!=sscanf(udata+16,"x264 - core %d",&build))
                {
                    ADM_info("Unregistered user data doesn't match the one expected for x264\n");
                    mixDump((uint8_t *)udata,sei_size);
                    break;
                }
                free(udata);
                *unregistered=sei_size;
                ADM_info("Found unregistered user data from x264 build %d, size: %u\n",build,sei_size);
                r |= ADM_H264_SEI_TYPE_USER_DATA_UNREGISTERED;
                break;
            }
            case 6: // Recovery point
            {
                if(!recoveryLength) break;
                getBits bits(sei_size,payload);
                int distance=bits.getUEG();
                if(distance<0)
                {
                    ADM_warning("Invalid UE golomb code encountered while decoding recovery distance.\n");
                    break;
                }
                seiprintf("Recovery distance: %d\n",distance);
                *recoveryLength=distance;
                r |= ADM_H264_SEI_TYPE_RECOVERY_POINT;
                break;
            }
            default:break;
        }
        payload+=sei_size;
    }
abtSei:
    free(payloadBuffer);
    return r;
}
/**
    \fn getNalType
    \brief Return the slice type. The stream is escaped by the function. If recovery==0
           or frame_num==0 I is considered IDR else as P.
*/
static bool getNalType (uint8_t *head, uint8_t *tail, uint32_t *flags, ADM_SPSInfo *sps, int *poc_lsb, int recovery)
{
    if(tail<=head)
        return false;
    uint8_t *out=(uint8_t *)malloc(tail-head+AV_INPUT_BUFFER_PADDING_SIZE);
    memset(out,0,tail-head+AV_INPUT_BUFFER_PADDING_SIZE);
    int size=ADM_unescapeH264(tail-head,head,out);

    getBits bits(size,out);
    uint32_t sliceType;
    uint32_t fieldFlags=0;
    int frame = -1;
    *poc_lsb = -1;

    bits.getUEG();               // first mb in slice
    sliceType = bits.getUEG31(); // get_ue_golomb_31??
    if(sps && sps->log2MaxFrameNum > 3 && sps->log2MaxFrameNum < 17) // sanity check
    {
        bits.getUEG(); // skip PPS id
        frame = bits.get(sps->log2MaxFrameNum);
        if(!sps->frameMbsOnlyFlag && bits.get(1))
        {
            fieldFlags |= AVI_FIELD_STRUCTURE;
            if(bits.get(1))
                fieldFlags |= AVI_BOTTOM_FIELD;
            else
                fieldFlags |= AVI_TOP_FIELD;
        }
        if(sps->hasPocInfo)
        {
            if(*flags & AVI_IDR_FRAME) // from NAL
                bits.getUEG(); // skip idr_pic_id
            *poc_lsb = bits.get(sps->log2MaxPocLsb);
        }
    }
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
        case 0: *flags = AVI_P_FRAME;break;
        case 1: *flags = AVI_B_FRAME;break;
        case 2: case 4:
                if((*flags & AVI_KEY_FRAME) && !sps)
                    break; // trust NAL when we cannot verify
                if(!recovery || !frame)
                    *flags = AVI_KEY_FRAME;
                else
                    *flags = AVI_P_FRAME;
                if(!frame)
                    *flags |= AVI_IDR_FRAME;
                break;

    }
    *flags |= fieldFlags;
    free(out);
    return true;
}

/**
    \fn extractH264FrameType
    \brief Parse access unit in buffer of size len, return frametype in flags (KEY/P/B).
           nalSize should be either the NALU length size value retrieved from extradata or 0 for autodetect.
           If ADM_SPSInfo is provided, return POC LSB or -1 if implicit or on error.
           Return value: 1 on success, 0 on failure.
           To be used only with AVCC (mkv/mp4) nal type (i.e. no startcode)
*/
uint8_t extractH264FrameType(uint8_t *buffer, uint32_t len, uint32_t nalSize, uint32_t *flags, int *pocLsb, ADM_SPSInfo *sps, uint32_t *extRecovery)
{
    uint8_t *head = buffer, *tail = buffer + len;
    uint8_t stream;
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
    uint32_t recovery=0xff;
    int p=-1;

    *flags=0;
    while (head + nalSize < tail)
    {
        length = 0;
        for(i = 0; i < nalSize; i++)
            length = (length << 8) + head[i];
        if (length > len)// || length < 2)
        {
            ADM_warning ("Warning , incomplete nal (%u/%u),(%0x/%0x)\n", length, len, length, len);
            *flags = 0;
            return 0;
        }
        head += nalSize;        // Skip nal lenth
        len = (len > nalSize)? len - nalSize : 0;
        int ref=(*(head)>>5) & 3;
        stream = *(head) & 0x1F;

        switch (stream)
        {
            case NAL_SEI:
                {
                    int sei=getInfoFromSei(length-1, head+1, &recovery, NULL);
                    if(extRecovery)
                    {
                        if(sei & ADM_H264_SEI_TYPE_RECOVERY_POINT)
                            *extRecovery=recovery;
                        else
                            recovery=*extRecovery;
                    }
                }
                break;
            case NAL_SPS:
            case NAL_PPS:
            case NAL_AU_DELIMITER:
            case NAL_FILLER:
                break;
            case NAL_IDR:
                *flags = AVI_KEY_FRAME + AVI_IDR_FRAME;
                if(!getNalType(head+1,head+length,flags,sps,&p,recovery))
                    return 0;
                if(sps && !(*flags & AVI_IDR_FRAME))
                {
                    ADM_warning("Mismatched frame (flags: %d) in IDR NAL unit!\n",*flags);
                    *flags &= ~AVI_B_FRAME;
                    *flags |= AVI_KEY_FRAME; // FIXME
                }
                if(pocLsb)
                    *pocLsb=p;
                return 1;
            case NAL_NON_IDR:
                if(!getNalType(head+1,head+length,flags,sps,&p,recovery))
                    return 0;
                if(!ref && (*flags & AVI_B_FRAME))
                    *flags |= AVI_NON_REF_FRAME;
                if(pocLsb)
                    *pocLsb=p;
                return 1;
            default:
                ADM_warning ("unknown nal ??0x%x\n", stream);
                break;
        }
        head+=length;
        len = (len > length)? len - length : 0;
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
uint8_t extractH264FrameType_startCode(uint8_t *buffer, uint32_t len, uint32_t *flags, int *pocLsb, ADM_SPSInfo *sps, uint32_t *extRecovery)
{
    uint8_t *head = buffer, *tail = buffer + len;
    uint8_t stream;
    uint32_t hnt=0xffffffff;
    uint32_t recovery=0xff;
    int counter = 0, length = 0;
    int ref = 0, p = -1;
    bool last = false;

    *flags = 0;

    while (head + 2 < tail)
    {
        // Search startcode
        hnt = (hnt << 8) + head[0];
        if((hnt & 0xffffff) != 1)
        {
            head++;
            if(head + 2 < tail)
                continue;
            if(!counter) break;
            last = true;
            length = head - buffer + 2;
        }
        int prevNaluRefIdc = 0;
        uint8_t prevNaluType = 0;
        if(!last)
        {
            head++;
            counter++;
            if(counter > 1)
                length = head - buffer - 3; // 3 bytes start code length no matter zero-prefixed or not
            prevNaluRefIdc = (*head >> 5) & 3;
            prevNaluType = *(head++) & 0x1f;
            if(!length)
            {
                buffer = head;
                ref = prevNaluRefIdc;
                stream = prevNaluType;
                continue;
            }
        }
        switch (stream)
        {
            case NAL_SEI:
                {
                    int sei=getInfoFromSei(length, buffer, &recovery, NULL);
                    if(extRecovery)
                    {
                        if(sei & ADM_H264_SEI_TYPE_RECOVERY_POINT)
                            *extRecovery=recovery;
                        else
                            recovery=*extRecovery;
                    }
                }
                break;
            case NAL_SPS: case NAL_PPS: case NAL_FILLER: case NAL_AU_DELIMITER: break;
            case NAL_IDR:
                *flags = AVI_KEY_FRAME + AVI_IDR_FRAME;
                if(!getNalType(buffer, buffer+length, flags, sps, &p, recovery))
                    return 0;
                if(sps && !(*flags & AVI_IDR_FRAME))
                {
                    ADM_warning("Mismatched frame (flags: %d) in IDR NAL unit!\n",*flags);
                    *flags &= ~AVI_B_FRAME;
                    *flags |= AVI_KEY_FRAME; // FIXME
                }
                if(pocLsb)
                    *pocLsb=p;
                return 1;
            case NAL_NON_IDR:
                if(!getNalType(buffer, buffer+length, flags, sps, &p, recovery))
                    return 0;
                if(!ref && (*flags & AVI_B_FRAME))
                    *flags |= AVI_NON_REF_FRAME;
                if(pocLsb)
                    *pocLsb=p;
                return 1;
            default:
                ADM_warning("Unknown NAL type ??0x%x\n", stream);
                break;
        }
        buffer = head;
        ref = prevNaluRefIdc;
        stream = prevNaluType;
    }
  printf ("No stream\n");
  return 0;
}

/**
 *  \fn extractH264SEI
 *  \brief If present, copy SEI containing x264 version info from access unit src to dest
 */
bool extractH264SEI(uint8_t *src, uint32_t inlen, uint32_t nalSize, uint8_t *dest, uint32_t bufsize, uint32_t *outlen)
{
    uint8_t *tail = src, *head = src + inlen;
    uint8_t stream;
    uint32_t i, length = 0;

    if(!nalSize || nalSize > 4)
    { // Try to detect NAL length size.
        nalSize = 4;
        for(i = 0; i < nalSize; i++)
        {
            length = (length << 8) + tail[i];
            if(i && length > inlen)
            {
                nalSize = i;
                break;
            }
        }
    }
    uint32_t unregistered = 0;

    while(tail + nalSize < head)
    {
        length = 0;
        for(i = 0; i < nalSize; i++)
            length = (length << 8) + tail[i];
        if(length > inlen)
        {
            ADM_warning ("Incomplete NALU, length: %u, available: %u\n", length, inlen);
            return false;
        }
        tail += nalSize;
        inlen = (inlen > nalSize)? inlen - nalSize : 0;
        stream = *(tail) & 0x1f;

        if(stream == NAL_SEI)
        {
            if(getInfoFromSei(length-1,tail+1,NULL,&unregistered) & ADM_H264_SEI_TYPE_USER_DATA_UNREGISTERED)
            {
                uint32_t l = nalSize + length;
                if(l > bufsize)
                {
                    ADM_warning("Insufficient destination buffer, need %u, got %u\n",l,bufsize);
                    return false;
                }
                if(dest)
                    memcpy(dest,tail-nalSize,l); // what about emulation prevention bytes??
                if(outlen)
                    *outlen = l;
                return true;
            }
        }
        tail += length;
        inlen = (inlen > length)? inlen - length : 0;
    }

    return false;
}



/**
        \fn extractSPSInfo_mp4Header
        \brief Only works for mp4 style headers i.e. begins by 0x01
*/
bool extractSPSInfo_mp4Header (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
    bool r=false;

    // duplicate
    int myLen=len+AV_INPUT_BUFFER_PADDING_SIZE;
    uint8_t *myData=new uint8_t[myLen];
    memset(myData,0x2,myLen);
    memcpy(myData,data,len);
    myData[len]=0; // stop ff_h264_decode_extradata() from trying to parse the remaining buffer content as PPS

    // 1-Create parser
    AVCodecParserContext *parser=av_parser_init(AV_CODEC_ID_H264);
    AVCodecContext *ctx=NULL;
    AVCodec *codec=NULL;
    uint8_t *d=NULL;

    if(!parser)
    {
        ADM_error("cannot create h264 parser\n");
        goto theEnd;
    }
    ADM_info("Parser created\n");
    codec=avcodec_find_decoder(AV_CODEC_ID_H264);
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

    ADM_info("Context created, ticks_per_frame = %d\n",ctx->ticks_per_frame);
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
           //ADM_warning("Failed to extract SPS info\n"); // it ain't necessarily so
           //  goto theEnd;
         }
    }
    // Size is not supposed to be set in AVCodecContext after parsing
    //ADM_info("Width  : %d\n",ctx->width);
    //ADM_info("Height : %d\n",ctx->height);
    {
        ffSpsInfo nfo;
        if(!ff_h264_info(parser,ctx->ticks_per_frame,&nfo))
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
        CPY(hasPocInfo);
        CPY(CpbDpbToSkip);
        CPY(log2MaxFrameNum);
        CPY(log2MaxPocLsb);
        CPY(frameMbsOnlyFlag);
        CPY(darNum);
        CPY(darDen);
        CPY(refFrames);
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
    \fn getRawH264SPS
    \brief Find the first SPS in mp4 style buffer and copy it to dest, return SPS length.
*/
uint32_t getRawH264SPS(uint8_t *data, uint32_t len, uint32_t nalSize, uint8_t *dest, uint32_t maxsize)
{
    if(!dest || !maxsize)
        return 0;

    uint8_t *head=data, *tail=data+len;
    uint8_t stream;

    uint32_t i, length=0;
    if(!nalSize || nalSize > 4)
    { // Try to detect NAL length size.
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

    while(head + nalSize < tail)
    {
        length=0;
        for(i = 0; i < nalSize; i++)
            length = (length << 8) + head[i];
        if(length > len)
        {
            ADM_warning ("Incomplete NALU, length: %u, available: %u\n", length, len);
            return 0;
        }
        head += nalSize;
        len = (len>nalSize)? len-nalSize : 0;
        stream = *head & 0x1F;

        if(stream == NAL_SPS)
        {
            if(length>maxsize)
            {
                ADM_warning("Buffer too small for SPS: need %u got %u\n",length,maxsize);
                return 0;
            }
            memcpy(dest,head,length);
            return length;
        }
        head += length;
        len = (len>length)? len-length : 0;
    }

    return 0;
}

/**
    \fn getRawH264SPS_startCode
    \brief Find the first SPS in AnnexB style buffer and copy it to dest, return SPS length.
*/
uint32_t getRawH264SPS_startCode(uint8_t *data, uint32_t len, uint8_t *dest, uint32_t maxsize)
{
    if(!dest || !maxsize)
        return 0;

    uint8_t *head = data;
    uint8_t *tail = head + len;
    uint8_t stream = 0;
    uint32_t hnt = 0xffffffff;
    int counter = 0, length = 0;
    bool last = false;
#define MAX_NALU_TO_CHECK 4
    while(head + 2 < tail)
    {
        if(counter > MAX_NALU_TO_CHECK)
            return 0;

        hnt = (hnt << 8) + head[0];
        if((hnt & 0xffffff) != 1)
        {
            head++;
            if(head + 2 < tail)
                continue;
            if(!counter) break;
            last = true;
        }
        length = head - data + 2;
        uint8_t prevNaluType = 0;
        if(!last)
        {
            head++;
            counter++;
            if(counter > 1)
                length = head - data - 3; // likely one zerobyte too much, harmless
            prevNaluType = *head & 0x1f;
            if(!length)
            {
                data = head;
                stream = prevNaluType;
                continue;
            }
        }
        if(stream == NAL_SPS)
        {
            if(length>maxsize)
            {
                ADM_warning("Buffer too small for SPS: need %d, got %u\n",length,maxsize);
                return 0;
            }
            memcpy(dest,data,length);
            return length;
        }
        data = head++;
        stream = prevNaluType;
    }
#undef MAX_NALU_TO_CHECK
    return 0;
}

/**
    \fn extractSPSInfoFromData
    \brief Decode raw SPS data
*/
bool extractSPSInfoFromData(uint8_t *data, uint32_t length, ADM_SPSInfo *spsinfo)
{
    uint32_t myExtraLen=length+8;
    uint8_t *myExtra=new uint8_t[myExtraLen];
    memset(myExtra,0,myExtraLen);
    uint8_t *p=myExtra;
    // Create fake avcC extradata
    *p++=1;       // AVC version
    *p++=data[1]; // Profile
    *p++=data[2]; // Profile compatibility
    *p++=data[3]; // Level
    *p++=0xff;    // Nal size minus 1
    *p++=0xe1;    // 1x SPS
    *p++=length>>8;
    *p++=length&0xFF;
    memcpy(p,data,length);

    bool r = extractSPSInfo_mp4Header(myExtra,myExtraLen,spsinfo);

    delete [] myExtra;
    myExtra=NULL;

    return r;
}

/**
        \fn extractSPSInfo2
        \brief Same as extractSPSInfo, but using libavcodec
*/
uint8_t extractSPSInfo_lavcodec (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
    if(data[0]==1) return extractSPSInfo_mp4Header(data,len,spsinfo);

    ADM_info("Incoming SPS info\n");
    mixDump(data,len);

    ADM_info("converted SPS info\n");

    uint32_t converted;
    uint8_t buffer[256];
    if(! ADM_SPSannexBToMP4(len,data,&converted,buffer))
    {
        ADM_warning("Cannot convert SPS\n");
        return false;
    }
    mixDump(buffer,converted);
    return    extractSPSInfo_mp4Header(buffer,converted,spsinfo) ;

}

bool  extractSPSInfo (uint8_t * data, uint32_t len, ADM_SPSInfo *spsinfo)
{
#define DPY(x) ADM_info(#x":%d\n",(int)spsinfo->x);
#if 1

        bool l=extractSPSInfo_lavcodec(data,len,spsinfo);
        if(l)
        {
            DPY(width);
            DPY(height);
            DPY(fps1000);
            DPY(hasStructInfo);
            DPY(hasPocInfo);
            DPY(CpbDpbToSkip);
            DPY(log2MaxFrameNum);
            DPY(log2MaxPocLsb);
            DPY(frameMbsOnlyFlag);
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
    bool zeroBytePrefixed,oldZbp=false;
    const uint32_t startCodePrefixLen=4;
    int index=0;

    while(true==ADM_findAnnexBStartCode(head,end,&startCode,&offset,&zeroBytePrefixed))
    {
        if(true==first)
        {
            head+=offset;
            first=false;
            oldStartCode=startCode;
            oldZbp=zeroBytePrefixed;
            continue;
        }
        if(index>=maxNalu)
        {
            ADM_warning("Number of NALUs exceeds max (%d), dropping the leftover.\n",maxNalu);
            return index;
        }
        desc[index].start=head;
        desc[index].size=offset-zeroBytePrefixed-startCodePrefixLen;
        desc[index].nalu=oldStartCode;
        desc[index].zerobyte=oldZbp;
        index++;
        head+=offset;
        oldStartCode=startCode;
        oldZbp=zeroBytePrefixed;
    }
    // leftover
    desc[index].start=head;
    desc[index].size=(uint32_t)(end-head);
    desc[index].nalu=oldStartCode;
    desc[index].zerobyte=oldZbp;
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
    \fn ADM_convertFromAnnexBToMP4_internal
    \brief convert annexB startcode (00 00 00 0 xx) to NALU
*/
int ADM_convertFromAnnexBToMP4(uint8_t *inData, uint32_t inSize,
                               uint8_t *outData,uint32_t outMaxSize)
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

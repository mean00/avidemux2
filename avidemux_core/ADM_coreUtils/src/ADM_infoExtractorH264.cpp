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


#include "ADM_includeFfmpeg.h"
#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
//#include "ADM_mp4.h"

#define aprintf(...) {}
#include "ADM_getbits.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_h264_tag.h"

static void refineH264FrameType (uint8_t * head, uint8_t * tail,
				 uint32_t * flags);
bool ADM_findMpegStartCode (uint8_t * start, uint8_t * end,
			    uint8_t * outstartcode, uint32_t * offset);
/**
    \fn unescapeH264
    \brief Remove escape stuff

*/
static uint32_t
unescapeH264 (uint32_t len, uint8_t * in, uint8_t * out)
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
        \fn extractVUIInfo
*/
static uint8_t  extractVUIInfo (getBits &bits, uint32_t * fps1000, uint32_t * darNum,
		uint32_t * darDen)
{
  *fps1000 = *darNum = *darDen = 0;

  if (bits.get(1))
    {
      unsigned int aspect_ratio_information = bits.get( 8);

      if (aspect_ratio_information == 255)
	{
	  *darNum = bits.get( 16);
	  *darDen = bits.get( 16);
	}
      else if (aspect_ratio_information <
	       sizeof (pixel_aspect) / sizeof (*pixel_aspect))
	{
	  *darNum = pixel_aspect[aspect_ratio_information].num;
	  *darDen = pixel_aspect[aspect_ratio_information].den;
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
	*fps1000 =
	  (uint32_t) (((float) timeinc_resolution / (float) timeinc_unit) *
		      1000);
/* No!
      if (fixed_fps)
	*fps1000 /= 2;
*/
    }

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
uint8_t extractSPSInfo (uint8_t * data, uint32_t len, uint32_t * wwidth,
		uint32_t * hheight, uint32_t * fps1000, uint32_t * darNum, uint32_t * darDen)
{
  
  *fps1000=0;
  uint32_t profile, constraint, level, pic_order_cnt_type, w, h, mbh,
    frame_mbs_only;
  uint8_t buf[len];
  uint32_t outlen;
  uint32_t id, dum;

  outlen = unescapeH264 (len, data, buf);
  getBits bits(outlen,buf);
  

  profile = bits.get(8);
  constraint = bits.get( 8) >> 5;
  level = bits.get( 8);
  id = bits.getUEG();	// Seq parameter set id           
  printf ("[H264]Profile : %u, Level :%u, SPSid:%u\n", profile, level, id);
  if (profile >= 100)		// ?? Borrowed from H264.C/FFMPEG
    {
      printf ("[H264]Warning : High profile\n");
      if (bits.getUEG() == 3)	//chroma_format_idc
        bits.get(1);		//residual_color_transform_flag
      bits.getUEG();	//bit_depth_luma_minus8
      bits.getUEG();	//bit_depth_chroma_minus8
      bits.get(1);		// Transform bypass
      if (bits.get(1))	// Scaling matrix
	  {
	    printf ("[H264] Scaling matrix present\n");
          decodeScalingMatrices(bits);
	  }
    }


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

  *wwidth = w * 16;
  *hheight = h * 16;

  if (!frame_mbs_only)
    bits.get(1);

  bits.get(1);

  if (bits.get(1))
    {
      bits.getUEG();
      bits.getUEG();
      bits.getUEG();
      bits.getUEG();
    }

  if (bits.get(1))
    {
      extractVUIInfo (bits, fps1000, darNum, darDen);
      printf ("[H264] Fps %" LU ", a.r. %" LU ",%" LU "\n", *fps1000, *darNum,
	      *darDen);
    }
  else
    {
      printf ("[H264] Unknown FPS, setting 25\n");
      *fps1000 = 25000;
    }
  return 1;
}
/**
      \fn extractH264FrameType
      \brief return frametype in flags (KEY_FRAME or 0). To be used only with  mkv/mp4 nal type (i.e. no startcode)
      
*/
uint8_t extractH264FrameType (uint32_t nalSize, uint8_t * buffer, uint32_t len,
		      uint32_t * flags)
{
  uint8_t *head = buffer, *tail = buffer + len;
  uint8_t stream;

  uint32_t val, hnt;

// FIXME :  no startcode only !

  while (head + 4 < tail)
    {

      uint32_t length =
	(head[0] << 24) + (head[1] << 16) + (head[2] << 8) + (head[3]);
      //printf("Block size : %"LU", available : %"LU"\n",length,len);
      if (length > len || length < 6)
	{
	  printf ("Warning , incomplete nal (%u/%u),(%0x/%0x)\n", length, len,
		  length, len);
	  *flags = 0;
	  return 0;
	}
      head += 4;		// Skip nal lenth
      stream = *(head) & 0x1F;
      switch (stream)
	{
	case NAL_IDR:
	  *flags = AVI_KEY_FRAME;

	  return 1;
	  break;
	case NAL_NON_IDR:
	  refineH264FrameType (head + 1, tail, flags);
	  return 1;
	  break;
	default:
	  printf ("??0x%x\n", stream);
	case NAL_SEI:
	  head += length;
	  continue;
	}
    }
  printf ("No stream\n");
  return 0;
}

/**
      \fn extractH264FrameType_startCode
      \brief return frametype in flags (KEY_FRAME or 0). To be used only with  avi / mpeg TS nal type (i.e. with startcode)
      
*/
uint8_t extractH264FrameType_startCode(uint32_t nalSize, uint8_t * buffer,uint32_t len, uint32_t * flags)
{
  uint8_t *head = buffer, *tail = buffer + len;
  uint8_t stream;
#define NAL_NON_IDR       1
#define NAL_IDR           5
#define NAL_SEI           6

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
	  refineH264FrameType (head, tail, flags);
	  return 1;
	  break;
	default:
	  printf ("??0x%x\n", stream);
	  continue;
	}
    }
  printf ("No stream\n");
  return 0;
}
/**
    \fn refineH264FrameType
    \brief Try to detect B slice, warning the stream is not escaped!
*/
void refineH264FrameType (uint8_t * head, uint8_t * tail, uint32_t * flags)
{
  getBits bits(tail-head,head);
  uint32_t sliceType;
  *flags = 0;
  
  bits.getUEG();
  sliceType = bits.getUEG31(); // get_ue_golomb_31??
  if (sliceType > 9)
    {
      printf ("Weird Slice %d\n", sliceType);
      return;
    }
  if (sliceType > 4)
    sliceType -= 5;
  if (sliceType == 1)
    *flags = AVI_B_FRAME;
  //printf("[H264] Slice type : %"LU"\n",sliceType);
}
//EOF

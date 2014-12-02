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
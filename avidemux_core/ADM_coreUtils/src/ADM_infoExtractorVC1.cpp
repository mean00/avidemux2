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



bool ADM_findMpegStartCode (uint8_t * start, uint8_t * end,
			    uint8_t * outstartcode, uint32_t * offset);

/**
 * 
 * @param start
 * @param size
 * @param frameType
 * @return 
 */

static int toType[16]= // unary ?
{
    0,0,0,0,0,0,0,0, // 0xxx
    AVI_B_FRAME,AVI_B_FRAME,AVI_B_FRAME,AVI_B_FRAME, // 10xx
    AVI_KEY_FRAME,AVI_KEY_FRAME, // 110x
    AVI_B_FRAME,0
};

bool ADM_VC1getFrameType(uint8_t *start, int size, int *frameType)
{
    uint8_t *end=start+size; // We should not need escaping, (??)
    uint8_t code;
    uint32_t offset;
    
    while(ADM_findMpegStartCode(start,end,&code,&offset))
    {
        start+=offset;
        if(code!=0x0D) // frame, field = 0xc , SLICE=0x0b
            continue;
        *frameType=toType[*start >> 4];
        printf("VC1 : => %02x : 0x%02x %02x %02x %02x\n",*frameType,code,start[0],start[1],start[2]);
        return true;
        // 
    }
    return false;
}
//EOF

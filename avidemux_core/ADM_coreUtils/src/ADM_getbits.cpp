/**
        \file ADM_getbits.cpp
        \brief Wrapper around ffmpeg getbits function
        \author mean fixounet@free.Fr (c) 2010

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"

#ifndef ADM_getbits_H
#define ADM_getbits_H

#define ADM_LAV_NO_CONFIG
#define CONFIG_FTRAPV 0

#define UNCHECKED_BITSTREAM_READER 0

extern "C"
{
#include "libavutil/common.h"
#include "libavutil/bswap.h"
#ifndef INT_MAX
#define INT_MAX (0x7FFFFFFF)
#endif
#include "libavutil/avconfig.h"
#include "libavcodec/get_bits.h"
#include "libavcodec/golomb.h"
}
#undef printf
#undef sprintf
#endif


#include "ADM_getbits.h"
static int my_get_ue_golomb(GetBitContext *gb);
/**
    \fn ctor
*/
getBits::getBits(const getBits &source)
{
    GetBitContext *c=new  GetBitContext;
    ctx=(void *)c;
    *c=*(GetBitContext *)source.ctx;

}
void getBits::align(void)
{
    align_get_bits((GetBitContext *)ctx);
}
/**
    \fn ctor
*/
getBits::getBits(int bufferSize, uint8_t *buffer)
{
    GetBitContext *c=new  GetBitContext;
    init_get_bits(c,buffer,bufferSize*8);
    ctx=(void *)c;

}

/**
    \fn dtor
*/
getBits::~getBits()
{
    GetBitContext *c= (GetBitContext *)ctx;
    delete c;
    ctx=NULL;
}
int getBits::get(int nb)
{
    if(nb>15) return get_bits_long((GetBitContext *)ctx,nb);
    return get_bits( (GetBitContext *)ctx,nb);
}
int getBits::skip(int nb)
{
     skip_bits( (GetBitContext *)ctx,nb);
     return 0; 
}
int getBits::getUEG(void)
{
    return my_get_ue_golomb( (GetBitContext *)ctx);
}
int getBits::getSEG(void)
{
    return get_se_golomb( (GetBitContext *)ctx);
}
int getBits::getUEG31(void)
{
    return get_ue_golomb_31( (GetBitContext *)ctx);
}
int getBits::getConsumedBits(void)
{
    return get_bits_count((GetBitContext *)ctx);
}
// Bug with gcc 4.8.3 on win64, force it to NOT inline
int my_get_ue_golomb(GetBitContext *gb)
{
    unsigned int buf;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf = GET_CACHE(re, gb);

    if (buf >= (1 << 27)) {
        buf >>= 32 - 9;
        LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
        CLOSE_READER(re, gb);

        return ff_ue_golomb_vlc_code[buf];
    } else {
        int log = 2 * av_log2(buf) - 31;
        LAST_SKIP_BITS(re, gb, 32 - log);
        CLOSE_READER(re, gb);
        if (log < 7) {
            av_log(NULL, AV_LOG_ERROR, "Invalid UE golomb code\n");
            return AVERROR_INVALIDDATA;
        }
        buf >>= log;
        buf--;

        return buf;
    }
}
// EOF

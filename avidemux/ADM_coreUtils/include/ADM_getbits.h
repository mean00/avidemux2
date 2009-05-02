/**
        \file ADM_getbits.h
        \brief Wrapper around ffmpeg getbits function

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
extern "C"
{
#define ADM_NO_CONFIG_H
#include "ADM_libraries/ADM_ffmpeg/libavutil/common.h"
#include "ADM_libraries/ADM_ffmpeg/libavutil/bswap.h"
#ifndef INT_MAX
#define INT_MAX (0x7FFFFFFF)
#endif
#include "ADM_libraries/ADM_ffmpeg/ffmpeg_config/config.h"
#include "ADM_libraries/ADM_ffmpeg/libavutil/internal.h"
#include "ADM_libraries/ADM_ffmpeg/libavcodec/get_bits.h"
#include "ADM_libraries/ADM_ffmpeg/libavcodec/golomb.h"
}
#undef printf
#undef sprintf
#endif


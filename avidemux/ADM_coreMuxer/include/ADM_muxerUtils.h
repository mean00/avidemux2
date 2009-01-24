/***************************************************************************
                          ADM_muxerUtils.cpp  -  description
                             -------------------
    copyright            : (C) 2008 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_MUXER_UTILS_H
#define ADM_MUXER_UTILS_H
extern "C" 
{
#include "ADM_lavformat/avformat.h"
};

#define ADM_NO_PTS 0xFFFFFFFFFFFFFFFFLL // FIXME

// Fwd ref
uint8_t isMpeg4Compatible (uint32_t fourcc);
uint8_t isH264Compatible (uint32_t fourcc);
uint8_t isMSMpeg4Compatible (uint32_t fourcc);
uint8_t isDVCompatible (uint32_t fourcc);
uint8_t isVP6Compatible (uint32_t fourcc);

/**
    \fn rescaleFps
    \brief Rescale fps to be accurate (i.e. 23.976 become 24000/1001)

*/
void  rescaleFps(uint32_t fps1000, AVRational *rational);

/**
        \fn rescaleLavPts
        \brief Rescale PTS/DTS the lavformat way, i.e. relative to the scale.
*/
uint64_t rescaleLavPts(uint64_t us, AVRational *scale);

#endif


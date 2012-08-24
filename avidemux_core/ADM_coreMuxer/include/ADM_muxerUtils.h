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
#include "libavformat/avformat.h"
};

#include "ADM_coreMuxer6_export.h"
#include "ADM_codecType.h"

/**
    \fn rescaleFps
    \brief Rescale fps to be accurate (i.e. 23.976 become 24000/1001)

*/
ADM_COREMUXER6_EXPORT void  rescaleFps(uint32_t fps1000, AVRational *rational);

/**
        \fn rescaleLavPts
        \brief Rescale PTS/DTS the lavformat way, i.e. relative to the scale.
*/
ADM_COREMUXER6_EXPORT uint64_t rescaleLavPts(uint64_t us, AVRational *scale);

#endif


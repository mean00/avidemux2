/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#ifndef ADM_CORE_CODEC_MAPPING_H
#define ADM_CORE_CODEC_MAPPING_H

#include "ADM_coreUtils6_export.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

typedef struct
{
    const char *string;
    AVCodecID  codecId;
    bool       extraData;
    bool       refCopy;
    bool       hasBFrame;
}ffVideoCodec;

/**
    \fn getCodecIdFromFourcc
*/
ADM_COREUTILS6_EXPORT const ffVideoCodec *getCodecIdFromFourcc(uint32_t fcc);
ADM_COREUTILS6_EXPORT AVCodecID ADM_codecIdFindByFourcc(const char *fcc);
#endif

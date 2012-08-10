/**
    \file ADM_dcainfo
    \brief Extracts and sync DTS/DCA frames
    \author mean (C) 2009 fixounet@free.fr

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_DCAINFO_H
#define ADM_DCAINFO_H

#include "ADM_audioParser6_export.h"

/**
    \struct ADM_DCA_INFO
*/
typedef struct
{
    uint32_t frequency;
    uint32_t bitrate;   // in b/s
    uint32_t channels;
    uint32_t frameSizeInBytes;
    uint32_t samples;
    uint32_t flags;
}ADM_DCA_INFO;
/**
    \fn ADM_DCAGetInfo
*/
ADM_AUDIOPARSER6_EXPORT bool ADM_DCAGetInfo(uint8_t *buf, uint32_t len,ADM_DCA_INFO *info,uint32_t *syncoff);

#define DTS_HEADER_SIZE (10)
#endif



/***************************************************************************
        \file ADM_codecType
        \brief Identifies codec family
    copyright            : (C) 2009 by mean

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CODECTYPE_H
#define ADM_CODECTYPE_H

#include "ADM_coreUtils6_export.h"

ADM_COREUTILS6_EXPORT bool isMpeg4Compatible  (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isH264Compatible   (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isH265Compatible   (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isMSMpeg4Compatible(uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isDVCompatible     (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isVP6Compatible    (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isMpeg12Compatible (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isVC1Compatible    (uint32_t fourcc);
ADM_COREUTILS6_EXPORT bool isVP9Compatible    (uint32_t fourcc);

#endif
//EOF

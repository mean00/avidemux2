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

bool isMpeg4Compatible  (uint32_t fourcc);
bool isH264Compatible   (uint32_t fourcc);
bool isMSMpeg4Compatible(uint32_t fourcc);
bool isDVCompatible     (uint32_t fourcc);
bool isVP6Compatible    (uint32_t fourcc);
bool isMpeg12Compatible (uint32_t fourcc);
bool isVC1Compatible    (uint32_t fourcc);

#endif
//EOF

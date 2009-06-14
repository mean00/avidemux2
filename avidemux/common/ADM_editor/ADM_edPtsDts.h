/***************************************************************************

    \file ADM_edPtsDts.h
    \brief Try to guess Pts from Dts

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
#ifndef ADM_EDPTSDTS_H
#define ADM_EDPTSDTS_H
#include "ADM_Video.h"

bool setPtsEqualDts(vidHeader *hdr,uint64_t timeIncrementUs);
bool setMpeg4PtsFromDts(vidHeader *hdr,uint64_t timeIncrementUs);

#endif

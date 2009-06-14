/***************************************************************************
    copyright            : (C) 2005 by mean
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

#ifndef _CNR2_
#define _CNR2_

typedef struct CNR2Param
{
        double scdthr;          // Scene change threshold in % default 10
        int32_t ln, lm;         // n=sensibility, m=maximum
        int32_t un, um;
        int32_t vn, vm;
        int32_t sceneChroma;    // If true, both luma & chroma are used for scene detection
        uint32_t mode;          // XX 00 00 Y, 00 XX 00 U , 00 00 XX V
                                // Default is wide, puting a bit means narrow

} CNR2Param;

#endif

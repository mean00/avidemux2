/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef BLEND_REMOVAL_PARAM_H
#define BLEND_REMOVAL_PARAM_H

typedef struct BLEND_REMOVER_PARAM
{
        uint32_t threshold;
        uint32_t noise;
        uint32_t show;
        uint32_t identical;
}BLEND_REMOVER_PARAM;
uint8_t  DIA_blendRemoval(BLEND_REMOVER_PARAM *mosaic);
#endif

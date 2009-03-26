/** *************************************************************************
    \file ADM_frameType
    \brief Return frametype from bitstream
                      
    copyright            : (C) 2009 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_frameType.h"
#include "ADM_codecType.h"

typedef int (*frameIdentifier)(uint32_t len,uint8_t *data);

/// \fn    ADM_getFrameIdentifier
/// \brief Returns a function that identify the frametype according to its bitstream
//bool        ADM_findMpegStartCode(uint8_t *start, uint8_t *end,uint8_t *outstartcode,uint32_t *offset);
frameIdentifier *ADM_getFrameIdentifier(uint32_t fourcc)
{
    //if(isMpeg4Compatible(fourcc))return frameIdMpeg4;
    //if(isMpeg12Compatible(fourcc))return frameIdH264;
    //if(isH264Compatible(fourcc))return frameIdH264;

    return NULL;
}


// EOF

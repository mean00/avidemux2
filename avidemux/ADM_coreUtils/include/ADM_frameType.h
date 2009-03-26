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
#ifndef ADM_frameType_h
#define ADM_frameType_h

#define AVI_KEY_FRAME	0x10
#define AVI_B_FRAME	    0x4000	// hopefully it is not used..
#define AVI_P_FRAME   	0x0
#define AVI_ERR_FRAME   0x8888

typedef int (*frameIdentifier)(uint32_t len,uint8_t *data);

/// \fn    ADM_getFrameIdentifier
/// \brief Returns a function that identify the frametype according to its bitstream

frameIdentifier *ADM_getFrameIdentifier(uint32_t fourcc);


#endif

/***************************************************************************
    \file ADM_imageFlags.h
    \brief Describe the flags field in image/demuxer
    \author Mean (c) 2010 fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_IMAGEFLAGS_H
#define ADM_IMAGEFLAGS_H


#define AVI_KEY_FRAME   0x10
#define AVI_B_FRAME	    0x4000	 
#define AVI_P_FRAME     0x0
#define AVI_FRAME_TYPE_MASK (AVI_KEY_FRAME+AVI_B_FRAME+AVI_P_FRAME)

/**
    For demuxers, it is the field type/frame
    For decoders, the TOP/Bottom is a hint about TFF/BFF as we always get a full picture
*/
#define AVI_TOP_FIELD        0x1000
#define AVI_BOTTOM_FIELD     0x2000
#define AVI_FIELD_STRUCTURE  0x8000
#define AVI_FRAME_STRUCTURE  0x0
#define AVI_STRUCTURE_TYPE_MASK (AVI_TOP_FIELD+AVI_BOTTOM_FIELD+AVI_FIELD_STRUCTURE)

#define AVI_ERR_FRAME   0x8888
#endif
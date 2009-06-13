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
#include "avidemutils.h"

/**
    \fn mpeg12FrameIdentifier
*/
static int mpeg12FrameIdentifier(uint32_t len,uint8_t *data)
{
    uint8_t *start=data;
    uint8_t *end=data+len;
    uint32_t offset;
    uint8_t code;
    while(ADM_findMpegStartCode(start,end,&code,&offset) && start<end-4)
    {
        if(code)
        {
            start+=offset;
            continue;
        }
        uint16_t val;
        int type;
        start+=offset;
        val=*start++;
        val=(val<<8)+*start++;
        type=7 & (val>>3);
        if( type<1 ||  type>3)
          {
                  printf("[mpeg12FrameType]Met illegal pic at offset %"LX"\n",(uint32_t)(start-data));
                  continue;
          }
        switch(type)
        {
            case 1:return AVI_KEY_FRAME;break;
            case 2:return AVI_P_FRAME;break;
            case 3:return AVI_B_FRAME;break;
        }
    }
    return AVI_ERR_FRAME;
}

/**
    \fn mpeg4FrameIdentifier
*/
static int mpeg4FrameIdentifier(uint32_t len,uint8_t *data)
{
    uint8_t *start=data;
    uint8_t *end=data+len;
    uint32_t offset;
    uint8_t code;
    while(ADM_findMpegStartCode(start,end,&code,&offset) && start<end-4)
    {
       start+=offset;
       if(code!=0xb6)
        {
            continue;
        }

        // Analyse a bit the vop header
        uint8_t coding_type=*start;
        coding_type>>=6;
        switch(coding_type)
        {
            case 0: return AVI_KEY_FRAME;break;
            case 1: return AVI_P_FRAME;break;
            case 2: return AVI_B_FRAME;break;
            default: printf("[mpeg4frame]Glouglou %d\n",coding_type);continue;break;

        }
    }
    return AVI_ERR_FRAME;
}

/** \fn    ADM_getFrameIdentifier
    \brief Returns a function that identify the frametype according to its bitstream
*/
frameIdentifier ADM_getFrameIdentifier(uint32_t fourcc)
{
    if(isMpeg4Compatible(fourcc))return mpeg4FrameIdentifier;
    if(isMpeg12Compatible(fourcc))return mpeg12FrameIdentifier;
    //if(isH264Compatible(fourcc))return frameIdH264;

    return NULL;
}
// EOF


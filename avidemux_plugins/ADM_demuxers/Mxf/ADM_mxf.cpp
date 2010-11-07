/***************************************************************************
     \file                     ADM_mxf.cpp
     \brief MXF demuxer
     \author mean, fixounet@free.fr (C) 2010
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"

#include <string.h>
#include <math.h>

#include "ADM_mxf.h"


#define aprintf(...) {}
/**
    \fn ctor
*/
mxfHeader::mxfHeader(void)
{
	
}
/**
    \fn getTime
*/

uint64_t                   mxfHeader::getTime(uint32_t frameNum)
{
    return 0;
}
/**
    \fn getVideoDuration
*/

uint64_t                   mxfHeader::getVideoDuration(void)
{
    return 0;
}
/**
    \fn getFrameSize
*/
uint8_t                 mxfHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    return 0;
}
/**
    \fn getFrame
*/
uint8_t mxfHeader::getFrame(uint32_t framenum, ADMCompressedImage *img)
{
	return 0;
}
/**
    \fn close
*/
uint8_t mxfHeader::close(void)
{
	return true;
}

/**
    \fn open
*/
uint8_t mxfHeader::open(const char *inname)
{
    
    return false;
}
/**
    \fn setFlag
*/

uint8_t mxfHeader::setFlag(uint32_t frame, uint32_t flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    return 0;
}
/**
    \fn getFlags
*/

uint32_t mxfHeader::getFlags(uint32_t frame, uint32_t * flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    return 0;
}
/**
    \fn getPtsDts
*/
bool       mxfHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    return false;
}
/**
    \fn setPtsDts
*/

bool       mxfHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    return false;
}
// EOF

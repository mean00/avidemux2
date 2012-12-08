/***************************************************************************
    copyright            : (C) 2003-2008 by mean
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
#include "ADM_default.h"
#include "ADM_bitstream.h"



ADMBitstream::ADMBitstream(uint32_t size)
{
    memset(this,0,sizeof(*this));
    bufferSize=size;
}
ADMBitstream::~ADMBitstream()
{
}
void ADMBitstream::cleanup (uint32_t framenum)
{
//    ptsFrame=dtsFrame=0;
    pts=dts=0;
    out_quantizer=0;
    flags=0;
    len=0;
//    dtsFrame=framenum;
}


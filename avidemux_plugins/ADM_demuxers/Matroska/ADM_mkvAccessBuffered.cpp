/***************************************************************************
    copyright            : (C) 2006 by mean
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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "math.h"
#include "ADM_Video.h"

#include "ADM_mkv.h"
#include "ADM_a52info.h"
#include "ADM_dcainfo.h"

#include "ADM_vidMisc.h"
#if 0
#define vprintf printf
#else
#define vprintf(...) {}
#endif

/**
    \fn mkvAccess
    \brief constructor

*/
mkvAccessBuffered::mkvAccessBuffered(mkvAccess *access, int maxSize)
{
    _maxSize=maxSize;
    _son=access;
    _buffer=new uint8_t[maxSize];
}

mkvAccessBuffered::~mkvAccessBuffered()
{
    if(_buffer) delete [] _buffer;
    if(_son) delete [] _son;
    _buffer=NULL;
    _son=NULL;
}
bool      mkvAccessBuffered::getExtraData(uint32_t *l, uint8_t **d)
{
    return _son->getExtraData(l,d);
}
/**
    \fn getDurationInUs
*/
uint64_t  mkvAccessBuffered::getDurationInUs(void)
{
    return _son->getDurationInUs();
}

/**
    \fn goToTime
*/
bool      mkvAccessBuffered::goToTime(uint64_t timeUs)
{
    return _son->goToTime(timeUs);
}
/**
    \fn getPacket
*/
bool    mkvAccessBuffered::getPacket(uint8_t *dest, uint32_t *packLen, uint32_t maxSize,uint64_t *timecode)
{
   return _son->getPacket(dest,packLen,maxSize,timecode);
}
//EOF

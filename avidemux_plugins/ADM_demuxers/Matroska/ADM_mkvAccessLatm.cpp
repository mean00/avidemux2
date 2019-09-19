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
#include "ADM_mkv.h"
#if 0
#define aprintf ADM_info
#else
#define aprintf(...) {}
#endif

/**
    \fn ctor
*/
mkvAccessLatm::mkvAccessLatm(mkvAccess *access, int maxSize)
{
    _maxSize=maxSize;
    _son=access;
    _buffer=new uint8_t[maxSize];
}
/**
    \fn dtor
*/
mkvAccessLatm::~mkvAccessLatm()
{
    if(_buffer) delete [] _buffer;
    if(_son) delete _son;
    _buffer=NULL;
    _son=NULL;
}
/**
    \fn getExtraData
*/
bool mkvAccessLatm::getExtraData(uint32_t *l, uint8_t **d)
{
    return latm.getExtraData(l,d);
}
/**
    \fn getDurationInUs
*/
uint64_t  mkvAccessLatm::getDurationInUs(void)
{
    return _son->getDurationInUs();
}
/**
    \fn goToTime
*/
bool mkvAccessLatm::goToTime(uint64_t timeUs)
{
    latm.flush();
    return _son->goToTime(timeUs);
}
/**
    \fn getPacket
*/
bool mkvAccessLatm::getPacket(uint8_t *dest, uint32_t *packLen, uint32_t maxSize, uint64_t *timecode)
{
    int retries=10;
    uint64_t time=ADM_NO_PTS;
    while(latm.empty()) // fetch next LOAS frame, it will contain several frames
    {
        if(!retries)
        {
            ADM_error("Cannot get AAC packet from LATM\n");
            return false;
        }
        if(ADM_latm2aac::LATM_MORE_DATA_NEEDED==latm.convert(time))
        {
            uint32_t len=0;
            if(!_son->getPacket(_buffer,&len,_maxSize,&time)) return false;
            if(false==latm.pushData(len,_buffer))
                latm.flush();
        }
        retries--;
    }
    latm.getData(&time,packLen,dest,maxSize);
    *timecode=time;
    return true;
}
//EOF

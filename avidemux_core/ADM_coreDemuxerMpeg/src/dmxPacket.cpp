/**
    \file dmxPacket
    \brief Packet demuxer for mpeg 
    copyright            : (C) 2005-2009 by mean
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

#include "dmxPacket.h"

/**
    \fn ADMMpegPacket
*/
ADMMpegPacket::ADMMpegPacket(void)
{
     doNoComplainAnyMore=0;
    _file=NULL;
    _size=0;

}
/**
    \fn ~ADMMpegPacket

*/
ADMMpegPacket::~ADMMpegPacket()
{
    if(_file) delete _file;
    _file=NULL;
}

/**
    \fn getPacketOfType
    \brief Only returns packet of type pid
*/      

bool        ADMMpegPacket::getPacketOfType(uint8_t pid,uint32_t maxSize, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt)
{
    uint8_t tmppid;
    while(1)
    {
        if(true!=getPacket(maxSize,&tmppid,packetSize,pts,dts,buffer,startAt))
                return false;
        else
                if(tmppid==pid) return true;
    }
    return false;
}
//EOF
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


#include "ADM_default.h"
#include "ADM_Video.h"

#include "fourcc.h"
#include "ADM_wtv.h"
/**
    \fn getDurationInUs
*/
uint64_t  wtvAudioAccess::getDurationInUs(void)
{
    return 0;
}
/**
    \fn wtvAudioAccess
*/

wtvAudioAccess::~wtvAudioAccess()
{
	printf("[asfAudio] Destroying track\n");

	fclose(_fd);
	_fd = NULL;
}

/**
    \fn wtvAudioAccess
*/

wtvAudioAccess::wtvAudioAccess(wtvHeader *father,uint32_t myRank)
{

}

uint64_t  wtvAudioAccess::getPos(void)
{
    return 0;
}

/**
    \fn setPos
*/

bool   wtvAudioAccess::setPos(uint64_t newoffset)
{

  return 1;
}

/**
    \fn goToTime
*/

bool   wtvAudioAccess::goToTime(uint64_t dts_us)
{

    return false;
}

/**
    \fn getPacket

*/
bool  wtvAudioAccess::getPacket(uint8_t *dest, uint32_t *len, uint32_t maxSize,uint64_t *dts)
{


  return 0;
}
//EOF

/***************************************************************************
                          TLK_clock.cpp  -  description
                             -------------------
	Handle a simple clock/timer class

	The API used returns the amound of days/hour/minute/seconds since
		1st Jan 1970
	As we only use part of the information (seconds and useconds) it
	may wrap around anytime

	The first time a clock is created, we store the "now date" and
	do the computation with getTime-"now date"
	It will wrap after 25 days, so it can be considerate safe


    begin                : Fri Apr 20 2003
    copyright            : (C) 2003 by mean
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include "ADM_default.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_CLOCKnTIMELEFT
#include "ADM_osSupport/ADM_debug.h"

static uint8_t inited=0;
static struct timeval _itimev;
static uint32_t getAbsTime( void );

#ifndef __WIN32
#include <unistd.h>

void ADM_usleep(unsigned long us)
{
  usleep(us);
}
#endif

Clock::Clock( void )
{
	reset();
}
Clock::~Clock(  )
{

}
uint32_t Clock::getElapsedMS(void )
{
   uint32_t ret = getAbsTime()-_startTime;
	//aprintf("Clock::getElapsedMS() -> %lu\n", ret);
	return ret;
}

/** Note:
*** * gettimeofday() returns seconds since 1.1.1970 in param1.tv_sec
*** * because we need a AbsTime with milliseconds for benchmarking we have:
***   max 0x7Fffffff (int32_t casted to uint32_t)
***    => 2147483647 msec => 2147483 sec => Sun Jan 25 21:31:23 1970
***   the return value will wrap all 25 days
*** * workaround:
***   1) start with an AbsTime of zero if object Clock() is created
***   2) finish your work within 25 days ;-)
**/
uint32_t getAbsTime( void )
{
     struct timeval timev;     
     TIMZ timez;

    int32_t tt;

    if(!inited)
    {
	gettimeofday(&_itimev, &timez);
	inited=1;
    }

    gettimeofday(&timev, &timez);
    tt = timev.tv_usec;
    tt /= 1000;
    tt += 1000 * (timev.tv_sec-_itimev.tv_sec);
    //aprintf("getAbsTime() -> %lu\n", tt&0x7Fffffff);
    return (tt&0x7Fffffff);

}
uint8_t Clock::reset(void)
{
	_startTime=getAbsTime();
}

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

#include "ADM_default.h"


static uint8_t inited=0;
static struct timeval _itimev;
static uint32_t getAbsTime( void );

#ifndef _WIN32
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

/**
 * 
 * @return 
 */
static uint64_t getAbsTimeUs( void )
{
     struct timeval timev;     
     TIMZ timez;

    int64_t tt;

    if(!inited)
    {
	gettimeofday(&_itimev, &timez);
	inited=1;
    }

    gettimeofday(&timev, &timez);
    int ref=_itimev.tv_usec;
    int now= timev.tv_usec;
    
    tt=now-ref; // can be negative        
    tt = timev.tv_usec;    
    tt += 1000 * 1000*(timev.tv_sec-_itimev.tv_sec);
    return tt;

}
/**
 * 
 * @return 
 */
uint32_t Clock::getElapsedMS(void )
{
    uint64_t t=getElapsedUS();
    return (uint32_t)(t/1000);
}
/**
 * 
 * @return 
 */
uint64_t Clock::getElapsedUS(void )
{
    return  getAbsTimeUs()-_startTimeUs;
}


uint8_t Clock::reset(void)
{
    _startTimeUs=getAbsTimeUs();
    return true;
}


//------------------------------------
/**
 * \fn ctor
 * @return 
 */
ADMCountdown::ADMCountdown(uint32_t valueMs)
{
    _running=false;
    _clockStartTime=valueMs;
}
/**
 * \fn dtor
 * @return 
 */
ADMCountdown::~ADMCountdown()
{
}
/**
 * \fn done
 * @return true if countdown elasped
 */
bool ADMCountdown::done()
{
    if(!_running) return false;
    if(_clock.getElapsedMS()>_clockStartTime) return true;
    return false;
}
/**
 * \fn reset
 * @brief reset countdown
 */
void ADMCountdown::reset()
{
        _running=true;
        _clock.reset();
}

// EOF

/***************************************************************************
                          ADM_audioClock.cpp  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#include "ADM_audioClock.h"
/**
 *
 * */
audioClock::audioClock(uint32_t fq)
{
        _nbSamples=0;
        _frequency=fq;
        _baseClock=0;

}
/**
 *
 * */
bool           audioClock::advanceBySample(uint32_t samples)
{
        _nbSamples+=samples;
        return true;
}
/**
 *
 * */
uint64_t       audioClock::getTimeUs(void)
{
        float f=_nbSamples;
                f=f*1000*1000;
                f/=_frequency;
                return _baseClock+(uint64_t)(f+0.5);
}
/**
 *
 * */
bool           audioClock::setTimeUs(uint64_t clk)
{
                uint64_t curTime=getTimeUs();
                int64_t delta=(int64_t)clk-(int64_t)curTime;
                if(labs((long int)delta)<2000) return true;
                printf("[audioClock] Drift detected :%"PRIu64" vs %"PRIu64", delta=%"PRId64"\n",curTime,clk,delta);
                _nbSamples=0;
                _baseClock=clk;
                return true; 


}

// EOF

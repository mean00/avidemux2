/** *************************************************************************
    \fn ADM_clock.h
    \brief Handle time class
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ADM_CLOCK_H
#define ADM_CLOCK_H

#include "ADM_core6_export.h"
/**
 *      \class Clock
 *      \brief a simple time counting class
 */
class ADM_CORE6_EXPORT Clock
{
	private: 
            uint64_t            _startTimeUs;

	public:
                                Clock(void );
                                ~Clock( );
			uint32_t getElapsedMS( void );
                        uint64_t getElapsedUS( void );
			uint8_t  reset( void );
};
/**
        \class ADMCountdown
        \brief a simple passive countdown clock (sort of timer)
*/
class ADM_CORE6_EXPORT  ADMCountdown
{
protected: 
            Clock       _clock;
            bool        _running;
            uint32_t    _clockStartTime;

public:
                    ADMCountdown(uint32_t valueMs );
                    ~ADMCountdown();
            bool    done( void );
            void    reset( void );
};
/**
    \class ADMBenchmark
    \brief a class to measure the time it takes to do something. Returns min/max/avg value
*/
class ADM_CORE6_EXPORT ADMBenchmark
{
protected:
        uint64_t bMin,bMax,bCumul;
        uint32_t nbRound;
        Clock    clk;
public:
             ADMBenchmark();
        void start(void);
        void end(void);
        void printResult(void);
        void getResult(float &avg, int &bmin,int &bmax);
        void getResultUs(float &avg, int &bmin,int &bmax);
};
#endif
//EOF

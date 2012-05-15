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
class Clock
{
	private: 
            uint32_t _startTime;

	public:
			Clock(void );
			~Clock( );
			uint32_t getElapsedMS( void );
			uint8_t reset( void );


};
/**
    \class ADMBenchmark
*/
class ADMBenchmark
{
protected:
        uint32_t bMin,bMax,bCumul;
        uint32_t nbRound;
        Clock    clk;
public:
        ADMBenchmark();
        void start(void);
        void end(void);
        void printResult(void);
        void getResult(float &avg, int &bmin,int &bmax);
};
#endif
//EOF

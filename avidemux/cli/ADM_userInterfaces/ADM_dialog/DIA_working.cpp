/***************************************************************************
                          DIA_working.cpp  -  description
                             -------------------
    begin                : Thu Apr 21 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

	This class deals with the working window

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

#include "config.h"
#include "ADM_default.h"

#define aprintf ADM_info

# include "config.h"
#include "DIA_working.h"

class DIA_workingNone : public DIA_workingBase
{
protected:
        virtual void 	    postCtor( void );
public:
                            DIA_workingNone( const char *title=NULL );
        virtual		        ~DIA_workingNone();
            // If returns 1 -> Means aborted
        virtual uint8_t  	update(uint32_t percent);
        virtual uint8_t 	update(uint32_t current,uint32_t total);
        virtual uint8_t  	isAlive (void );
        
};
//**********************************
DIA_workingNone::DIA_workingNone( const char *title ) : DIA_workingBase(title)
{
}
void DIA_workingNone :: postCtor( void )
{

		lastper=0;
		_nextUpdate=0;
}
uint8_t DIA_workingNone::update(uint32_t percent)
{
	#define  GUI_UPDATE_RATE 1000

                if(!_priv) return 1;
                if(!percent) return 0;
                if(percent==lastper)
                {

                   return 0;
                }
                aprintf("DIA_working::update(%lu) called\n", percent);
                elapsed=_clock.getElapsedMS();
                if(elapsed<_nextUpdate) 
                {

                  return 0;
                }
                _nextUpdate=elapsed+1000;
                lastper=percent;
  
        
		//
		// 100/totalMS=percent/elapsed
		// totalM=100*elapsed/percent

		double f;
		f=100.;
		f*=elapsed;
		f/=percent;

		f-=elapsed;
		f/=1000;

		uint32_t sectogo=(uint32_t)floor(f);

	char b[300];
   		int  mm,ss;
    			mm=sectogo/60;
      			ss=sectogo%60;
    			printf(" %d m %d s left ", mm,ss);

		double p;
			p=percent;
			p=p/100.;
                        printf("%% done : %f\n",p);
		return 0;


}
uint8_t DIA_workingNone::update(uint32_t cur, uint32_t total)
{
		double d,n;
		uint32_t percent;
		if(!_priv) return 1;

		aprintf("DIA_working::update(uint32_t %lu,uint32_t %lu) called\n", cur, total);
		if(!total) return 0;

		d=total;
		n=cur;
		n=n*100.;

		n=n/d;

		percent=(uint32_t )floor(n);
		return update(percent);

}

uint8_t DIA_workingNone::isAlive (void )
{
	if(!_priv) return 0;
	return 1;
}

DIA_workingNone::~DIA_workingNone()
{
	
        
}
namespace ADM_CliCoreUIToolkit
{
DIA_workingBase *createWorking(const char *title)
{
        return new DIA_workingNone(title);

}
}

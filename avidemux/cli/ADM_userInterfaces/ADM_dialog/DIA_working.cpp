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
    virtual void        postCtor( void );
public:
                        DIA_workingNone( const char *title=NULL );
    virtual             ~DIA_workingNone();
    // If returns 1 -> Means aborted
    virtual uint8_t     update(uint32_t percent);
    virtual uint8_t     update(uint32_t current,uint32_t total);
    virtual uint8_t     isAlive (void );
    virtual void        reuseAs( const char *title=NULL );

};
//**********************************
DIA_workingNone::DIA_workingNone( const char *title ) : DIA_workingBase(title)
{
    this->postCtor();
}

void DIA_workingNone :: postCtor( void )
{
    lastper=0;
    _nextUpdate=0;
}

uint8_t DIA_workingNone::update(uint32_t percent)
{
    if(!percent || percent==lastper)
        return 0;

    lastper=percent;
    //aprintf("DIA_working::update(%lu) called\n", percent);
    elapsed=_clock.getElapsedMS();
    if(elapsed==_nextUpdate)
        return 0;

    if(elapsed<1000)
    {
        printf("%*u%% done, elapsed: %*u ms\n",3,percent,4,elapsed);
        _nextUpdate=elapsed;
        return 0;
    }
    if(elapsed<_nextUpdate)
        return 0;
    _nextUpdate=elapsed+1000;

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
    printf("%*u%% done, %d m %*d s left\n",3,percent,mm,2,ss);

    return 0;

}

uint8_t DIA_workingNone::update(uint32_t cur, uint32_t total)
{
    double d,n;
    uint32_t percent;

    //aprintf("DIA_working::update(uint32_t %lu,uint32_t %lu) called\n", cur, total);
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
    return 1;
}

void DIA_workingNone::reuseAs( const char *title )
{
    lastper=0;
    _nextUpdate=0;
    _clock.reset();
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

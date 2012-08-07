/***************************************************************************
    \fn DIA_working.cpp 
    \brief UI that handles working state with cancel & percent

    copyright            : (C) 2003/2009 by mean fixounet@free.fr



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DIA_WK__
#define __DIA_WK__
#include "ADM_coreUI6_export.h"
#include "ADM_clock.h"
#include "ADM_assert.h"
/**
    \class DIA_workingBase
*/
class DIA_workingBase
{
    protected :
            uint32_t 	lastper;
            Clock	    _clock;
            uint32_t	_nextUpdate;
            uint32_t 	elapsed;
            
    public:
            void 		*_priv;
                        DIA_workingBase( const char *title=NULL ) {};
            virtual		~DIA_workingBase(){};
            // If returns 1 -> Means aborted
            virtual uint8_t  	update(uint32_t percent) {ADM_assert(0);return 1;}
            virtual uint8_t 	update(uint32_t current,uint32_t total){ADM_assert(0);return 1;};
            virtual uint8_t  	isAlive (void ){ADM_assert(0);return 1;};
            
};
/**
    \class DIA_working
*/
#if 0
class DIA_working : public DIA_workingBase
{
protected:
                void        *son;
        
public:
                            DIA_working( const char *title=NULL );
        virtual		        ~DIA_working();
            // If returns 1 -> Means aborted
        virtual uint8_t  	update(uint32_t percent);
        virtual uint8_t 	update(uint32_t current,uint32_t total);
        virtual uint8_t  	isAlive (void );
        
};
#endif
ADM_COREUI6_EXPORT DIA_workingBase *createWorking(const char *title);

#endif

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
#pragma once
#include "ADM_coreUI6_export.h"
#include "ADM_clock.h"
#include "ADM_assert.h"
/**
    \class DIA_workingBase
*/
class DIA_processingBase
{
    protected :
            Clock	_clock;
            uint32_t	_nextUpdate;
            uint32_t    _currentFrames;
            uint32_t    _lastFrames;
            uint32_t    _totalFrame;
            uint64_t    _totalToProcess;
            
    public:
            void 		*_priv;
                                DIA_processingBase( const char *title, uint64_t _totalToProcess ) {};
            virtual		~DIA_processingBase(){};
            virtual bool  	update(uint32_t frame,uint64_t processed) {return false;} // Return true = abort
            
};
ADM_COREUI6_EXPORT DIA_processingBase *createProcessing(const char *title,uint64_t totalToProcess);


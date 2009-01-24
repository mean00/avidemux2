/** *************************************************************************
             
    \fn ADM_default.h
    \brief include that file to get most of the basic types & functions
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_DEFAULT_H
#define ADM_DEFAULT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADM_coreConfig.h"
#include "ADM_inttype.h"
#include "ADM_assert.h"

#ifdef __cplusplus
#include "ADM_cpuCap.h"
#include "ADM_clock.h"
#include "ADM_misc.h"
#endif

#include "ADM_mangle.h"
#include "ADM_files.h"

#ifndef QT_TR_NOOP
#define QT_TR_NOOP(x) x
#endif

#endif

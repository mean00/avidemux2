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

#ifndef _WIN32 // needed for unlink
#include <unistd.h>
#endif

#include "ADM_core6_export.h"
#include "ADM_coreConfig.h"
#include "ADM_inttype.h"
#include "ADM_assert.h"
#define ADM_NO_PTS 0xffffffffffffffffLL
#ifdef __cplusplus
#include "ADM_cpuCap.h"
#include "ADM_clock.h"
#include "ADM_misc.h"
#endif

#ifdef __cplusplus
extern "C" 
{
#endif
ADM_CORE6_EXPORT void ADM_warning2(const char *f,const char *st, ...) ;
ADM_CORE6_EXPORT void ADM_info2(const char *f,const char *st, ...) ;
ADM_CORE6_EXPORT void ADM_error2(const char *f,const char *st, ...) ;

#define ADM_warning(a,...)  ADM_warning2(__FUNCTION__,a, ##__VA_ARGS__)
#define ADM_info(a,...)     ADM_info2(__FUNCTION__,a,    ##__VA_ARGS__)
#define ADM_error(a,...)    ADM_error2(__FUNCTION__,a,   ##__VA_ARGS__)

ADM_CORE6_EXPORT const char *ADM_translate(const char *domain, const char *stringToTranslate);

#ifdef __cplusplus
}
#endif



#include "ADM_mangle.h"
#include "ADM_files.h"


//

#undef QT_TR_NOOP
#undef QT_TRANSLATE_NOOP
#define QT_TR_NOOP(x) ADM_translate("adm",x)
#define QT_TRANSLATE_NOOP(a,x) ADM_translate(a,x)

#endif

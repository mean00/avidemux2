/** *************************************************************************
    \fn ADM_mangle.h
    \brief Handle symbol mangling & register name for inline asm
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/
#pragma once

#include "ADM_coreConfig.h"
#define MANGLE(a)  #a
#define LOCAL_MANGLE(a) #a
#define FUNNY_MANGLE(x) x
#define FUNNY_MANGLE_ARRAY(x, y) x[y] 
#define attribute_used
#define ASM_CONST attribute_used 
#define ADM_NO_OPTIMIZE 
#define ASM_ALIGNED(x) __declspec(align(x))

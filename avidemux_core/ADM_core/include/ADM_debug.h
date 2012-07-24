/***************************************************************************
                        Debug #define, ugly actually
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by mean
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
#if !defined(MODULE_NAME)
         #error define MODULE_NAME when using aprintf !
#endif

#define ADM_PRINT_ERROR         0
#define ADM_PRINT_INFO          1
#define ADM_PRINT_DEBUG         2
#define ADM_PRINT_VERY_VERBOSE  3

#if !defined(ADM_DEBUG)
         #define aprintf(prt, ...) ;
         #define adm_printf(level,prt,...) ;
#else
        #if (MODULE_NAME !=0x8000)
	#ifdef __cplusplus
	extern "C" {
	#endif
	 	void indirect_printf(int entity, const char *fmt, ...);
                void indirect_printf_long(int level,const char *modname,int entity, const char *prf, ...);
	#ifdef __cplusplus
	}
	#endif
			
                #define aprintf(prt, ...) indirect_printf(MODULE_NAME,prt, ## __VA_ARGS__)
                #define adm_printf(level,prt, ...) indirect_printf_long(level,__FILE__,MODULE_NAME,prt, ## __VA_ARGS__)
        #else
                #define aprintf(prt, ...) ;
                #define adm_printf(level,prt, ...) ;
        #endif
#endif


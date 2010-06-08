/**
    \file ADM_scriptUtil
    \brief Standard includes and defines
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_SCRIPT_UTIL_H
#define ADM_SCRIPT_UTIL_H
#ifdef __cplusplus
int scriptSetContainer(const char *container, CONFcouple *conf);
extern "C" {
#endif
int scriptTestCrash(void);
int scriptTestAssert(void);
#ifdef __cplusplus
}
#endif

#endif
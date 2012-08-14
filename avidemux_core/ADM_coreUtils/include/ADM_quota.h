/** *************************************************************************
    \fn ADM_quota.h
    \brief Handle disk full etc
                      
    copyright            : (C) 2008 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_quota_h
#define ADM_quota_h

#include "ADM_coreUtils6_export.h"
#include <stdio.h>

#ifdef _MSC_VER
#	include <windows.h>

	typedef SSIZE_T ssize_t;
#endif

#include "ADM_coreConfig.h"

/* qfopen stands for quota-fopen() */

ADM_COREUTILS6_EXPORT FILE *qfopen(const char *, const char *);
#ifdef __cplusplus
ADM_COREUTILS6_EXPORT FILE *qfopen(const std::string &name, const char *);
#endif
/* qfprintf stands for quota-fprintf() */
ADM_COREUTILS6_EXPORT void qfprintf(FILE *, const char *, ...);
size_t qfwrite(const void *ptr, size_t size, size_t  nmemb, FILE *stream);
ssize_t qwrite(int fd, const void *buf, size_t numbytes);

/* qfclose stands for quota-fclose() */
ADM_COREUTILS6_EXPORT int qfclose(FILE *);

ADM_COREUTILS6_EXPORT uint8_t  quotaInit(void);

#endif

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

#include <stdio.h>
#include <unistd.h>
#include "ADM_coreConfig.h"

#ifdef __cplusplus
#include <string>
using std::string;
FILE *qfopen(const string &name, const char *);
#endif
/* qfopen stands for quota-fopen() */

FILE *qfopen(const char *, const char *);

/* qfprintf stands for quota-fprintf() */
void qfprintf(FILE *, const char *, ...);
size_t qfwrite(const void *ptr, size_t size, size_t  nmemb, FILE *stream);
ssize_t qwrite(int fd, const void *buf, size_t numbytes);

/* qfclose stands for quota-fclose() */
int qfclose(FILE *);


#endif

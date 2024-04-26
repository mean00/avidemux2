/** *************************************************************************
    \fn ADM_memFile.h
    \brief temporary file in memory
                      
    copyright            : (C) 2008 by mean
                               2024 szlldm
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ADM_memFile_h
#define ADM_memFile_h

#include "ADM_coreUtils6_export.h"
#include <stdio.h>

#ifdef _MSC_VER
#	include <windows.h>

	typedef SSIZE_T ssize_t;
#endif

#include "ADM_coreConfig.h"

typedef struct
{
    char * buf;
    ssize_t seek;
    size_t length;
    size_t allocated;
} MFILE;

ADM_COREUTILS6_EXPORT MFILE *mfopen(const char *, const char *);
#ifdef __cplusplus
ADM_COREUTILS6_EXPORT MFILE *mfopen(const std::string &name, const char *);
#endif

ADM_COREUTILS6_EXPORT void mfprintf(MFILE *, const char *, ...);
size_t mfwrite(const void *ptr, size_t size, size_t  nmemb, MFILE *stream);

ADM_COREUTILS6_EXPORT int mfseek (MFILE * stream, ssize_t offset, int origin);
ADM_COREUTILS6_EXPORT char * mfgets (char * str, int num, MFILE * stream);

ADM_COREUTILS6_EXPORT int mfclose(MFILE *);     // dummy close!

ADM_COREUTILS6_EXPORT void mfcleanup(const char * name);
#ifdef __cplusplus
ADM_COREUTILS6_EXPORT void mfcleanup(const std::string &name);
#endif


ADM_COREUTILS6_EXPORT uint8_t  memFileInit(void);

#endif

/** *************************************************************************
             
    \fn ADM_filest.h
    \brief Helpers function to access configuration files
                      
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
#ifndef ADM_FILES_H
#define ADM_FILES_H

#include "ADM_core6_export.h"

#ifdef __cplusplus
ADM_CORE6_EXPORT void ADM_initBaseDir(bool portableMode);
#endif
// Returns dir to ~/.avidemux, no need to free it
ADM_CORE6_EXPORT char *ADM_getBaseDir(void);
// Returns dir to ~/.avidemux/jobs, no need to free it
ADM_CORE6_EXPORT char *ADM_getJobDir(void);
// Returns dir to ~/.avidemux/custom, no need to free it
ADM_CORE6_EXPORT char *ADM_getCustomDir(void);
// Returns dir to ~/.avidemux/autoScript, no need to free it
ADM_CORE6_EXPORT char *ADM_getAutoDir(void);
// Returns dir to ~/.avidemux/autoScript, no need to free it
ADM_CORE6_EXPORT const char *ADM_getUserPluginSettingsDir(void);
ADM_CORE6_EXPORT const char *ADM_getSystemPluginSettingsDir(void);
//
ADM_CORE6_EXPORT uint8_t ADM_copyFile(const char *source, const char *target);
ADM_CORE6_EXPORT uint8_t ADM_renameFile(const char *source, const char *target);
#ifdef __cplusplus
/* Returns the full path relative to install dir i.e. /usr +base1/base2, needs to be deleted [] by caller */
ADM_CORE6_EXPORT char *ADM_getInstallRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
/* Returns the full path relative to .avidemux dir i.e. /home/fx/... +base1/base2 needs to be deleted []*/
ADM_CORE6_EXPORT char *ADM_getHomeRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
#endif
ADM_CORE6_EXPORT uint8_t buildDirectoryContent(uint32_t *outnb,const char *base, char *jobName[],int maxElems,const char *ext);
ADM_CORE6_EXPORT uint8_t clearDirectoryContent(const uint32_t nb, char *jobName[]);
#endif

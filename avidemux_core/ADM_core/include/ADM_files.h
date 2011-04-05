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
// Returns dir to ~/.avidemux, no need to free it
char *ADM_getBaseDir(void);
// Returns dir to ~/.avidemux/jobs, no need to free it
char *ADM_getJobDir(void);
// Returns dir to ~/.avidemux/custom, no need to free it
char *ADM_getCustomDir(void);
// Returns dir to ~/.avidemux/autoScript, no need to free it
char *ADM_getAutoDir(void);
// Returns dir to ~/.avidemux/autoScript, no need to free it
const char *ADM_getUserPluginSettingsDir(void);
const char *ADM_getSystemPluginSettingsDir(void);
//
uint8_t ADM_copyFile(const char *source, const char *target);
#ifdef __cplusplus
/* Returns the full path relative to install dir i.e. /usr +base1/base2, needs to be deleted [] by caller */
char *ADM_getInstallRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
/* Returns the full path relative to .avidemux dir i.e. /home/fx/... +base1/base2 needs to be deleted []*/
char *ADM_getHomeRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
#endif
uint8_t buildDirectoryContent(uint32_t *outnb,const char *base, char *jobName[],int maxElems,const char *ext);

#endif

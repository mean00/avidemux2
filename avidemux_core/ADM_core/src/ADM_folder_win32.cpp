/***************************************************************************
                    
    copyright            : (C) 2006 by mean
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

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string>


#	include <io.h>
#	include <direct.h>
#	include <shlobj.h>
#	include "ADM_win32.h"

#include "ADM_default.h"
char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);
static char ADM_basedir[1024] = {0};
static char *ADM_autodir = NULL;
static char *ADM_systemPluginSettings=NULL;


#undef fread
#undef fwrite
#undef fopen
#undef fclose

/**
    \fn ADM_getAutoDir
    \brief  Get the  directory where auto script are stored. No need to free the string.
******************************************************/
char *ADM_getAutoDir(void)
{
    if (ADM_autodir )
        return ADM_autodir;
    const char *startDir=ADM_RELATIVE_LIB_DIR;
    ADM_autodir = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "autoScripts");
	return ADM_autodir;
}
/**
    \fn ADM_getPluginSettingsDir
    \brief Get the folder containing the plugin settings (presets etc..)
*/
const char *ADM_getSystemPluginSettingsDir(void)
{
    if(ADM_systemPluginSettings) return ADM_systemPluginSettings;
    const char *startDir=ADM_RELATIVE_LIB_DIR;
    ADM_systemPluginSettings=ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "pluginSettings");
    return ADM_systemPluginSettings;
}


static void AddSeparator(char *path)
{
	if (path && (strlen(path) < strlen(ADM_SEPARATOR) || strncmp(path + strlen(path) - strlen(ADM_SEPARATOR), ADM_SEPARATOR, strlen(ADM_SEPARATOR)) != 0))
		strcat(path, ADM_SEPARATOR);
}

/**
 * 	\fn char *ADM_getHomeRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
 *  \brief Returns home directory +base 1 + base 2... The return value is a copy, and must be deleted []
 */
char *ADM_getHomeRelativePath(const char *base1, const char *base2, const char *base3)
{
	return ADM_getRelativePath(ADM_getBaseDir(), base1, base2, base3);
}

char *ADM_getInstallRelativePath(const char *base1, const char *base2, const char *base3)
{

	wchar_t wcModuleName[MAX_PATH];

	GetModuleFileNameW(0, wcModuleName, sizeof(wcModuleName) / sizeof(wchar_t));

	int len = wideCharStringToUtf8(wcModuleName, -1, NULL);
	char *moduleName = new char[len];

	wideCharStringToUtf8(wcModuleName, -1, moduleName);

	char *slash = strrchr(moduleName, '\\');
		
	if (slash)
		*slash = '\0';

	char *relativePath = ADM_getRelativePath(moduleName, base1, base2, base3);

	delete [] moduleName;

	return relativePath;
}

/*
      Get the root directory for .avidemux stuff
******************************************************/
char *ADM_getBaseDir(void)
{
	return ADM_basedir;
}

/**
 * \fn ADM_initBaseDir
 * \brief ADM_initBaseDir
 */
void ADM_initBaseDir(int argc, char *argv[])
{
	char *home = NULL;

    bool portableMode=isPortableMode(argc,argv);
	// Get the base directory

    if (portableMode)
    {
        // Portable mode...
        home = ADM_getInstallRelativePath(NULL, NULL, NULL);
    }
	else
    {
        wchar_t wcHome[MAX_PATH];

        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, wcHome) == S_OK)
        {
            int len = wideCharStringToUtf8(wcHome, -1, NULL);
            home = new char[len];

            wideCharStringToUtf8(wcHome, -1, home);
        }
        else
        {
            printf("Oops: can't determine the Application Data folder.");
            home=new char[10];
            strcpy(home,"c:\\");
        }
    }


	// Try to open the .avidemux directory

	if (home)
	{
		strcpy(ADM_basedir, home);
		AddSeparator(ADM_basedir);


		const char *ADM_DIR_NAME;

		if (portableMode)
			ADM_DIR_NAME = "settings";
		else
			ADM_DIR_NAME = "avidemux";

		strcat(ADM_basedir, ADM_DIR_NAME);
		strcat(ADM_basedir, ADM_SEPARATOR);

		delete [] home;

		if (ADM_mkdir(ADM_basedir))
		{
			printf("Using %s as base directory for prefs, jobs, etc.\n", ADM_basedir);
		}
		else
		{
			ADM_error("Oops: cannot create the .avidemux directoryi (%s)\n", ADM_basedir);
		}
	}
}



// EOF

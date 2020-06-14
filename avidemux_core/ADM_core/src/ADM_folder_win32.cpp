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


#include <errno.h>
#include <string>
#include <io.h>
#include <direct.h>
#include <shlobj.h>
#include "ADM_win32.h"

#include "ADM_default.h"

extern void simplify_path(char **buf);
extern char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);
static char ADM_basedir[1024] = {0};
static char ADM_logdir[1024] = {0};
static std::string ADM_autodir;
static std::string ADM_systemPluginSettings;

#undef fread
#undef fwrite
#undef fopen
#undef fclose

/**
    \fn ADM_getAutoDir
    \brief  Get the  directory where auto script are stored. No need to free the string.
******************************************************/
const std::string ADM_getAutoDir(void)
{
    if (ADM_autodir.size())
        return ADM_autodir;
    const char *startDir=ADM_RELATIVE_LIB_DIR;
    const char *s = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "autoScripts");
    ADM_autodir = std::string(s);
    delete [] s;
    s=NULL;
    return ADM_autodir;
}
/**
    \fn ADM_getPluginSettingsDir
    \brief Get the folder containing the plugin settings (presets etc..)
*/
const std::string ADM_getSystemPluginSettingsDir(void)
{
    if(ADM_systemPluginSettings.size())
        return ADM_systemPluginSettings;
    const char *startDir=ADM_RELATIVE_LIB_DIR;
    const char *s = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "pluginSettings");
    ADM_systemPluginSettings = std::string(s);
    delete [] s;
    s=NULL;
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


/*
      Get the root directory for .avidemux stuff
******************************************************/
const char *ADM_getBaseDir(void)
{
	return ADM_basedir;
}

/*
      Get the root directory for .avidemux stuff
******************************************************/
const char *ADM_getLogDir(void)
{
	return ADM_logdir;
}

/**
 * \fn ADM_initBaseDir
 * \brief ADM_initBaseDir
 */
void ADM_initBaseDir(int argc, char *argv[])
{
	char *home = NULL;
	char *log = NULL;

    bool portableMode=isPortableMode(argc,argv);
	// Get the base directory

    if (portableMode)
    {
        // Portable mode...
        home = ADM_getInstallRelativePath(NULL, NULL, NULL);
        log = ADM_getInstallRelativePath(NULL, NULL, NULL);
    }
	else
    {
        wchar_t wcHome[MAX_PATH];
        wchar_t wcLog[MAX_PATH];

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
        
        if (SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, wcLog) == S_OK)
        {
            int len = wideCharStringToUtf8(wcLog, -1, NULL);
            log = new char[len];

            wideCharStringToUtf8(wcLog, -1, log);
        }
        else
        {
            printf("Oops: can't determine the Local Application Data folder.");
            log=new char[10];
            strcpy(log,"c:\\");
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
	
	if (log)
	{
		strcpy(ADM_logdir, log);
		AddSeparator(ADM_logdir);


		const char *ADM_DIR_NAME;

		if (portableMode)
			ADM_DIR_NAME = "settings";
		else
			ADM_DIR_NAME = "avidemux";

		strcat(ADM_logdir, ADM_DIR_NAME);
		strcat(ADM_logdir, ADM_SEPARATOR);

		delete [] log;

		if (ADM_mkdir(ADM_logdir))
		{
			printf("Using %s as log directory.\n", ADM_logdir);
		}
		else
		{
			ADM_error("Oops: cannot create the log directory (%s)\n", ADM_logdir);
		}
	}
}
/**
 * \fn ADM_getI8NDir
 */
const std::string ADM_getI8NDir(const std::string &flavor)
{
    //
    char *home = ADM_getInstallRelativePath(flavor.c_str(), "i18n", NULL);
    std::string r=std::string(home);
    delete [] home;
    return r;
}
//--- 8< 8< 8<

#	include <io.h>
#	include <direct.h>
#	include <shlobj.h>
#	include <fcntl.h>
#	include "ADM_win32.h"

/*

** note: it modifies it's first argument
*/
void simplify_path(char **buf)
{
	unsigned int last1slash = 0;
	unsigned int last2slash = 0;

	while (!strncmp(*buf, "/../", 4))
		memmove(*buf, *buf + 3, strlen(*buf + 3) + 1);

	for (unsigned int i = 0; i < strlen(*buf) - 2; i++)
		while (!strncmp(*buf + i, "/./", 3))
			memmove(*buf + i, *buf + i + 2, strlen(*buf + i + 2) + 1);

	for (unsigned int i = 0; i < strlen(*buf) - 3; i++)
	{
		if (*(*buf + i) == '/')
		{
			last2slash = last1slash;
			last1slash = i;
		}

		if (!strncmp(*buf + i, "/../", 4))
		{
			memmove(*buf + last2slash, *buf + i + 3, strlen(*buf + i + 3) + 1);

			return simplify_path(buf);
		}
	}
}


/**
        \fn ADM_PathCanonize
        \brief Canonize the path, returns a copy of the absolute path given as parameter
*/
char *ADM_PathCanonize(const char *tmpname)
{
	char path[300];
	char *out;

	if (!getcwd(path, 300))
	{
		fprintf(stderr, "\ngetcwd() failed with: %s (%u)\n", strerror(errno), errno);
		path[0] = '\0';
	}

	if (!tmpname || tmpname[0] == 0)
	{
		out = new char[strlen(path) + 2];
		strcpy(out, path);
#ifndef _WIN32
		strcat(out, "/");
#else
		strcat(out, "\\");
#endif
		printf("\n Canonizing null string ??? (%s)\n", out);
	}
	else if (tmpname[0] == '/'
#if defined(_WIN32)
		|| tmpname[1] == ':'
#endif
		)
	{
		out = new char[strlen(tmpname) + 1];
		strcpy(out, tmpname);

		return out;
	}
	else
	{
		out = new char[strlen(path) + strlen(tmpname) + 6];
		strcpy(out, path);
#ifndef _WIN32
		strcat(out, "/");
#else
		strcat(out, "\\");
#endif
		strcat(out, tmpname);
	}

	simplify_path(&out);

	return out;
}

/**
        \fn ADM_PathStripName
	\brief Returns path only /foo/bar.avi -> /foo INPLACE, no copy done

*/
std::string ADM_extractPath(const std::string &str)
{
    std::string p;
         p=str;
         size_t idx=p.find_last_of ("\\");
         if(idx!=std::string::npos)
            p.resize(idx);
         return p;
}

/**
    \fn ADM_GetFileName
    \brief Get the filename without path. /foo/bar.avi -> bar.avi INPLACE, NO COPY

*/
const std::string ADM_getFileName(const std::string &str)
{
    size_t idx=str.find_last_of ("\\");
    size_t idx2=str.find_last_of ("/");


    // no / nor \

    if(idx2==std::string::npos && idx==std::string::npos)
        return str;
    // Both, take the further one
    if(idx2!=std::string::npos && idx!=std::string::npos)
        if(idx2>idx)
            return str.substr(idx2+1);
        else
            return str.substr(idx+1);

    // Only one found
    if(idx2!=std::string::npos)
         return str.substr(idx2+1);
    return str.substr(idx+1);
}

/**
    \fn ADM_eraseFile
    \brief utf8-capable unlink(), so that we can use utf8 string even on win32
*/
uint8_t ADM_eraseFile(const char *file)
{
    int fileNameLength = utf8StringToWideChar(file, -1, NULL);
    wchar_t *wcFile = new wchar_t[fileNameLength];

    utf8StringToWideChar(file, -1, wcFile);

    bool r = DeleteFileW(wcFile);
    delete [] wcFile;
    if(!r)
        return 0;
    return 1;
}

// EOF

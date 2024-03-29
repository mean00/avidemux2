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
#include <Carbon/Carbon.h>
#include <unistd.h>

#include "ADM_default.h"
extern char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);

#define MAX_PATH_SIZE 1024

static char ADM_basedir[MAX_PATH_SIZE] = {0};
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

    const char *startDir="../lib";
    const char *s = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "autoScripts");
    ADM_autodir = s;
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

    const char *startDir="../lib";
    const char *s = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR, "pluginSettings");
    ADM_systemPluginSettings = s;
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
 *     \fn char *ADM_getHomeRelativePath(const char *base1, const char *base2=NULL,const char *base3=NULL);
 *  \brief Returns home directory +base 1 + base 2... The return value is a copy, and must be deleted []
 */
char *ADM_getHomeRelativePath(const char *base1, const char *base2, const char *base3)
{
    return ADM_getRelativePath(ADM_getBaseDir(), base1, base2, base3);
}

char *ADM_getInstallRelativePath(const char *base1, const char *base2, const char *base3)
{
    char buffer[MAX_PATH_SIZE];

    CFURLRef url(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    buffer[0] = '\0';

    if (url)
    {
        CFURLGetFileSystemRepresentation(url, true, (UInt8*)buffer, MAX_PATH_SIZE);
        CFRelease(url);

        char *slash = strrchr(buffer, '/');
        
        if (slash)
            *slash = '\0';
    }

    return ADM_getRelativePath(buffer, base1, base2, base3);
}

/*
      Get the root directory for .avidemux stuff
******************************************************/
const char *ADM_getBaseDir(void)
{
    return ADM_basedir;
}

/**
 * \fn ADM_getConfigBaseDir
 * \brief Get the root directory for avidemux configuration
 */
const char *ADM_getConfigBaseDir(void)
{
    return ADM_basedir;
}
/**
 */
void ADM_initBaseDir(int argc, char *argv[])
{
    // Get the base directory

    const char* homeEnv = getenv("HOME");

    if (!homeEnv)
    {
        ADM_warning("Oops: can't determine $HOME.");
        return;
    }
    // Try to open the .avidemux directory

    strcpy(ADM_basedir, homeEnv);
    AddSeparator(ADM_basedir);

    const char *ADM_DIR_NAME = ".avidemux6";

    strcat(ADM_basedir, ADM_DIR_NAME);
    strcat(ADM_basedir, ADM_SEPARATOR);

    if (ADM_mkdir(ADM_basedir))
    {
        ADM_info("Using \"%s\" as base directory for prefs, jobs, etc.\n", ADM_basedir);
    }
    else
    {
        ADM_error("Oops: cannot create the .avidemux directory (\"%s\")\n", ADM_basedir);
    }
}
/**
 * \fn ADM_getI8NDir
 */
const std::string ADM_getI8NDir(const std::string &flavor)
{
    std::string partialPath = flavor;
    partialPath += "/i18n";
#ifdef CREATE_BUNDLE
    char *ppath=ADM_getInstallRelativePath("../Resources/share","avidemux6",partialPath.c_str());
#else
    char *ppath=ADM_getInstallRelativePath("../share","avidemux6",partialPath.c_str());
#endif
    std::string r = ppath;
    delete [] ppath;
    ppath=NULL;
    return  r;
    
}

#include "ADM_folder_unix.cpp"
// EOF

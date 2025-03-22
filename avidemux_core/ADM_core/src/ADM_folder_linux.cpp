/***************************************************************************
                    
    copyright            : (C) 2006/2016 by mean
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
#include <stdlib.h>
#include <unistd.h>
#include "ADM_default.h"

static bool isPortable=false;

extern void ADM_setPluginDir(std::string path);
extern char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);

#define PATH_BUF_LEN 1024
static char ADM_basedir[PATH_BUF_LEN] = {0};
static char ADM_configdir[PATH_BUF_LEN] = {0};

static std::string ADM_installPath;
static std::string ADM_autodir;
static std::string ADM_systemPluginSettings;
static std::string ADM_i18nDir;
/**
 * 
 * @param in
 * @return 
 */
static std::string canonize(const std::string &in)
{
    std::string out;
    char *simple2=canonicalize_file_name(in.c_str());
    if(simple2)
    {
        out=std::string(simple2);
        free(simple2);simple2=NULL;
    }else
    {
         out=in;
    }
    if(out.size() && *out.rbegin() != '/')
        out+=std::string("/");
    return out;
}


static void AddSeparator(char *path)
{
	if (path && (strlen(path) < strlen(ADM_SEPARATOR) || strncmp(path + strlen(path) - strlen(ADM_SEPARATOR), ADM_SEPARATOR, strlen(ADM_SEPARATOR)) != 0))
		strcat(path, ADM_SEPARATOR);
}

/**
    \fn ADM_getAutoDir
    \brief  Get the  directory where auto script are stored.
******************************************************/
const std::string ADM_getAutoDir(void)
{
    if (ADM_autodir.size())
        return ADM_autodir;

    const char *name="autoScripts";
    if(isPortable)
    {
        ADM_autodir = ADM_getPluginDir();
        ADM_autodir += name;
    }else
    {
        const char *s = ADM_getInstallRelativePath(ADM_RELATIVE_LIB_DIR, ADM_PLUGIN_DIR, name);
        ADM_autodir = s;
        delete [] s;
        s=NULL;
    }
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

    const char *name="pluginSettings";
    if(isPortable)
    {
        ADM_systemPluginSettings = ADM_getPluginDir();
        ADM_systemPluginSettings += name;
    }else
    {
        const char *s = ADM_getInstallRelativePath(ADM_RELATIVE_LIB_DIR, ADM_PLUGIN_DIR, name);
        ADM_systemPluginSettings = s;
        delete [] s;
        s=NULL;
    }
    return ADM_systemPluginSettings;
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
	return ADM_getRelativePath(ADM_INSTALL_DIR, base1, base2, base3);
}
const std::string ADM_getI8NDir(const std::string &flavor)
{
    if(ADM_i18nDir.size())
        return ADM_i18nDir;

    if(isPortable)
    {
        if (ADM_installPath.size())
        {
            std::string i18n = ADM_installPath;
            i18n += "share/avidemux6/";
            i18n += flavor;
            i18n += "/i18n";
            ADM_i18nDir = canonize(i18n);
        }
    }else
    {
        std::string partialPath = flavor;
        partialPath += "/i18n";
        char *ppath=ADM_getInstallRelativePath("share","avidemux6",partialPath.c_str());
        ADM_i18nDir = ppath;
        delete [] ppath;
        ppath=NULL;
    }
    return ADM_i18nDir;
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
    return ADM_configdir;
}
/**
 * 
 * @param argc
 * @param argv
 */

void ADM_initBaseDir(int argc, char *argv[])
{
    const char *admDirName = "avidemux6";
    char* homeEnv = NULL;

    homeEnv = getenv("XDG_DATA_HOME");

    if (homeEnv)
    {
        if (strlen(homeEnv) + strlen(admDirName) >= PATH_BUF_LEN)
        {
            ADM_warning("Path to XDG_DATA_HOME too long\n");
            return;
        }
        strcpy(ADM_basedir, homeEnv);
    } else
    {
        homeEnv = getenv("HOME");
        if (!homeEnv)
        {
            ADM_warning("Cannot locate HOME\n");
            return;
        }
        const char *suffix = "/.local/share";

        if (strlen(homeEnv) + strlen(suffix) + (strlen(ADM_SEPARATOR) * 2) + strlen(admDirName) >= PATH_BUF_LEN)
        {
            ADM_warning("Path to HOME too long\n");
            return;
        }
        strcpy(ADM_basedir, homeEnv);
        strcat(ADM_basedir, suffix);
    }

    homeEnv = getenv("XDG_CONFIG_HOME");

    if (homeEnv)
    {
        if (strlen(homeEnv) + strlen(admDirName) >= PATH_BUF_LEN)
        {
            ADM_warning("Path to XDG_CONFIG_HOME too long\n");
            return;
        }
        strcpy(ADM_configdir, homeEnv);
    } else
    {
        homeEnv = getenv("HOME");
        if (!homeEnv)
        {
            ADM_warning("Cannot locate HOME\n");
            return;
        }
        const char *suffix = "/.config";

        if (strlen(homeEnv) + strlen(suffix) + (strlen(ADM_SEPARATOR) * 2) + strlen(admDirName) >= PATH_BUF_LEN)
        {
            ADM_warning("Path to HOME too long\n");
            return;
        }
        strcpy(ADM_configdir, homeEnv);
        strcat(ADM_configdir, suffix);
    }

    AddSeparator(ADM_basedir);
    AddSeparator(ADM_configdir);

    strcat(ADM_basedir, admDirName);
    strcat(ADM_configdir, admDirName);
    strcat(ADM_basedir, ADM_SEPARATOR);
    strcat(ADM_configdir, ADM_SEPARATOR);

    if (ADM_mkdir(ADM_basedir))
    {
        ADM_info("Using \"%s\" as base directory for settings, jobs etc.\n", ADM_basedir);
    } else
    {
        ADM_error("Cannot create avidemux data directory (\"%s\")\n", ADM_basedir);
    }
    if (ADM_mkdir(ADM_configdir))
    {
        ADM_info("Using \"%s\" as base directory for prefs.\n", ADM_configdir);
    } else
    {
        ADM_error("Cannot create avidemux prefs directory (\"%s\")\n", ADM_configdir);
    }
    if(isPortableMode(argc,argv))
    {
        ADM_info("Portable mode\n");
        isPortable=true;
        char *copy=ADM_PathCanonize(argv[0]);
        std::string p=ADM_extractPath(copy);
        delete [] copy;copy=NULL;
        std::string plugins=p;
        plugins += "/../";
        ADM_installPath = canonize(plugins);
        plugins += ADM_RELATIVE_LIB_DIR;
        plugins += "/";
        plugins += ADM_PLUGIN_DIR;
        plugins = canonize(plugins);
        ADM_setPluginDir(plugins);
        ADM_info("Relative to install plugin mode : <%s>\n",plugins.c_str());
    }
}
#include "ADM_folder_unix.cpp"
// EOF

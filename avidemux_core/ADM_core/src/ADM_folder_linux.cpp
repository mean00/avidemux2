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
extern std::string pluginDir;

extern char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);

static char ADM_basedir[1024] = {0};

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
        ADM_autodir=pluginDir+std::string(name);
    }else
    {
        const char *s = ADM_getInstallRelativePath(ADM_RELATIVE_LIB_DIR, ADM_PLUGIN_DIR, name);
        ADM_autodir=std::string(s);
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
        ADM_systemPluginSettings=pluginDir+std::string(name);
    }else
    {
        const char *s = ADM_getInstallRelativePath(ADM_RELATIVE_LIB_DIR, ADM_PLUGIN_DIR, name);
        ADM_systemPluginSettings=std::string(s);
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
    //
    // 181n
    if(isPortable)
    {
        std::string i18n=pluginDir;
        i18n+=std::string("/../../share/avidemux6/")+flavor+std::string("/i18n");
        ADM_i18nDir=canonize(i18n);
        ADM_info("Relative to install i18n mode : <%s>\n",ADM_i18nDir.c_str());
        // 181n
    }else
    {        
        std::string partialPath=flavor+std::string("/i18n");
        char *ppath=ADM_getInstallRelativePath("share","avidemux6",partialPath.c_str());
        ADM_i18nDir=std::string(ppath);
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
 * 
 * @param argc
 * @param argv
 */

void ADM_initBaseDir(int argc, char *argv[])
{
	const char* homeEnv = getenv("HOME");

	if (!homeEnv)
	{
        ADM_warning("Cannot locate HOME...\n");
        return;
    }
    strcpy(ADM_basedir, homeEnv);
    AddSeparator(ADM_basedir);

    const char *ADM_DIR_NAME = ".avidemux6";

    strcat(ADM_basedir, ADM_DIR_NAME);
    strcat(ADM_basedir, ADM_SEPARATOR);

    if (ADM_mkdir(ADM_basedir))
    {
        printf("Using %s as base directory for prefs, jobs, etc.\n", ADM_basedir);
    }
    else
    {
        ADM_error("Oops: cannot create the .avidemux directoryi (%s)\n", ADM_basedir);
    }	
    
    if(isPortableMode(argc,argv))
    {
        ADM_info("Portable mode\n");
        isPortable=true;
        char *copy=ADM_PathCanonize(argv[0]);
        std::string p=ADM_extractPath(copy);
        delete [] copy;copy=NULL;
        std::string plugins=p;
        plugins+=std::string("/../lib/")+std::string(ADM_PLUGIN_DIR);
        pluginDir=canonize(plugins);
        ADM_info("Relative to install plugin mode : <%s>\n",pluginDir.c_str());
    }
    
}
#include "ADM_folder_unix.cpp"
// EOF

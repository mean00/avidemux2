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

#if defined(__APPLE__)
#	include <Carbon/Carbon.h>
#else
#	include <fcntl.h>
#endif

#ifdef _WIN32
#	include <io.h>
#	include <direct.h>
#	include <shlobj.h>
#	include "ADM_win32.h"
#else
#	include <unistd.h>
#endif

#include "ADM_default.h"


#undef fread
#undef fwrite
#undef fopen
#undef fclose
static void AddSeparator(char *path)
{
	if (path && (strlen(path) < strlen(ADM_SEPARATOR) || strncmp(path + strlen(path) - strlen(ADM_SEPARATOR), ADM_SEPARATOR, strlen(ADM_SEPARATOR)) != 0))
		strcat(path, ADM_SEPARATOR);
}
size_t ADM_fread (void *ptr, size_t size, size_t n, FILE *sstream)
{
	return fread(ptr,size,n,sstream);
}

size_t ADM_fwrite(const void *ptr, size_t size, size_t n, FILE *sstream)
{
	return fwrite(ptr,size,n,sstream);
}
/**
    \fn ADM_eraseFile
*/
uint8_t ADM_eraseFile(const char *file)
{
    if(!unlink(file))
        return true;
    return false;
}
/**
    \fn ADM_fileSize
    \brief return filesize, -1 on error
*/
int64_t ADM_fileSize(const char *file)
{
    FILE *f=ADM_fopen(file,"r");
    if(!f) return -1;
    fseeko(f,0,SEEK_END);
    int64_t v=ftello(f);
    fclose(f);
    return v;
}

int ADM_fclose(FILE *file)
{
	return fclose(file); 
}


#ifdef _WIN32
#define DIR _WDIR
#define dirent _wdirent
#define opendir _wopendir
#define readdir _wreaddir
#define closedir _wclosedir
#endif

/**
    \fn clearDirectoryContent
*/
uint8_t    clearDirectoryContent(const uint32_t nb, char *jobName[])
{
    for(int i=0;i<nb;i++)
        if(jobName[i])
        {
            ADM_dealloc(jobName[i]);
            jobName[i]=NULL;
        }
    return true;
}
/**
 * \fn ADM_PathSplit
 * \brief std::string version of the above
 * @param in
 * @param root
 * @param ext
 */
void            ADM_PathSplit(const std::string &in,std::string &root, std::string &ext)
{
  	char *full;
    std::string canonized;

	full = ADM_PathCanonize(in.c_str());
    canonized=std::string(full);
    delete [] full;full=NULL;
    
    size_t pos=canonized.find_last_of(".");
    
    
    // no "." ?
    if(pos==std::string::npos)
      {
        root=canonized;
        ext=std::string("");
        return;
      }
    
    // else split
    root=canonized.substr(0,pos);
    ext=canonized.substr(pos+1);
}
/**
    \fn ADM_copyFile
*/
uint8_t ADM_copyFile(const char *source, const char *target)
{
    FILE *fin=ADM_fopen(source,"rb");
    if(!fin)
    {
        ADM_error("Cannot open %s for reading\n",source);
        return false;
    }
    FILE *fout=ADM_fopen(target,"wb");
    if(!fout)
    {
        fclose(fin);
        ADM_error("Cannot open %s for writting\n",target);
        return false;
    }
    uint8_t buffer[1024];
    while(!feof(fin))
    {
        int r=fread(buffer,1,1024,fin);
        fwrite(buffer,1,r,fout);
        if(r!=1024) break;
    }

    fclose(fin);
    fclose(fout);
    return true;
}


/**
 * 	\fn ADM_getRelativePath
 */
char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3)
{
	char *result;
	int length = strlen(base0) + 2;

	if (base1)
		length += strlen(base1) + 1;

	if (base2)
		length += strlen(base2) + 1;

	if (base3)
		length += strlen(base3) + 1;

	result = (char *)new char[length];
	strcpy(result, base0);
	AddSeparator(result);

	if (base1)
	{
		if (strlen(base1))
		{
			strcat(result, base1);
			strcat(result, ADM_SEPARATOR);
		}

		if (base2)
		{
			if (strlen(base2))
			{
				strcat(result, base2);
				strcat(result, ADM_SEPARATOR);
			}

			if (base3 && strlen(base3))
			{
				strcat(result, base3);				
				strcat(result, ADM_SEPARATOR);
			}
		}
	}

	return result;
}


/**
 * \fn ADM_getCustomDir
 * \brief      Get the  directory where jobs are stored
*/
static char *ADM_customdir = NULL;
const char *ADM_getCustomDir(void)
{
	if (ADM_customdir)
		return ADM_customdir;

	ADM_customdir = ADM_getHomeRelativePath("custom");

	if (!ADM_mkdir(ADM_customdir))
	{
		printf("can't create custom directory (%s).\n", ADM_customdir);
		return NULL;
	}

	return ADM_customdir;
}


/**
 *      \fn ADM_getJobDir
      \brief Get the  directory where jobs are stored
*/
static char *ADM_jobdir = NULL;
const char *ADM_getJobDir(void)
{
	if (ADM_jobdir)
		return ADM_jobdir;

	ADM_jobdir = ADM_getHomeRelativePath("jobs");


	if (!ADM_mkdir(ADM_jobdir))
	{
		printf("can't create custom directory (%s).\n", ADM_jobdir);
		return NULL;
	}

	return ADM_jobdir;
}

/**
 * \fn  ADM_getUserPluginSettingsDir
 * \brief returns the user plugin setting 
*/
static char *ADM_userPluginSettings=NULL;
const char *ADM_getUserPluginSettingsDir(void)
{
    if(ADM_userPluginSettings) return ADM_userPluginSettings;
    ADM_userPluginSettings=ADM_getHomeRelativePath("pluginSettings");
    return ADM_userPluginSettings;
}


/**
    \fn isPortableMode
    \brief returns true if we are in portable mode
*/
bool isPortableMode(int argc, char *argv[])
{
	bool portableMode = false;
    std::string mySelf=argv[0];
    // if the name ends by "_portable.exe" => portable
    int match=mySelf.find("portable");
    if(match!=-1)
    {
        ADM_info("Portable mode\n");
        return true;
    }

    for (int i = 0; i < argc; i++)
    {
            if (!strcmp(argv[i], "--portable"))
            {
                    portableMode = true;
                    break;
            }
    }

    return portableMode;
}
/**
 */
std::string pluginDir;
std::string ADM_getPluginDir(const char *subfolder)
{
    std::string out;
    if(!pluginDir.size())      
    {
            #ifdef __APPLE__
                const char *startDir="../lib";
            #else
                const char *startDir=ADM_RELATIVE_LIB_DIR;
            #endif
        
            char *p=ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR,"");
            pluginDir = std::string(p);
            delete [] p;p=NULL;
    }
    return pluginDir+std::string(ADM_SEPARATOR)+std::string(subfolder);
}            

// EOF

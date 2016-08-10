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
/**
    \fn ADM_fopen
    \brief utf8 aware fopen, so that we can use utf8 string even on win32
*/
FILE *ADM_fopen(const char *file, const char *mode)
{
#ifdef _WIN32
	// Override fopen to handle Unicode filenames and to ensure exclusive access when initially writing to a file.
	int fileNameLength = utf8StringToWideChar(file, -1, NULL);
	wchar_t *wcFile = new wchar_t[fileNameLength];
	int creation = 0, access = 0;
	HANDLE hFile;

	utf8StringToWideChar(file, -1, wcFile);

	while (true)
	{
		if (strchr(mode, 'w'))
		{
			creation = CREATE_ALWAYS;
			access = GENERIC_WRITE;

			if (strchr(mode, '+'))
				access |= GENERIC_READ;
		}
		else if (strchr(mode, 'r'))
		{
			creation = OPEN_EXISTING;
			access = GENERIC_READ;

			if (strchr(mode, '+'))
				access = GENERIC_WRITE;
		}
		else if (strchr(mode, 'a'))
		{
			creation = OPEN_ALWAYS;
			access = GENERIC_WRITE;

			if (strchr(mode, '+'))
				access |= GENERIC_READ;
		}

		if (creation & GENERIC_WRITE)
		{
			hFile = CreateFileW(wcFile, access, 0, NULL, creation, 0, NULL);

			if (hFile == INVALID_HANDLE_VALUE)
				break;
			else
				CloseHandle(hFile);
		}

		hFile = CreateFileW(wcFile, access, FILE_SHARE_READ, NULL, creation, 0, NULL);
		break;
	}

	delete [] wcFile;

	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;
	else
		return _fdopen(_open_osfhandle((intptr_t)hFile, 0), mode);
#else
	return fopen(file, mode);
#endif
}

#if _WIN32
extern "C"
{
	// libavformat uses open (in the file_open function) so we need to override that too.
	// Following the same rules as ADM_fopen.
	int ADM_open(const char *path, int oflag, ...)
	{
		int fileNameLength = utf8StringToWideChar(path, -1, NULL);
		wchar_t *wcFile = new wchar_t[fileNameLength];
		int creation = 0, access = 0;
		HANDLE hFile;

		utf8StringToWideChar(path, -1, wcFile);

		delete [] wcFile;

		if (oflag & O_WRONLY || oflag & O_RDWR)
		{
			access = GENERIC_WRITE;

			if (oflag & O_RDWR)
				access |= GENERIC_READ;

			if (oflag & O_CREAT)
			{
				if (oflag & O_EXCL)
					creation = CREATE_NEW;
				else if (oflag & O_TRUNC)
					creation = CREATE_ALWAYS;
				else
					creation = OPEN_ALWAYS;
			}
			else if (oflag & O_TRUNC)
				creation = TRUNCATE_EXISTING;
		}
		else if (oflag & O_RDONLY)
			creation = OPEN_EXISTING;

		if (creation & GENERIC_WRITE)
		{
			hFile = CreateFileW(wcFile, access, 0, NULL, creation, 0, NULL);

			if (hFile == INVALID_HANDLE_VALUE)
				return -1;
			else
				CloseHandle(hFile);
		}

		hFile = CreateFileW(wcFile, access, FILE_SHARE_READ, NULL, creation, 0, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return -1;
		else
			return _open_osfhandle((intptr_t)hFile, oflag);
	}
}
#endif

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

/*----------------------------------------
      Create a directory
      If it already exists, do nothing
------------------------------------------*/
uint8_t ADM_mkdir(const char *dirname)
{
	DIR *dir = NULL;
	uint8_t retVal = 0;

#ifdef _WIN32
	int dirNameLength = utf8StringToWideChar(dirname, -1, NULL);
	wchar_t *dirname2 = new wchar_t[dirNameLength];

	utf8StringToWideChar(dirname, -1, dirname2);
#else
	const char* dirname2 = dirname;
#endif

	while (true)
	{
		// Check it already exists ?
		dir = opendir(dirname2);

		if (dir)
		{
			printf("Directory %s exists.Good.\n", dirname);
			closedir(dir);

			retVal = 1;
			break;
		}
#ifdef _WIN32
		if (_wmkdir(dirname2))
		{
			printf("Oops: mkdir failed on %s\n", dirname);
			break;
		}
#else
		printf("Creating dir :%s\n", dirname2);
		mkdir(dirname2,0755);
#endif

		if ((dir = opendir(dirname2)) == NULL)
			break;

		closedir(dir);

		retVal = 1;

		break;
	}

#ifdef _WIN32
	delete [] dirname2;
#endif

	return retVal;
}
/**
 *  \fn buildDirectoryContent
 * 	\brief Returns the content of a dir with the extension ext. The receiving array must be allocated by caller
 * (just the array, not the names themselves)
 */
uint8_t buildDirectoryContent(uint32_t *outnb, const char *base, char *jobName[], int maxElems, const char *ext)
{
	DIR *dir;
	struct dirent *direntry;
	int dirmax = 0, len;
	int extlen = strlen(ext);

	ADM_assert(extlen);

#ifdef _WIN32
	int dirNameLength = utf8StringToWideChar(base, -1, NULL);
	wchar_t *base2 = new wchar_t[dirNameLength];

	utf8StringToWideChar(base, -1, base2);
#else
	const char *base2 = base;
#endif

	dir = opendir(base2);

#ifdef _WIN32
	delete [] base2;
#endif

	if (!dir)
	{
		return 0;
	}

#ifdef _WIN32
	char *d_name = NULL;
#endif

	while ((direntry = readdir(dir)))
	{
#ifdef _WIN32
		delete [] d_name;

		int dirLength = wideCharStringToUtf8(direntry->d_name, -1, NULL);
		d_name = new char[dirLength];

		wideCharStringToUtf8(direntry->d_name, -1, d_name);
#else
		const char *d_name = direntry->d_name;
#endif

		len = strlen(d_name);

		if (len < (extlen + 1))
			continue;

		int xbase = len - extlen;

		if (memcmp(d_name + xbase, ext, extlen))
		{
			printf("ignored: %s\n", d_name);
			continue;
		}

		jobName[dirmax] = (char *)ADM_alloc(strlen(base) + strlen(d_name) + 2);
		strcpy(jobName[dirmax], base);
		AddSeparator(jobName[dirmax]);
		strcat(jobName[dirmax], d_name);
		dirmax++;

		if (dirmax >= maxElems)
		{
			printf("[jobs]: Max # of jobs exceeded\n");
			break;
		}
	}

#ifdef _WIN32
	delete [] d_name;
#endif

	closedir(dir);
	*outnb = dirmax;

	return 1;
}
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
//------------------------------------------------------------------

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
void ADM_PathStripName(char *str)
{
	int len = strlen(str);

	if (len <= 1)
		return;

	len--;

#ifndef _WIN32
	while (*(str + len) != '/' && len)
#else
	while (*(str + len) != '\\' && len)
#endif
	{
		*(str + len) = 0;
		len--;
	}
}

/**
    \fn ADM_GetFileName
    \brief Get the filename without path. /foo/bar.avi -> bar.avi INPLACE, NO COPY

*/
const char *ADM_GetFileName(const char *str)
{
	const char *filename;

#ifndef _WIN32
	filename = strrchr(str, '/');
#else
	const char *filename2;

	filename = strrchr(str, '\\');
	filename2 = strrchr(str, '/');

	if (filename2 && filename)
		if (filename2 > filename)
			filename = filename2;
#endif

	if (filename)
		return filename + 1;
	else
		return str;
}

/**
    \fn ADM_PathSplit
    \brief Split path into absolute path+name and extention i.e. /foo/bar/zee.avi -> /foo/bar/zee,avi.             Copy are returned

*/
void ADM_PathSplit(const char *str, char **root, char **ext)
{
	char *full;
	uint32_t l;

	full = ADM_PathCanonize(str);
	// Search the last
	l = strlen(full);
	l--;
	ADM_assert(l > 0);

	while (*(full + l) != '.' && l)
		l--;

	if (!l || l == (strlen(full) - 1))
	{
		if (l == (strlen(full) - 1))
			*(full + l) = 0;  // remove trailing

		*ext = new char[2];
		*root = full;
		strcpy(*ext, "");

		return;
	}
	// else we do get an extension
	// starting at l+1
	uint32_t suff;

	suff = strlen(full) - l - 1;
	*ext = new char[suff + 1];
	strcpy(*ext, full + l + 1);
	*(full + l) = 0;
	*root = full;
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
    \fn ADM_copyFile
*/
uint8_t ADM_renameFile(const char *source, const char *target)
{
#ifdef __MINGW32__
	int sourceFileNameLength = utf8StringToWideChar(source, -1, NULL);
    int targetFileNameLength = utf8StringToWideChar(target, -1, NULL);
    wchar_t wcFileSource[sourceFileNameLength];
    wchar_t wcFileTarget[targetFileNameLength];
    
    utf8StringToWideChar(source, -1, wcFileSource);
    utf8StringToWideChar(target, -1, wcFileTarget);
   
    if(!_wrename(wcFileSource,wcFileTarget)) return true;
    ADM_error("Failed to rename %s to %s\n",source,target);
    return false;
#else
    if(!rename(source,target)) return true;
    return false;
#endif
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

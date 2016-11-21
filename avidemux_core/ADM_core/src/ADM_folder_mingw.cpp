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
#include <io.h>
#include <direct.h>
#include <shlobj.h>
#include "ADM_win32.h"

#include "ADM_default.h"

extern void simplify_path(char **buf);
char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);
static char ADM_basedir[1024] = {0};
static char *ADM_autodir = NULL;
static char *ADM_systemPluginSettings=NULL;


#undef fread
#undef fwrite
#undef fopen
#undef fclose

static void AddSeparator(char *path)
{
	if (path && (strlen(path) < strlen(ADM_SEPARATOR) || strncmp(path + strlen(path) - strlen(ADM_SEPARATOR), ADM_SEPARATOR, strlen(ADM_SEPARATOR)) != 0))
		strcat(path, ADM_SEPARATOR);
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

//--- 8< 8< 8<

#	include <io.h>
#	include <direct.h>
#	include <shlobj.h>
#	include <fcntl.h>
#	include "ADM_win32.h"


/**
    \fn ADM_fopen
    \brief utf8 aware fopen, so that we can use utf8 string even on win32
*/
FILE *ADM_fopen(const char *file, const char *mode)
{

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
}


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



#define DIR _WDIR
#define dirent _wdirent
#define opendir _wopendir
#define readdir _wreaddir
#define closedir _wclosedir

/*----------------------------------------
      Create a directory
      If it already exists, do nothing
------------------------------------------*/
uint8_t ADM_mkdir(const char *dirname)
{
	DIR *dir = NULL;
	uint8_t retVal = 0;


	int dirNameLength = utf8StringToWideChar(dirname, -1, NULL);
	wchar_t *dirname2 = new wchar_t[dirNameLength];

	utf8StringToWideChar(dirname, -1, dirname2);

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

		if (_wmkdir(dirname2))
		{
			printf("Oops: mkdir failed on %s\n", dirname);
			break;
		}

		if ((dir = opendir(dirname2)) == NULL)
			break;

		closedir(dir);

		retVal = 1;

		break;
	}


	delete [] dirname2;

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


	int dirNameLength = utf8StringToWideChar(base, -1, NULL);
	wchar_t *base2 = new wchar_t[dirNameLength];

	utf8StringToWideChar(base, -1, base2);

	dir = opendir(base2);


	delete [] base2;

	if (!dir)
	{
		return 0;
	}


	char *d_name = NULL;

	while ((direntry = readdir(dir)))
	{
		delete [] d_name;

		int dirLength = wideCharStringToUtf8(direntry->d_name, -1, NULL);
		d_name = new char[dirLength];

		wideCharStringToUtf8(direntry->d_name, -1, d_name);

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


	delete [] d_name;

	closedir(dir);
	*outnb = dirmax;

	return 1;
}

/**
    \fn ADM_copyFile
*/
uint8_t ADM_renameFile(const char *source, const char *target)
{

    int sourceFileNameLength = utf8StringToWideChar(source, -1, NULL);
    int targetFileNameLength = utf8StringToWideChar(target, -1, NULL);
    wchar_t wcFileSource[sourceFileNameLength];
    wchar_t wcFileTarget[targetFileNameLength];

    utf8StringToWideChar(source, -1, wcFileSource);
    utf8StringToWideChar(target, -1, wcFileTarget);

    if(!_wrename(wcFileSource,wcFileTarget)) return true;
    ADM_error("Failed to rename %s to %s\n",source,target);
    return false;
}


// EOF

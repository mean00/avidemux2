/***************************************************************************
 * \name ADM_folder_unix.cpp
 * \author mean (c) 2006/2016
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
#include "ADM_default.h"

#undef fopen
#undef mkdir

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
    \fn ADM_fopen
    \brief utf8 aware fopen, so that we can use utf8 string even on win32
*/
FILE *ADM_fopen(const char *file, const char *mode)
{
	return fopen(file, mode);
}


/*----------------------------------------
      Create a directory
      If it already exists, do nothing
------------------------------------------*/
uint8_t ADM_mkdir(const char *dirname)
{
	DIR *dir = NULL;
	uint8_t retVal = 0;

	const char* dirname2 = dirname;

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
		printf("Creating dir :%s\n", dirname2);
		mkdir(dirname2,0755);

		if ((dir = opendir(dirname2)) == NULL)
			break;

		closedir(dir);

		retVal = 1;

		break;
	}

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

	const char *base2 = base;

	dir = opendir(base2);


	if (!dir)
	{
		return 0;
	}


	while ((direntry = readdir(dir)))
	{
		const char *d_name = direntry->d_name;

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


	closedir(dir);
	*outnb = dirmax;

	return 1;
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
		strcat(out, "/");
		printf("\n Canonizing null string ??? (%s)\n", out);
	}
	else if (tmpname[0] == '/'		)
	{
		out = new char[strlen(tmpname) + 1];
		strcpy(out, tmpname);

		return out;
	}
	else
	{
		out = new char[strlen(path) + strlen(tmpname) + 6];
		strcpy(out, path);
		strcat(out, "/");
		strcat(out, tmpname);
	}

	simplify_path(&out);

	return out;
}

/**
        \fn ADM_extractPath
	\brief Returns path only /foo/bar.avi -> /foo

*/
std::string ADM_extractPath(const std::string &str)
{
    std::string p;
         p=str;
         size_t idx=p.find_last_of ("/");
         if(idx!=std::string::npos)
            p.resize(idx);
         return p;
}
/**
*/
const std::string ADM_getFileName(const std::string &str)
{
    size_t idx=str.find_last_of ("/");
    if(idx==std::string::npos) return str;
    return str.substr(idx+1);
}

/**
    \fn ADM_copyFile
*/
uint8_t ADM_renameFile(const char *source, const char *target)
{
    if(!rename(source,target)) return true;
    return false;
}
       

// EOF

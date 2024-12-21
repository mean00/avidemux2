 /***************************************************************************

     copyright            : (C) 2016 by mean
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
 #include <algorithm>
 #include <io.h>
 #include <direct.h>
 #include <shlobj.h>
 #include <fcntl.h>
 #include "ADM_win32.h"
 #include "ADM_default.h"

extern char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3);

 #undef fread
 #undef fwrite
 #undef fopen
 #undef fclose

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


 /*----------------------------------------
       Create a directory
       If it already exists, do nothing
 ------------------------------------------*/
 uint8_t ADM_mkdir(const char *dirname)
 {

    int dirNameLength = utf8StringToWideChar(dirname, -1, NULL);
    wchar_t *dirname2 = new wchar_t[dirNameLength];
    utf8StringToWideChar(dirname, -1, dirname2);
    int er = _wmkdir(dirname2);
    delete[] dirname2;
    uint8_t retVal = 0;


    if (er != -1)
        retVal = true;
    else
    {
        if (errno == EEXIST)
            retVal = true;
        else
        {
            ADM_warning("Failed to create folder %s with error %d\n", dirname, errno);
            retVal = false;
        }
    }

 	
 	return retVal;
 }
 


  // Convert string from Wide Char to ANSI code page
 static int wideCharStringToAnsi(const wchar_t *wideCharString, int wideCharStringLength, char *ansiString, const char *filler)
 {
     int flags = WC_COMPOSITECHECK;

     if (filler)
         flags |= WC_NO_BEST_FIT_CHARS | WC_DEFAULTCHAR;

     int ansiStringLen = WideCharToMultiByte(CP_ACP, flags, wideCharString, wideCharStringLength, NULL, 0, filler, NULL);

     if (ansiString)
         WideCharToMultiByte(CP_ACP, flags, wideCharString, wideCharStringLength, ansiString, ansiStringLen, filler, NULL);

     return ansiStringLen;
 }
/**
 *  \fn buildDirectoryContent
 *  \brief Returns the content of a dir with the extension ext. The receiving vector must be allocated by caller
 */
uint8_t buildDirectoryContent(const char *base, std::vector<std::string> *list, const char *ext)
 {
    std::string joker = base;
    joker += "/*.";
    joker += ext;

    list->clear();

 	int dirNameLength = utf8StringToWideChar(joker.c_str(), -1, NULL);
 	wchar_t *base2 = new wchar_t[dirNameLength];
    utf8StringToWideChar(joker.c_str(), -1, base2);
//--
    HANDLE hFind;
    WIN32_FIND_DATAW FindFileData;

    hFind = FindFirstFileW(base2,&FindFileData);
    if(hFind == INVALID_HANDLE_VALUE)
    {
        ADM_warning("Cannot list content of %s\n", base);
        delete[] base2;
        return true;
    }

    do 
    {
        WCHAR *wname = FindFileData.cFileName;
        //int wideCharStringToAnsi(const wchar_t *wideCharString, int wideCharStringLength, char *ansiString, const char *filler)
        int nameLength = wideCharStringToUtf8(wname, -1, NULL);
        char *shortName = new char[nameLength];
        nameLength = wideCharStringToUtf8(wname, -1, shortName);
        std::string item = base;
        item += ADM_SEPARATOR;
        item += shortName;
        list->push_back(item);
        delete[] shortName;

    } while (FindNextFileW(hFind, &FindFileData));
    FindClose(hFind);

    delete[] base2;
    std::sort(list->begin(),list->end());
    return true;
 }

 /**
     \fn ADM_copyFile
 */
 uint8_t ADM_renameFile(const char *source, const char *target)
 {

     int sourceFileNameLength = utf8StringToWideChar(source, -1, NULL);
     int targetFileNameLength = utf8StringToWideChar(target, -1, NULL);
     wchar_t *wcFileSource=(wchar_t*)_alloca(sourceFileNameLength*sizeof(wchar_t));
     wchar_t *wcFileTarget= (wchar_t*)_alloca(targetFileNameLength * sizeof(wchar_t));

     utf8StringToWideChar(source, -1, wcFileSource);
     utf8StringToWideChar(target, -1, wcFileTarget);

     if(!_wrename(wcFileSource,wcFileTarget)) return true;
     ADM_error("Failed to rename %s to %s\n",source,target);
     return false;
 }

// EOF

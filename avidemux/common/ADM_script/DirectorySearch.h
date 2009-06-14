// DirectorySearch.h: interface for the CDirectorySearch class.
//
//////////////////////////////////////////////////////////////////////
/*
Copyright 2001-2005 Anish Mistry. All rights reserved.

Note:  This file is available under a BSD license.  Contact the author
at amistry@am-productions.biz
*/

#ifndef _DIRECTORYSEARCH_H
#define _DIRECTORYSEARCH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef __MINGW32__
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#endif
#include <cstring>
#include "StdFile.h"

// create a non file bit for unix
#define _A_NONFILE	0x03

#if defined( __unix__) || defined(__APPLE__)

// wrap the file mode bits
#define _A_NORMAL	0x00
#define _A_HIDDEN	0x02
#define _A_ARCH		0x20
#define _A_SUBDIR	0x10

// we need to emulate the windows style directory searching
struct _finddata_t {
	unsigned    attrib;
	time_t      time_create;    /* -1 for FAT file systems */
	time_t      time_access;    /* -1 for FAT file systems */
	time_t      time_write;
	int64_t     size;
	char        name[260];
	_finddata_t() : attrib(0), time_create(0), time_access(0), time_write(0), size(0) {name[0] = 0;}
};

#endif


class CDirectorySearch
{
public:
	const char * GetExtension();
	bool IsExtension(const char *pExtension);
	inline bool NextFile()
	{// begin NextFile
		if(_findnext(m_hSearch,&m_fdData) == -1)
			return false;
		return true;
	}// end NextFile
	bool Init(std::string sDirectory);
	bool IsDoubleDot();
	bool IsSingleDot();
	inline unsigned long int GetFileSize()
	// returns the file size in bytes
	{// begin GetFileSize
		return m_fdData.size;
	}// end GetFileSize
	inline bool IsHidden()
	{// begin IsHidden
		return (bool)(m_fdData.attrib & _A_HIDDEN);
	}// end IsHidden
	inline bool IsArchive()
	{// begin IsArchive
		return (bool)(m_fdData.attrib & _A_ARCH);
	}// end IsArchive
	inline bool IsDirectory()
	{// begin IsDirectory
		return (bool)(m_fdData.attrib & _A_SUBDIR);
	}// end IsDirectory
	inline bool IsNotFile()
	{// begin IsNotFile
		return (bool)(m_fdData.attrib & _A_NONFILE);
	}// end IsNotFile
	std::string GetFileDirectory();
	inline char * GetFileName(char *pBuffer)
	{// begin GetFileName
		return strcpy(pBuffer,m_fdData.name);
	}// end GetFileName
	inline const char * GetFileName()
	{// begin GetFileName
		return (const char *)m_fdData.name;
	}// end GetFileName
	std::string GetFilePath();
	bool Close();
	CDirectorySearch();
	virtual ~CDirectorySearch();

protected:
	std::string GetFileDirectory(std::string sFilePath) const;
	long m_hSearch;
	_finddata_t m_fdData;
	std::string m_sDirectory;
private:
#if defined(__unix__) || defined(__APPLE__)
	// prototypes
	int _findfirst(const char *path,_finddata_t *pfdData);
	int _findnext(unsigned long int hDir,_finddata_t *pfdData);
	int _findclose(unsigned long int hDir);
#endif
};

#endif // !defined(_DIRECTORYSEARCH_H)

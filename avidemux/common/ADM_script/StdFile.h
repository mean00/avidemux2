// StdFile.h: interface for the CStdFile class.
//
//////////////////////////////////////////////////////////////////////
/*
Copyright 2001-2005 Anish Mistry. All rights reserved.

Note:  This file is available under a BSD license.  Contact the author
at amistry@am-productions.biz
*/

#if !defined(AFX_STDFILE_H__44214218_8F1A_4792_8D4D_8147F50E168D__INCLUDED_)
#define AFX_STDFILE_H__44214218_8F1A_4792_8D4D_8147F50E168D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace std;

#ifdef __WIN32
	const char DIRECTORY_DELIMITOR = '\\';
#else
	const char DIRECTORY_DELIMITOR = '/';
	#define _MAX_PATH 1024
#endif

class CStdFile
{
public:
	char Peek();
	int WriteString(const char *pBuffer);
	char * ReadString(char *pBuffer, int nMaxLen,char cDelim = ' ');
	unsigned long int GetPos();
	unsigned long int Seek(unsigned long int pos,ios::seekdir start);
	unsigned long int GetLength();
	unsigned long int Write(const char *buffer, unsigned long int bytesToWrite);
	unsigned long int Read(char *buffer, unsigned long int bytesToRead);
	const char * GetFilePath();
	const char * GetFileName();
	CStdFile(void);
	virtual ~CStdFile(void);
//	static bool Copy(const char *source,const char *dest, bool bOverwrite);
	static int Rename(const char *source,const char *dest);
	static int Delete(const char *fileName);
	virtual bool Open(const char *filename, ios::openmode accessFlags);
	unsigned long int WriteLine(const char *);
	unsigned long int ReadLine(char *bufferOut);
	virtual bool Close();

protected:
	void GetName(const char *fileString,char *fileBufferOut);
	char *m_pPath;
	char *m_pName;
	fstream m_fsFile;
};

#endif // !defined(AFX_STDFILE_H__44214218_8F1A_4792_8D4D_8147F50E168D__INCLUDED_)

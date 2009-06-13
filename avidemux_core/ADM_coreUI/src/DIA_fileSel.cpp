/***************************************************************************
                          DIA_fileSel.cpp
  
  (C) Mean 2008 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/stat.h>

#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"

static DIA_FILESEL_DESC_T *fileSelDescriptor=NULL;
/**
 * 	\fn DIA_fileSelInit
 *  \brief Hook the fileDialog
 */
uint8_t DIA_fileSelInit(DIA_FILESEL_DESC_T *d)
{
	fileSelDescriptor=d;
	fileSelDescriptor->fileInit();
	return 1;
}

// A bunch of boomerang functions
/**
 * \fn GUI_FileSelRead
 * \brief Select a file for reading, when ok the CB callback is called with the name as argument
 */	
void GUI_FileSelRead(const char *label,SELFILE_CB *cb) 
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileReadCb(label,cb);	
}
/**
 * \fn GUI_FileSelWrite
 * \brief Select a file for Writing, when ok the CB callback is called with the name as argument
 */	
void GUI_FileSelWrite(const char *label,SELFILE_CB *cb) 
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileWriteCb(label,cb);	
}
/**
 * \fn GUI_FileSelRead
 * \brief Select a file for reading, name is allocated with a copy of the name, null if fail.
 */	
void GUI_FileSelRead(const char *label,char * * name) 
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileReadName(label,name);	
}
/**
 * \fn GUI_FileSelWrite
 * \brief Select a file for Writing, name is allocated with a copy of the name, null if fail.
 */	

void GUI_FileSelWrite(const char *label,char * * name) 
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileWriteName(label,name);	
}
/**
 * \fn FileSel_SelectWrite
 * \brief Select a file for Writing, name is allocated with a copy of the name, null if fail.
 * @param title : [in] Title of the dialog box
 * @param target: [in/out] Buffer that will hold the name, must be at least max bytes big
 * @param max : [in] Max number of bytes the buffer will hold
 * @param source : [in] Initial value for the file, can be null
 */	
uint8_t FileSel_SelectWrite(const char *title,char *target,uint32_t max, const char *source)
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileSelectWrite(title,target,max, source);
}
/**
 * \fn FileSel_SelectRead
 * \brief Select a file for Reading, name is allocated with a copy of the name, null if fail.
 * @param title : [in] Title of the dialog box
 * @param target: [in/out] Buffer that will hold the name, must be at least max bytes big
 * @param max : [in] Max number of bytes the buffer will hold
 * @param source : [in] Initial value for the file, can be null
 */	
uint8_t FileSel_SelectRead(const char *title,char *target,uint32_t max, const char *source)
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileSelectRead(title,target,max, source);
}
/**
 * \fn FileSel_SelectDir
 * \brief Select a directory, name is allocated with a copy of the name, null if fail.
 * @param title : [in] Title of the dialog box
 * @param target: [in/out] Buffer that will hold the name, must be at least max bytes big
 * @param max : [in] Max number of bytes the buffer will hold
 * @param source : [in] Initial value for the file, can be null
 */	
uint8_t FileSel_SelectDir(const char *title,char *target,uint32_t max, const char *source)
{
	ADM_assert(fileSelDescriptor);
	return fileSelDescriptor->fileSelectDirectory(title,target,max, source);
}

#warning QT_TR_NOOP
#ifndef QT_TR_NOOP
#define QT_TR_NOOP(x) x
#endif
void FileSel_ReadWrite(SELFILE_CB *cb, int rw, const char *name, const char *actual_workbench_file)
{
	if(name)
	{
		if(cb)
		{
			FILE *fd;
			fd=ADM_fopen(name,"rb");
			if(rw==0) // read
			{
				// try to open it..
				if(!fd)
				{
					GUI_Error_HIG(QT_TR_NOOP("File error"), QT_TR_NOOP("Cannot open \"%s\"."), name);
					return;
				}
			}
			else // write
			{
				if(fd){
					struct stat buf;
					int fdino;
					ADM_fclose(fd);

					char msg[300];

					snprintf(msg, 300, QT_TR_NOOP("%s already exists.\n\nDo you want to replace it?"), ADM_GetFileName(name));

					if(!GUI_Question(msg))
						return;
					/*
					** JSC Fri Feb 10 00:07:30 CET 2006
					** compare existing output file inode against each current open files inode
					** i'm ignoring st_dev, so we may get false positives
					** i'm testing until fd=1024, should be MAXFD computed by configure
					** keep in mind:
					** you can overwrite .idx files, they are loaded into memory and closed soon
					** you cannot overwrite segment data files, all files are kept open and
					** are detected here
					*/
#ifndef __WIN32
					if( stat(name,&buf) == -1 ){
						fprintf(stderr,"stat(%s) failed\n",name);
						return;
					}
#endif
					fdino = buf.st_ino;
					for(int i=0;i<1024;i++){
						if( fstat(i,&buf) != -1 ){
							if( buf.st_ino == fdino ){
								char str[512];
								snprintf(str,512,"File \"%s\" exists and is opened by Avidemux",name);
								GUI_Error_HIG(str,
									QT_TR_NOOP("It is possible that you are trying to overwrite an input file!"));
								return;
							}
						}
					}
					/*
					** compare output file against actual EMCAscript file
					** need to stat() to avoid symlink (/home/x.js) vs. real file (/export/home/x.js) case
					*/
					if( actual_workbench_file ){
						if( stat(actual_workbench_file,&buf) != -1 ){
							if( buf.st_ino == fdino ){
								char str[512];
								snprintf(str,512,"File \"%s\" exists and is the actual ECMAscript file",name);
								GUI_Error_HIG(str,QT_TR_NOOP("It is possible that you are trying to overwrite an input file!"));
								return;
							}
						}
					}
				}

				// check we have right access to it
				fd=ADM_fopen(name,"wb");
				if(!fd)
				{
					GUI_Error_HIG(QT_TR_NOOP("Cannot write the file"),QT_TR_NOOP( "No write access to \"%s\"."), name);
					return;
				}
			}
			ADM_fclose(fd);
			cb(name);
		} // no callback -> return value
	}
}

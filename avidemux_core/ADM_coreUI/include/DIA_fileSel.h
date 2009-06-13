
/***************************************************************************
    copyright            : (C) 2001/2005 by mean
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

#ifndef DIA_FILESEL_H
#define DIA_FILESEL_H
typedef void SELFILE_CB(const char *);

// Associated functions type
typedef void    DIA_FILE_SEL_CB(const char *label,SELFILE_CB *cb);
typedef void    DIA_FILE_SEL_NAME(const char *label, char * * name);
typedef uint8_t DIA_FILE_SELECT(const char *title,char *target,uint32_t max, const char *source);
typedef void	DIA_FILE_INIT(void);

// Old FileSel interface
void GUI_FileSelRead(const char *label,SELFILE_CB *cb) ;
void GUI_FileSelWrite(const char *label,SELFILE_CB *cb) ;
void GUI_FileSelRead(const char *label, char * * name);
void GUI_FileSelWrite(const char *label, char * * name);

// New FileSel interface, the receiving buffer is allocated by the caller
uint8_t FileSel_SelectWrite(const char *title,char *target,uint32_t max, const char *source);
uint8_t FileSel_SelectRead(const char *title,char *target,uint32_t max, const char *source);
uint8_t FileSel_SelectDir(const char *title,char *target,uint32_t max, const char *source);


#endif
//EOF

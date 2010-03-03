/** *************************************************************************
    \file                      DIA_toolkit.h
    \brief Handle basic popup window
                             
    begin                : Fri Dec 14 2001--8
    copyright            : (C) 2001 by mean
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

#ifndef ADM_COREUI_TOOLKIT__
#define ADM_COREUI_TOOLKIT__

typedef enum 
{
        ADM_LOG_NONE=0,
        ADM_LOG_IMPORTANT=1,
        ADM_LOG_INFO=2,
        ADM_LOG_DEBUG=3
  
} ADM_LOG_LEVEL;

// Display a warning/info/debug message. The primary field is the "title" of the window, secondary format is printf like
void            GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format, ...);
// Display an error message. The primary field is the "title" of the window, secondary format is printf like
void            GUI_Error_HIG(const char *primary, const char *secondary_format, ...);
// ask for confirmation. The button_confirm will be the title of the button (accept, do it,...) 
int             GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format, ...);
// Ask for yes/no. Yes will return 1, No will return 0
int             GUI_YesNo(const char *primary, const char *secondary_format, ...);
// About the same as GUI_YesNo, the button will be ok/cancel
int             GUI_Question(const char *alertstring);
// Give some time to the UI to handle its events
void            GUI_Sleep(uint32_t ms);
// Ask to choose between choice1 and choice2
int             GUI_Alternate(const char *title,const char *choice1,const char *choice2);


// Set ui in verbose mode (default). Show all popup & questions
void            GUI_Verbose(void);
// Set the ui in silent mode. All popups & questions will be answered with their default value
void            GUI_Quiet(void);
// Is the UI in quiet mode ?
uint8_t			GUI_isQuiet(void);
//
void            UI_purge(void);
//

#endif

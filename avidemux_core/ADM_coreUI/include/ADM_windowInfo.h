/**


*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef ADM_WINDOW_INFO_H
#define ADM_WINDOW_INFO_H
typedef struct
{
    void *display;  // X11 display System
    void *widget;   // QT widget
    int  systemWindowId;
    int x;
    int y;
    int width;
    int height;
} GUI_WindowInfo;
#endif

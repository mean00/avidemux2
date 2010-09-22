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
    void *display;
    void *widget;
    int  window;
    int x;
    int y;
    int width;
    int height;
} GUI_WindowInfo;
#endif

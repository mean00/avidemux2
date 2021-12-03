/***************************************************************************
                          gtkgui.h  -  description
                             -------------------
    begin                : Mon Dec 10 2001
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

void 			GUI_setCurrentFrameAndTime(uint64_t offset=0);
void 			GUI_setAllFrameAndTime(void );



bool GUI_NextFrame(void);
bool GUI_PrevFrame(void);
bool GUI_NextKeyFrame(void);
bool GUI_PreviousKeyFrame(void);

bool GUI_GoToTime(uint64_t time);
uint8_t GUI_close(void);

void GUI_PrevBlackFrame(void);
void GUI_NextBlackFrame( ) ;
void GUI_NextPrevBlackFrame( int ) ;
uint8_t A_ListAllBlackFrames( char *name);
void GUI_PlayAvi(bool quit = false);
void GUI_DisplayAudio(void);
uint32_t GUI_GetScale( void );
void     GUI_SetScale( uint32_t scale );
void GUI_detransient(void );
void GUI_retransient(void );

//

//EOF


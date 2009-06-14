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

int 			A_openAvi		(const char *name);
void 			A_saveAudio	(char *name);
void 			A_saveAudioDecoded	(char *name);
void 			A_saveAVI		(char *name);
void 			A_playAvi		(void);
void 			GUI_setCurrentFrameAndTime(void );
void 			GUI_setAllFrameAndTime(void );



void GUI_NextFrame( uint32_t frameCount = 1 );
void GUI_PrevFrame( uint32_t frameCount = 1 );
void GUI_NextKeyFrame( void ) ;
void GUI_NextPrevBlackFrame( int ) ;
void GUI_PreviousKeyFrame( void );
uint8_t A_ListAllBlackFrames( char *name);
void GUI_PlayAvi( void );
uint32_t GUI_GetScale( void );
void GUI_detransient(void );
void GUI_retransient(void );

//

//EOF


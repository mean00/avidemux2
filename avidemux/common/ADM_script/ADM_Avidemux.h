/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_AVIDEMUX_H
#define _ADM_AVIDEMUX_H
#include "ADM_JSGlobal.h"
#include "ADM_JSAvidemuxAudio.h"
#include "ADM_JSAvidemuxVideo.h"

class ADM_Avidemux
{
public:
	ADM_Avidemux(void);
	virtual ~ADM_Avidemux(void);

	// member variables
	JSObject *m_pAudio;
	JSObject *m_pVideo;
	JSString *m_pContainer;
	int m_nCurrentFrame;
	double m_dFPS;

};

#endif

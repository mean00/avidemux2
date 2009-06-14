/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define JSDECLARE
#include "ADM_Avidemux.h"

ADM_Avidemux::ADM_Avidemux(void) :  m_pAudio(NULL), m_pVideo(NULL), m_pContainer(NULL), m_nCurrentFrame(0), m_dFPS(0)
{// begin ADM_Avidemux

	// initialize audio property
	JSObject *pTempObject = ADM_JSAvidemuxAudio::JSInit(g_pCx,g_pObject);
	ADM_JSAvidemuxAudio *pAudio = new ADM_JSAvidemuxAudio();
	pAudio->setObject(new ADM_AvidemuxAudio());
	JS_SetPrivate(g_pCx,pTempObject,pAudio);
	m_pAudio = pTempObject;

	// initialize video property
	pTempObject = ADM_JSAvidemuxVideo::JSInit(g_pCx,g_pObject);
	ADM_JSAvidemuxVideo *pVideo = new ADM_JSAvidemuxVideo();
	pVideo->setObject(new ADM_AvidemuxVideo());
	JS_SetPrivate(g_pCx,pTempObject,pVideo);
	m_pVideo = pTempObject;
}// end ADM_Avidemux

ADM_Avidemux::~ADM_Avidemux()
{// begin ~ADM_Avidemux

}// end ~ADM_Avidemux

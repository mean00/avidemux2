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
/******************************************/
JSPropertySpec *ADM_JsAudioGetProperties(void);
JSFunctionSpec *ADM_JsAudioGetFunctions(void);
JSPropertySpec *ADM_JsVideoGetProperties(void);
JSFunctionSpec *ADM_JsVideoGetFunctions(void);
JSPropertySpec *ADM_JsClassGetProperties(void);
JSFunctionSpec *ADM_JsClassGetFunctions(void);
JSFunctionSpec *ADM_JsDebugGetFunctions(void);

static void dumpFunc(JSFunctionSpec *f)
{
    while(f->name)
    {
        jsLog(JS_LOG_NORMAL,"     %s(..)\n",f->name);
        f++;
    }
}
static void dumpProps(JSPropertySpec *f)
{
    while(f->name)
    {
        jsLog(JS_LOG_NORMAL,"     %s=xxx\n",f->name);
        f++;
    }
}
/**
    \fn ADM_dumpJSHooks
    \brief Printf the additional avidemux functions/properties
*/
void ADM_dumpJSHooks(void)
{
    jsLog(JS_LOG_NORMAL,"Dumping JS interface\n");
    jsLog(JS_LOG_NORMAL,"********************\n");
    jsLog(JS_LOG_NORMAL,"Debug functions :\n");
    dumpFunc(ADM_JsDebugGetFunctions());
    jsLog(JS_LOG_NORMAL,"app.\n");
    dumpFunc(ADM_JsClassGetFunctions());
    dumpProps(ADM_JsClassGetProperties());
    jsLog(JS_LOG_NORMAL,"app.video\n");
    dumpFunc(ADM_JsVideoGetFunctions());
    dumpProps(ADM_JsVideoGetProperties());
    jsLog(JS_LOG_NORMAL,"app.audio\n");
    dumpFunc(ADM_JsAudioGetFunctions());
    dumpProps(ADM_JsAudioGetProperties());
    jsLog(JS_LOG_NORMAL,"Done\n");
    jsLog(JS_LOG_NORMAL,"****\n");

}
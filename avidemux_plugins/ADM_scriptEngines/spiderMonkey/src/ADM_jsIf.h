#ifndef ADM_JSIF_H
#define ADM_JSIF_H

#include "jsapi.h"

extern "C"
{
	void jsHelp(JSContext *cx, const char *s);
	void jsPrint(JSContext *cx, const char *s);
	void jsPopupError(const char *s);
	void jsPopupInfo(const char *s);
}

#endif
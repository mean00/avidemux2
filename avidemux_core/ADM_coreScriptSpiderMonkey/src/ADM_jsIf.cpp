#include <string.h>
#include <sstream>

using namespace std;

#include "ADM_inttype.h"
#include "SpiderMonkeyEngine.h"
#include "DIA_coreToolkit.h"
#include "ADM_jsIf.h"

extern void A_Resync(void);

/**
\fn jsEvaluate
*/
/*static bool jsEvaluate(const char *str)
{
jsval rval;
JS_EvaluateScript(g_pCx,g_pObject,str,strlen(str),"dummy",1,&rval);
return true; 
}*/
/**
\fn    interactiveECMAScript
\brief interprete & execute ecma script (interactive)
*/
/*bool interactiveECMAScript(const char *name)
{
ADM_startShell(jsEvaluate);
JS_GC(g_pCx);
A_Resync();
ADM_info("Ending JS shell...\n");
return true;
}*/

static void dump(SpiderMonkeyEngine *engine, JSFunctionSpec *f)
{
	while (f->name)
	{
		stringstream stream;

		stream << "    " << f->name;

		engine->callEventHandlers(IScriptEngine::EVENT_TYPE_INFORMATION, NULL, -1, stream.str().c_str());
		f++;
	}
}

void jsHelp(JSContext *cx, const char *s)
{
	SpiderMonkeyEngine *engine = (SpiderMonkeyEngine*)JS_GetContextPrivate(cx);
	int n = engine->jsHooks.size();

	if (!s)
	{
		goto none;
	}

	for (int i = 0; i < n; i++)
	{
		if (!strcasecmp(s, engine->jsHooks[i].name))
		{
			const char *t = engine->jsHooks[i].text;

			if(t)
			{
				engine->callEventHandlers(IScriptEngine::EVENT_TYPE_INFORMATION, NULL, -1, t);
			}

			dump(engine, engine->jsHooks[i].jsFunctions);
		}
	}

none:
	engine->callEventHandlers(IScriptEngine::EVENT_TYPE_INFORMATION, NULL, -1, "please use help(\"xxx\") with xx among");

	for (int i = 0; i < n; i++)
	{
		stringstream stream;

		stream << "    " << engine->jsHooks[i].name;

		engine->callEventHandlers(IScriptEngine::EVENT_TYPE_INFORMATION, NULL, -1, stream.str().c_str());
	}
}

void jsPrint(JSContext *cx, const char *s)
{
	SpiderMonkeyEngine *engine = (SpiderMonkeyEngine*)JS_GetContextPrivate(cx);

	engine->callEventHandlers(IScriptEngine::EVENT_TYPE_INFORMATION, NULL, -1, s);
}

void jsPopupError(const char *s)
{// begin displayError

	GUI_Verbose();
	GUI_Error_HIG("Error",s);
	GUI_Quiet();

}// end displayError
/**
\fn displayInfo
\brief info popup
*/

void jsPopupInfo(const char *s)
{// begin displayInfo

	GUI_Verbose();
	GUI_Info_HIG(ADM_LOG_IMPORTANT,"Info",s);
	GUI_Quiet();

}// end displayInfo

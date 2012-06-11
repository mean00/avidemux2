#define __DECLARE__
#include "ADM_default.h"
#include "ADM_script.h"

#ifdef USE_QTSCRIPT
#include "QtScriptEngine.h"
#endif

#ifdef USE_TINYPY
#include "PythonEngine.h"
#endif

#ifdef USE_SPIDERMONKEY
#include "SpiderMonkeyEngine.h"
#endif

#include "ScriptShell.h"
#include "A_functions.h"

using namespace std;

static void consoleEventHandler(IScriptEngine::EngineEvent *event)
{
	printf("[Script] %s ", event->engine->name().c_str());

	switch (event->eventType)
	{
		case IScriptEngine::Information:
			printf("INFO - ");
			break;

		case IScriptEngine::Warning:
			printf("WARNING - ");
			break;

		case IScriptEngine::Error:
			printf("ERROR - ");
			break;

		default:
			printf("UNKNOWN - ");
	}

	printf("%s\n", event->message);
}

vector<IScriptEngine*> initialiseScriptEngines(IEditor *editor)
{
    ADM_assert(engines.size() == 0);

#ifdef USE_TINYPY
	engines.push_back(new PythonEngine());
#endif

#ifdef USE_QTSCRIPT
	engines.push_back(new QtScriptEngine());
#endif

#ifdef USE_SPIDERMONKEY
	engines.push_back(new SpiderMonkeyEngine());
#endif

	for (int i = 0; i < engines.size(); i++)
	{
		engines[i]->registerEventHandler(consoleEventHandler);
		engines[i]->initialise(editor);
	}

	return engines;
}

void destroyScriptEngines()
{
	for (int i = 0; i < engines.size(); i++)
	{
		delete engines[i];
	}
}

static IScriptEngine* getEngine(vector<IScriptEngine*> engines, string engineName)
{
    IScriptEngine *engine = NULL;

    for (int i = 0; i < engines.size(); i++)
	{
		if (engines[i]->name().compare(engineName) == 0)
		{
		    engine = engines[i];
		    break;
		}
	}

	return engine;
}

#ifdef USE_QTSCRIPT
IScriptEngine* getQtScriptEngine()
{
    return getEngine(engines, "QtScript");
}
#endif

#ifdef USE_SPIDERMONKEY
IScriptEngine* getSpiderMonkeyEngine()
{
    return getEngine(engines, "SpiderMonkey");
}
#endif

#ifdef USE_TINYPY
IScriptEngine* getPythonEngine()
{
    return getEngine(engines, "Python");
}
#endif

// This shouldn't really be used but since the UI isn't very OOP it's kinda necessary at the moment
vector<IScriptEngine*> getScriptEngines()
{
	return engines;
}

void interactiveScript(IScriptEngine *engine)
{
	ADM_startShell(engine);
	A_Resync();

	ADM_info("Ending shell...\n");
}

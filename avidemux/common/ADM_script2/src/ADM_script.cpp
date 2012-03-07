#define __DECLARE__
#include "ADM_script.h"
#include "PythonEngine.h"
#include "SpiderMonkeyEngine.h"
#include "ScriptShell.h"
#include "A_functions.h"

using namespace std;

static void consoleEventHandler(IScriptEngine::EngineEvent *event)
{
	printf("[Script] %s ", event->engine->getName().c_str());

	switch (event->eventType)
	{
		case IScriptEngine::EVENT_TYPE_INFORMATION:
			printf("INFO - ");
			break;

		case IScriptEngine::EVENT_TYPE_ERROR:
			printf("ERROR - ");
			break;

		default:
			printf("UNKNOWN - ");
	}

	printf("%s\n", event->message);
}

list<IScriptEngine*> initialiseScriptEngines(IEditor *editor)
{
    ADM_assert(engines.size() == 0);

	engines.push_back(new PythonEngine());
	engines.push_back(new SpiderMonkeyEngine());

	for (list<IScriptEngine*>::iterator it = engines.begin(); it != engines.end(); it++)
	{
		(*it)->registerEventHandler(consoleEventHandler);
		(*it)->initialise(editor);
	}

	return engines;
}

void destroyScriptEngines()
{
	for (list<IScriptEngine*>::iterator it = engines.begin(); it != engines.end(); it++)
	{
		delete (*it);
	}
}

static IScriptEngine* getEngine(list<IScriptEngine*> engines, string engineName)
{
    IScriptEngine *engine = NULL;

    for (list<IScriptEngine*>::iterator it = engines.begin(); it != engines.end(); it++)
	{
		if ((*it)->getName().compare(engineName) == 0)
		{
		    engine = (*it);
		    break;
		}
	}

	return engine;
}

IScriptEngine* getSpiderMonkeyEngine()
{
    return getEngine(engines, "SpiderMonkey");
}

IScriptEngine* getPythonEngine()
{
    return getEngine(engines, "Python");
}

void interactiveScript(IScriptEngine *engine)
{
	ADM_startShell(engine);
	A_Resync();

	ADM_info("Ending shell...\n");
}

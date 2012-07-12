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

static std::vector<IScriptEngine*> engines;

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
	engines.push_back(new ADM_qtScript::QtScriptEngine());
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

static int getEngineMaturityRanking(IScriptEngine* engine)
{
	if (engine->name().compare("QtScript"))
	{
		return 2;
	}
	else if (engine->name().compare("Tinypy"))
	{
		return 1;
	}

	return 0;
}

IScriptEngine* getDefaultScriptEngine()
{
    IScriptEngine *engine = NULL;
	int lastRank = -1;

    for (int i = 0; i < engines.size(); i++)
	{
		int rank = getEngineMaturityRanking(engines[i]);

		if (rank > lastRank)
		{
			engine = engines[i];
		}
	}

	return engine;
}

// This shouldn't really be used but since the UI isn't very OOP it's kinda necessary at the moment
const vector<IScriptEngine*>& getScriptEngines()
{
	return engines;
}

void interactiveScript(IScriptEngine *engine)
{
	ADM_startShell(engine);
	A_Resync();

	ADM_info("Ending shell...\n");
}

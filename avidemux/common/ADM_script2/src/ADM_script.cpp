#include <algorithm>

#include "ADM_script.h"
#include "ADM_ScriptEngineLoader.h"
#include "ScriptShell.h"
#include "A_functions.h"

using namespace std;

static vector<ADM_ScriptEngineLoader*> engineLoaders;
static vector<IScriptEngine*> engines;

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

bool compareEngineRank(IScriptEngine *engine1, IScriptEngine *engine2)
{
	return engine1->maturityRanking() > engine2->maturityRanking();
}

const vector<IScriptEngine*>& initialiseScriptEngines(const char* path, IEditor *editor)
{
    ADM_assert(engines.size() == 0);

    const int maxEngineCount = 50;
    char *files[maxEngineCount];
    uint32_t fileCount;

    memset(files, 0, sizeof(char *) * maxEngineCount);
    printf("[Script] Scanning directory %s\n", path);

    if (!buildDirectoryContent(&fileCount, path, files, maxEngineCount, SHARED_LIB_EXT))
    {
        printf("[Script] Cannot parse plugin\n");

        return engines;
    }

    for (int index = 0; index < fileCount; index++)
    {
        ADM_ScriptEngineLoader *loader = new ADM_ScriptEngineLoader(files[index]);

        if (loader->isAvailable())
        {
            IScriptEngine *engine = loader->createEngine();

            engine->registerEventHandler(consoleEventHandler);
            engine->initialise(editor);

            engineLoaders.push_back(loader);
            engines.push_back(engine);
        }
        else
        {
            delete loader;
            printf("[Script] ERROR - Unable to load %s\n", files[index]);
        }
    }

	sort(engines.begin(), engines.end(), compareEngineRank);

    return engines;
}

void destroyScriptEngines()
{
    for (int i = 0; i < engines.size(); i++)
    {
        delete engines[i];
    }

	for (int i = 0; i < engines.size(); i++)
	{
		delete engineLoaders[i];
	}
}

IScriptEngine* getDefaultScriptEngine()
{
	return engines.size() == 0 ? NULL : engines[0];
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

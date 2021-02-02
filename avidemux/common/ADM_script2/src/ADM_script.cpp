#include <algorithm>
#include "ADM_assert.h"
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
/**
 * 
 */
static void tryLoadingEngine(const char* path, IEditor *editor)
{
    std::vector<std::string> files;
    printf("[Script] Scanning directory %s\n", path);

    if (!buildDirectoryContent(path, &files, SHARED_LIB_EXT))
    {
        printf("[Script] Cannot open plugin directory\n");
        return ;
    }

    for (int index = 0; index < files.size(); index++)
    {
        ADM_ScriptEngineLoader *loader = new ADM_ScriptEngineLoader(files.at(index).c_str());

        if (loader->isAvailable())
        {
            IScriptEngine *engine = loader->createEngine();

            engine->registerEventHandler(consoleEventHandler);
            engine->initialise(editor);

            engineLoaders.push_back(loader);
            engines.push_back(engine);
            printf("[Script] loaded %s\n", files.at(index).c_str());
        }
        else
        {
            delete loader;
            printf("[Script] ERROR - Unable to load %s\n", files.at(index).c_str());
        }
    }
}

/**
 * 
 * @param path
 * @param editor
 * @return 
 */
const vector<IScriptEngine*>& initialiseScriptEngines(const char* path, IEditor *editor,const char *subFolder)
{
    ADM_assert(engines.size() == 0);

    std::string p=std::string(path);
    tryLoadingEngine(p.c_str(),editor);
    p+=std::string("/")+std::string(subFolder);
    tryLoadingEngine(p.c_str(),editor);
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
    engines.clear();
    engineLoaders.clear();
}

IScriptEngine* getDefaultScriptEngine()
{
	return engines.size() == 0 ? NULL : engines[0];
}
/**
    \fn getPythonScriptEngine
*/
IScriptEngine* getPythonScriptEngine()
{
    int n=engines.size();
    if(!n) return NULL;
    for(int i=0;i<n;i++)
    {
        IScriptEngine *ng=engines[i];
        if(!ng->defaultFileExtension().compare("py"))
                return ng;
    }
	return NULL;
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

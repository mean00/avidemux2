#ifndef ADM_ScriptEngineLoader_h
#define ADM_ScriptEngineLoader_h

#include "ADM_coreScript_export.h"
#include "ADM_dynamicLoading.h"
#include "IScriptEngine.h"

typedef IScriptEngine* (ADM_ScriptEngine_CreateFunction)();

class ADM_CORESCRIPT_EXPORT ADM_ScriptEngineLoader : public ADM_LibWrapper
{
public:
    ADM_ScriptEngine_CreateFunction* createEngine;

    ADM_ScriptEngineLoader(const char *file);
};

#endif

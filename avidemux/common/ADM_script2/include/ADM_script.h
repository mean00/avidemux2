#ifndef ADM_SCRIPT_H
#define ADM_SCRIPT_H
#include "ADM_assert.h"
#include <vector>
#include "IScriptEngine.h"

const std::vector<IScriptEngine*>& initialiseScriptEngines(const char *path, IEditor *editor);
void destroyScriptEngines();
const std::vector<IScriptEngine*>& getScriptEngines();
IScriptEngine* getDefaultScriptEngine();
void interactiveScript(IScriptEngine *engine);
IScriptEngine* getPythonScriptEngine();
#endif
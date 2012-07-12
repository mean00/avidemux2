#ifndef ADM_SCRIPT_H
#define ADM_SCRIPT_H

#include <vector>
#include "IScriptEngine.h"

#ifndef __DECLARE__
extern
#endif

std::vector<IScriptEngine*> initialiseScriptEngines(IEditor *editor);
void destroyScriptEngines();
const std::vector<IScriptEngine*>& getScriptEngines();
IScriptEngine* getDefaultScriptEngine();
void interactiveScript(IScriptEngine *engine);

#endif
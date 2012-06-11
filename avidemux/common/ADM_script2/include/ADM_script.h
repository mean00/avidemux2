#ifndef ADM_SCRIPT_H
#define ADM_SCRIPT_H

#include <vector>
#include "IScriptEngine.h"

#ifndef __DECLARE__
extern
#endif
std::vector<IScriptEngine*> engines;

std::vector<IScriptEngine*> initialiseScriptEngines(IEditor *editor);
void destroyScriptEngines();
IScriptEngine* getQtScriptEngine();
IScriptEngine* getPythonEngine();
IScriptEngine* getSpiderMonkeyEngine();
std::vector<IScriptEngine*> getScriptEngines();
void interactiveScript(IScriptEngine *engine);

#endif
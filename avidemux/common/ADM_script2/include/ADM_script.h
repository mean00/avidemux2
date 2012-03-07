#ifndef ADM_SCRIPT_H
#define ADM_SCRIPT_H

#include <list>
#include "IScriptEngine.h"

#ifndef __DECLARE__
extern
#endif
std::list<IScriptEngine*> engines;

std::list<IScriptEngine*> initialiseScriptEngines(IEditor *editor);
void destroyScriptEngines();
IScriptEngine* getSpiderMonkeyEngine();
IScriptEngine* getPythonEngine();
void interactiveScript(IScriptEngine *engine);

#endif
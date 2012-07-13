#include "ADM_ScriptEngineLoader.h"

ADM_ScriptEngineLoader::ADM_ScriptEngineLoader(const char *file) : ADM_LibWrapper()
{
	this->initialised = (loadLibrary(file) && getSymbols(1, &createEngine, "createEngine"));
}

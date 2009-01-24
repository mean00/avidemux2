#include "ADM_inttype.h"
#include "configGuiLoader.h"

configGuiLoader::configGuiLoader(const char *file) : ADM_LibWrapper()
{
	initialised = (loadLibrary(file) && getSymbols(1, &showXvidConfigDialog, "showXvidConfigDialog"));
}

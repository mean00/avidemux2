#ifndef configGuiLoader_h
#define configGuiLoader_h

#include "ADM_dynamicLoading.h"
#include "options.h"

extern "C"
{
#include "ADM_vidEnc_plugin.h"
}

typedef bool _showXvidConfigDialog(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties, vidEncOptions *encodeOptions, xvidOptions *options);

class configGuiLoader : public ADM_LibWrapper
{
	public:
		_showXvidConfigDialog *showXvidConfigDialog;

		configGuiLoader(const char *file);
};

#endif	// configGuiLoader_h

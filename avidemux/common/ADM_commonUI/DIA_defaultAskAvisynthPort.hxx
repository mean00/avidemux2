#include "ADM_default.h"
#include "DIA_factory.h"
#include "prefs.h"

#include <iostream>

/**
	\fn UI_askAvisynthPort
	\brief Called if avisynth always ask is enabled
*/
bool UI_askAvisynthPort(uint32_t &port)
{
	uint32_t localPort = port;
	diaElemUInteger portFrame(&localPort,"Default port to use",1024,65535);
	diaElem * portLabel[] = { &portFrame};
	if(diaFactoryRun("Select the default port",1, (diaElem **)&portLabel))
	{
		//std::cerr << "\n\n\n\n\n" << localPort << "\n\n\n\n\n";
		prefs->set(AVISYNTH_AVISYNTH_LOCALPORT, localPort);
		port = localPort;
		return true;
	}
	return false;
}

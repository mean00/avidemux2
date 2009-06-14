/***************************************************************************
                              presetOptions.h

    begin                : Fri May 23 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef presetOptions_h
#define presetOptions_h

#include "options.h"

class x264PresetOptions : public x264Options
{
private:
	vidEncOptions _encodeOptions;

	void parseEncodeOptions(xmlNode *node, vidEncOptions *encodeOptions);
	void setEncodeOptionsToDefaults(void);

public:
	x264PresetOptions(void);

	vidEncOptions* getEncodeOptions(void);
	void setEncodeOptions(vidEncOptions* encodeOptions);
	char* toXml(void);
	int fromXml(const char *xml);
};

#endif	// presetOptions_h

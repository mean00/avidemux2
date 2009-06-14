/***************************************************************************
                               zoneOptions.h

    begin                : Thu May 15 2008
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

#ifndef zoneOptions_h
#define zoneOptions_h

extern "C"
{
#include "x264.h"
}

typedef enum
{
	ZONE_MODE_QUANTISER,
	ZONE_MODE_BITRATE_FACTOR
} zoneMode;

class x264ZoneOptions
{
private:
    unsigned int _frameStart, _frameEnd;
    zoneMode _zoneMode;
    unsigned int _parameter;

public:
	x264ZoneOptions(void);
	~x264ZoneOptions(void);

	zoneMode getZoneMode(void);
	unsigned int getFrameStart(void);
	unsigned int getFrameEnd(void);
	void setFrameRange(unsigned int frameStart, unsigned int frameEnd);
	unsigned int getZoneParameter(void);
	void setQuantiser(unsigned int quantiser);
	void setBitrateFactor(unsigned int bitrateFactor);
	void setX264Zone(x264_zone_t *zone);
	x264ZoneOptions* clone(void);
};

#endif	// zoneOptions_h

/***************************************************************************
                              zoneOptions.cpp

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

#include "ADM_inttype.h"
#include "zoneOptions.h"

x264ZoneOptions::x264ZoneOptions(void)
{
	_frameStart = _frameEnd = 0;
	_zoneMode = ZONE_MODE_QUANTISER;
	_parameter = 26;
}

x264ZoneOptions::~x264ZoneOptions(void)
{
}

zoneMode x264ZoneOptions::getZoneMode(void)
{
	return _zoneMode;
}

unsigned int x264ZoneOptions::getFrameStart(void)
{
	return _frameStart;
}

unsigned int x264ZoneOptions::getFrameEnd(void)
{
	return _frameEnd;
}

void x264ZoneOptions::setFrameRange(unsigned int frameStart, unsigned int frameEnd)
{
	_frameStart = frameStart;
	_frameEnd = frameEnd;
}

unsigned int x264ZoneOptions::getZoneParameter(void)
{
	return _parameter;
}

void x264ZoneOptions::setQuantiser(unsigned int quantiser)
{
	_zoneMode = ZONE_MODE_QUANTISER;
	_parameter = quantiser;
}

void x264ZoneOptions::setBitrateFactor(unsigned int bitrateFactor)
{
	_zoneMode = ZONE_MODE_BITRATE_FACTOR;
	_parameter = bitrateFactor;
}

void x264ZoneOptions::setX264Zone(x264_zone_t *zone)
{
	memset(zone, 0, sizeof(x264_zone_t));

	zone->i_start = _frameStart;
	zone->i_end = _frameEnd;
	zone->b_force_qp = (getZoneMode() == ZONE_MODE_QUANTISER);

	if (zone->b_force_qp)
		zone->i_qp = _parameter;
	else
		zone->f_bitrate_factor = _parameter / 100.;
}

x264ZoneOptions* x264ZoneOptions::clone(void)
{
	x264ZoneOptions* zoneOptions = new x264ZoneOptions;

	zoneOptions->setFrameRange(getFrameStart(), getFrameEnd());

	if (getZoneMode() == ZONE_MODE_QUANTISER)
		zoneOptions->setQuantiser(getZoneParameter());
	else
		zoneOptions->setBitrateFactor(getZoneParameter());

	return zoneOptions;
}

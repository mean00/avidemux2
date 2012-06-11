/***************************************************************************
     \file  ADM_edEditableAudioTracks.h
     \brief Editor class
     \author (C) 2012 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_EDEDITABLEAUDIOTRACK_H
#define ADM_EDEDITABLEAUDIOTRACK_H

#include "ADM_edAudioTrack.h"
#include "audiofilter_internal.h"
#include "audiofilter_conf.h"
#include "audioencoderInternal.h"

/**
    \class EditableAudioTrack
*/
class EditableAudioTrack
{
public:
	int objectId;
// source
	ADM_edAudioTrack            *edTrack;
// filter
	VectorOfAudioFilter         EncodingVector;
	ADM_AUDIOFILTER_CONFIG      audioEncodingConfig;
// encoder
	int                         encoderIndex;
	CONFcouple                  *encoderConf;
    int                         poolIndex;

	EditableAudioTrack();
	EditableAudioTrack(const EditableAudioTrack &src);
	~EditableAudioTrack();

private:
	static int objectCount;
};

#endif

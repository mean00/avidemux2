/***************************************************************************
    \file  ADM_edActiveAudioTrack
    \author mean fixounet@free.fr (C) 2012
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_edEditableAudioTrack.h"

EditableAudioTrack::EditableAudioTrack()
{
	encoderIndex = 0;
	encoderConf = NULL;
	edTrack = NULL;
    poolIndex=0;
	objectId = objectCount++;
}

EditableAudioTrack::EditableAudioTrack(const EditableAudioTrack &src)
{
	encoderIndex = src.encoderIndex;
	edTrack = src.edTrack;

	if (src.encoderConf)
	{
		encoderConf = CONFcouple::duplicate(src.encoderConf);
	}
	else
	{
		encoderConf = NULL;
	}

	EncodingVector = src.EncodingVector;
	audioEncodingConfig = src.audioEncodingConfig;
    poolIndex=src.poolIndex;
	objectId = objectCount++;
}

EditableAudioTrack::~EditableAudioTrack()
{
	edTrack = NULL;
	EncodingVector.clear(); // memleak ?

	if (encoderConf)
	{
		delete encoderConf;
	}

	encoderConf = NULL;
}

int EditableAudioTrack::objectCount = 0;

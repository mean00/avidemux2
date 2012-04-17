/***************************************************************************
     \file  ADM_edActiveAudioTracks.h
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
#ifndef ADM_EDACTIVEAUDIOTRACKS_H
#define ADM_EDACTIVEAUDIOTRACKS_H

#include "BVector.h"
#include "ADM_edEditableAudioTrack.h"
#include "ADM_edAudioTrack.h"

class ActiveAudioTracks
{
protected:
	BVector <EditableAudioTrack *> tracks;

public:
	ActiveAudioTracks();
	~ActiveAudioTracks();
	int size() const;
	EditableAudioTrack *atEditable(int ix);
	ADM_edAudioTrack *atEdAudio(int ix);
	bool addTrack(EditableAudioTrack *x);
	bool addTrack(int poolIndex,ADM_edAudioTrack *x);
	bool clear();
    bool dump();
};

#endif

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
	virtual ~ActiveAudioTracks();
	virtual unsigned int size() const;
	virtual EditableAudioTrack *atEditable(int ix);
	virtual ADM_edAudioTrack *atEdAudio(int ix);
	virtual bool addTrack(EditableAudioTrack *x);
	virtual bool addTrack(int poolIndex,ADM_edAudioTrack *x);
	virtual bool clear();
    virtual bool dump();
    virtual bool insertTrack(int index, int poolIndex, ADM_edAudioTrack *x);
    virtual void removeTrack(int removeTrack);
};

#endif

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
#include "ADM_edActiveAudioTracks.h"

ActiveAudioTracks::ActiveAudioTracks() {}

ActiveAudioTracks::~ActiveAudioTracks() {}

int ActiveAudioTracks::size() const
{
	return tracks.size();
}

EditableAudioTrack* ActiveAudioTracks::atEditable(int ix)
{
	if (ix >= size())
	{
		ADM_warning("Request to get track at %d, only %d tracks available\n", ix, size());
		return NULL;
	}

	return tracks[ix];
}

ADM_edAudioTrack* ActiveAudioTracks::atEdAudio(int ix)
{
	EditableAudioTrack *e = atEditable(ix);
	return e->edTrack;
}

bool ActiveAudioTracks::dump()
{
    for(int i=0;i<size();i++)
    {
        EditableAudioTrack *Editable=atEditable(i);
        ADM_info(" Active Track %d, pool index=%d\n",i,Editable->poolIndex);
        ADM_edAudioTrack   *track=Editable->edTrack;
        switch(track->getTrackType())
        {
            case   ADM_EDAUDIO_FROM_VIDEO: ADM_info("\t from video\n");break;
            case   ADM_EDAUDIO_EXTERNAL:   ADM_info("\t from external file\n");break;
            default : ADM_info("\t???\n");break; 
        }
    }
}

bool ActiveAudioTracks::addTrack(EditableAudioTrack *x)
{
	if (!x)
	{
		ADM_warning("Cannot add editable track to active track! \n");
		return false;
	}

	tracks.append(x) ;
	return true;
}

bool ActiveAudioTracks::addTrack(int poolIndex,ADM_edAudioTrack *x)
{
	if (!x)
	{
		ADM_warning("Cannot add track to active track! \n");
		return false;
	}

	EditableAudioTrack *e = new EditableAudioTrack;
    e->poolIndex=poolIndex;
	e->edTrack = x;
	tracks.append(e) ;
	return true;
}

bool ActiveAudioTracks::clear()
{
	int n = size();

	for (int i = 0; i < n; i++)
	{
		EditableAudioTrack *t = atEditable(i);


		delete t;
	}

	tracks.clear();
	return true;
}

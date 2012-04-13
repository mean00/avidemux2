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

bool ActiveAudioTracks::addTrack(ADM_edAudioTrack *x)
{
	if (!x)
	{
		ADM_warning("Cannot add track to active track! \n");
		return false;
	}

	EditableAudioTrack *e = new EditableAudioTrack;
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

		if (t->edTrack->destroyable())
		{
			delete t->edTrack;
		}

		delete t;
	}

	tracks.clear();
	return true;
}

#include "ADM_edPoolOfAudioTracks.h"
#include "ADM_edAudioTrackFromVideo.h"
#include "ADM_edAudioTrackExternal.h"

PoolOfAudioTracks::PoolOfAudioTracks() {}

int PoolOfAudioTracks::size() const
{
	return tracks.size();
}

ADM_edAudioTrack* PoolOfAudioTracks::at(int ix)
{
	if (ix >= size())
	{
		ADM_assert(0);
	}

	return tracks[ix];
}

bool PoolOfAudioTracks::addInternalTrack(ADM_edAudioTrack *x)
{
	tracks.append(x) ;
	return true;
};

bool PoolOfAudioTracks::clear()
{
	tracks.clear();
	return true;
}

/**
     \fn dump
*/
bool PoolOfAudioTracks::dump(void)
{
	for (int i = 0; i < size(); i++)
	{
		ADM_edAudioTrack *track = at(i);
		ADM_info("Track %d, pool index=%d : \n", i, i);

		switch (track->getTrackType())
		{
			case ADM_EDAUDIO_FROM_VIDEO:
			{
				ADM_edAudioTrackFromVideo *vid = track->castToTrackFromVideo();
				ADM_assert(vid);
				ADM_info("\t track %d from video\n", vid->getMyTrackIndex());
			}
			break;

			case ADM_EDAUDIO_EXTERNAL:
			{
				ADM_edAudioTrackExternal *vid = track->castToExternal();
				ADM_assert(vid);
				ADM_info("\t from file %s\n", vid->getMyName().c_str());
			}
			break;

			default:
				ADM_assert(0);
				break;
		}
	}

	return true;
}

PoolOfAudioTracks::~PoolOfAudioTracks()
{
	for (int i = 0; i < size(); i++)
	{
		ADM_edAudioTrack *track = at(i);

		if (true == track->destroyable())
		{
			delete track;
		}
	}

	clear();
}

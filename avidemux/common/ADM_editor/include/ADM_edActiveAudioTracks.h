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
	bool addTrack(ADM_edAudioTrack *x);
	bool clear();
};

#endif

#ifndef ADM_EDPOOLOFAUDIOTRACKS_H
#define ADM_EDPOOLOFAUDIOTRACKS_H

#include "BVector.h"
#include "ADM_edAudioTrack.h"

class PoolOfAudioTracks
{
protected:
	BVector <ADM_edAudioTrack *>tracks;

public:
	PoolOfAudioTracks();
	~PoolOfAudioTracks();
	int size() const;
	ADM_edAudioTrack *at(int ix);
	bool addInternalTrack(ADM_edAudioTrack *x);
	bool clear();
	bool dump(void);
};

#endif

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
	virtual ~PoolOfAudioTracks();
	virtual int size() const;
	virtual ADM_edAudioTrack *at(int ix);
	virtual bool addInternalTrack(ADM_edAudioTrack *x);
	virtual bool clear();
	virtual bool dump(void);
};

#endif

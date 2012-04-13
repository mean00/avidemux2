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
// source
	ADM_edAudioTrack            *edTrack;
// filter
	VectorOfAudioFilter         EncodingVector;
	ADM_AUDIOFILTER_CONFIG      audioEncodingConfig;
// encoder
	int                         encoderIndex;
	CONFcouple                  *encoderConf;
	ADM_audioEncoder            *audioEncoder;

	EditableAudioTrack();
	EditableAudioTrack(const EditableAudioTrack &src);
	~EditableAudioTrack();
};

#endif

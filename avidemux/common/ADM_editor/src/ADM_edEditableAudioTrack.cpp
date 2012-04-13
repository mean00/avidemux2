#include "ADM_edEditableAudioTrack.h"

EditableAudioTrack::EditableAudioTrack()
{
	encoderIndex = 0;
	encoderConf = NULL;
	audioEncoder = NULL;
	edTrack = NULL;
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

	audioEncoder = NULL;
	EncodingVector = src.EncodingVector;
	audioEncodingConfig = src.audioEncodingConfig;
}

EditableAudioTrack::~EditableAudioTrack()
{
	edTrack = NULL;

	if (audioEncoder)
	{
		delete audioEncoder;
	}

	audioEncoder = NULL;
	EncodingVector.clear(); // memleak ?

	if (encoderConf)
	{
		delete encoderConf;
	}

	encoderConf = NULL;
}

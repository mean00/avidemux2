#include "AudioProperties.h"

namespace ADM_qtScript
{
	AudioProperties::AudioProperties(
		IEditor * editor, ADM_audioStreamTrack * track) : QtScriptObject(editor)
	{
		this->_bitrate = (track->wavheader.byterate * 8) / 1000;
		this->_channels = track->wavheader.channels;
		this->_codec = getStrFromAudioCodec(track->wavheader.encoding);
		this->_frequency = track->wavheader.frequency;
	}

	QScriptValue AudioProperties::getBitrate(void)
	{
		return this->_bitrate;
	}

	QScriptValue AudioProperties::getChannels(void)
	{
		return this->_channels;
	}

	QScriptValue AudioProperties::getCodec(void)
	{
		return this->_codec;
	}

	QScriptValue AudioProperties::getFrequency(void)
	{
		return this->_frequency;
	}
}

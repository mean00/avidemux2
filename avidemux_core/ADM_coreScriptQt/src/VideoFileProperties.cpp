#include "VideoFileProperties.h"
#include "AudioProperties.h"

#include "ADM_codec.h"
#include "fourcc.h"

namespace ADM_qtScript
{
	VideoFileProperties::VideoFileProperties(IEditor * editor, _VIDEOS * video) : QtScriptObject(editor)
	{
		aviInfo info;

		video->_aviheader->getVideoInfo(&info);

		this->_duration = video->_aviheader->getVideoDuration();
		this->_fourCC = fourCC::tostring(info.fcc);
		this->_frameRate = info.fps1000;
		this->_height = info.height;
		this->_parHeight = video->decoder->getPARHeight();
		this->_parWidth = video->decoder->getPARWidth();
		this->_name = video->_aviheader->getMyName();
		this->_width = info.width;
		this->_video = video;

		this->initialiseAudioProperties();
	}

	VideoFileProperties::~VideoFileProperties()
	{
		for (unsigned int trackIndex = 0; trackIndex < this->_audioProps.size(); trackIndex++)
		{
			delete this->_audioProps[trackIndex];
		}
	}

	QScriptValue VideoFileProperties::getDuration(void)
	{
		return this->_duration;
	}

	QScriptValue VideoFileProperties::getFourCC(void)
	{
		return this->_fourCC;
	}

	QScriptValue VideoFileProperties::getFrameRate(void)
	{
		return this->_frameRate;
	}

	QScriptValue VideoFileProperties::getHeight(void)
	{
		return this->_height;
	}

	QScriptValue VideoFileProperties::getParHeight(void)
	{
		return this->_parHeight;
	}

	QScriptValue VideoFileProperties::getParWidth(void)
	{
		return this->_parWidth;
	}

	QScriptValue VideoFileProperties::getName(void)
	{
		return this->_name;
	}

	QScriptValue VideoFileProperties::getWidth(void)
	{
		return this->_width;
	}

	QScriptValue VideoFileProperties::getAudioProperties()
	{
		int count = this->_audioProps.size();

		if (count)
		{
			QScriptValue audioTracks = this->engine()->newArray(count);

			for (int audioIndex = 0; audioIndex < count; audioIndex++)
			{
				audioTracks.setProperty(
					audioIndex, this->engine()->newQObject(
						this->_audioProps[audioIndex], QScriptEngine::ScriptOwnership));
			}

			return audioTracks;
		}
		else
		{
			return QScriptValue(QScriptValue::NullValue);
		}
	}

	void VideoFileProperties::initialiseAudioProperties()
	{
		for (vector<ADM_audioStreamTrack*>::size_type audioIndex = 0;
				audioIndex < this->_video->audioTracks.size(); audioIndex++)
		{
			this->_audioProps.push_back(
				new AudioProperties(this->_editor, this->_video->audioTracks[audioIndex]));
		}
	}
}

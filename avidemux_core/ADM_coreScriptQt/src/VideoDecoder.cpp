#include <QtScript/QScriptEngine>

#include "VideoDecoder.h"
#include "VideoFileProperties.h"

namespace ADM_qtScript
{
	VideoDecoder::VideoDecoder(
		QScriptEngine *engine, IEditor* editor, _VIDEOS *video) : QtScriptConfigObject(editor)
	{
		this->_video = video;
		this->_videoFile = video->_aviheader->getMyName();
		this->_configObject = this->createConfigContainer(engine);
	}

	QScriptValue VideoDecoder::getConfiguration(void)
	{
		return this->_configObject;
	}

	QScriptValue VideoDecoder::getName(void)
	{
		if (verifyVideo())
		{
			return this->_video->decoder->getDecoderName();
		}
		else
		{
			return this->engine()->undefinedValue();
		}
	}

	QScriptValue VideoDecoder::getVideoFileProperties(void)
	{
		if (verifyVideo())
		{
			return this->engine()->newQObject(new VideoFileProperties(this->_editor, this->_video));
		}
		else
		{
			return this->engine()->undefinedValue();
		}
	}

	void VideoDecoder::resetConfiguration(void)
	{
		if (verifyVideo())
		{
			this->_video->decoder->resetConfiguration();
		}
	}

	void VideoDecoder::getConfCouple(CONFcouple** conf, const QString& containerName)
	{
		if (verifyVideo())
		{
			this->_video->decoder->getConfiguration(conf);
		}
		else
		{
			*conf = NULL;
		}
	}

	void VideoDecoder::setConfCouple(CONFcouple* conf, const QString& containerName)
	{
		if (verifyVideo())
		{
			this->_video->decoder->setConfiguration(conf);
		}
	}

	bool VideoDecoder::verifyVideo()
	{
		for (int videoIndex = 0; videoIndex < this->_editor->getVideoCount(); videoIndex++)
		{
			_VIDEOS *video = this->_editor->getRefVideo(videoIndex);

			if (video == this->_video && video->_aviheader->getMyName() == this->_videoFile)
			{
				return true;
			}
		}

		return false;
	}
}

#include "AudioEncoder.h"
#include "MyQScriptEngine.h"

namespace ADM_qtScript
{
	AudioEncoder::AudioEncoder(
		QScriptEngine* engine, IEditor* editor, ADM_audioEncoder* encoder, int encoderIndex,
		EditableAudioTrack* track) : QtScriptConfigObject(editor)
	{
		this->_encoderConf = NULL;
		this->encoderPlugin = encoder;
		this->encoderIndex = encoderIndex;
		this->_track = track;

		if (track == NULL)
		{
			this->resetConfiguration();
		}

		this->_configObject = this->createConfigContainer(
								  engine, QtScriptConfigObject::defaultConfigGetterSetter);
	}

	AudioEncoder::~AudioEncoder()
	{
		if (this->_encoderConf)
		{
			delete this->_encoderConf;
		}
	}

	QScriptValue AudioEncoder::constructor(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->isCalledAsConstructor())
		{
			AudioEncoder *audioEncProto = qobject_cast<AudioEncoder*>(
											  context->thisObject().prototype().toQObject());
			AudioEncoder *audioEnc = new AudioEncoder(
				engine, static_cast<MyQScriptEngine*>(engine)->wrapperEngine->editor(), audioEncProto->encoderPlugin,
				audioEncProto->encoderIndex);

			return engine->newQObject(audioEnc, QScriptEngine::ScriptOwnership);
		}

		return engine->undefinedValue();
	}


	QScriptValue AudioEncoder::getName(void)
	{
		return this->encoderPlugin->menuName;
	}

	QScriptValue AudioEncoder::getConfiguration(void)
	{
		return this->_configObject;
	}

	void AudioEncoder::resetConfiguration(void)
	{
		if (this->encoderPlugin && this->encoderPlugin->getDefaultConfiguration)
		{
			CONFcouple* couple;

			this->encoderPlugin->getDefaultConfiguration(&couple);
			this->setEncoderConfiguration(couple);
		}
	}

	void AudioEncoder::getConfCouple(CONFcouple** conf, const QString& containerName)
	{
		*conf = CONFcouple::duplicate(this->getEncoderConfiguration());
	}

	void AudioEncoder::setConfCouple(CONFcouple* conf, const QString& containerName)
	{
		this->setEncoderConfiguration(CONFcouple::duplicate(conf));
	}

	CONFcouple* AudioEncoder::getEncoderConfiguration()
	{
		if (this->_track)
		{
			return this->_track->encoderConf;
		}
		else
		{
			return this->_encoderConf;
		}
	}

	void AudioEncoder::setEncoderConfiguration(CONFcouple *couple)
	{
		if (this->_encoderConf)
		{
			delete this->_encoderConf;
			this->_encoderConf = NULL;
		}

		if (_track)
		{
			if (this->_track->encoderConf)
			{
				delete this->_track->encoderConf;
			}

			this->_track->encoderConf = couple;
		}
		else
		{
			this->_encoderConf = couple;
		}
	}

	void AudioEncoder::useEncoderForAudioOutput(EditableAudioTrack* track)
	{
		this->_track = track;
		this->_track->encoderIndex = this->encoderIndex;
		this->setEncoderConfiguration(CONFcouple::duplicate(this->_encoderConf));
	}

	bool AudioEncoder::isEncoderUsed()
	{
		return this->_track != NULL;
	}
}

#include "AudioEncoder.h"
#include "AudioOutputCollectionPrototype.h"

namespace ADM_qtScript
{
	AudioOutputCollectionPrototype::AudioOutputCollectionPrototype(QObject* parent, IEditor* editor) :
		QtScriptObject(editor)
	{
		this->setParent(parent);
		this->_tracks = editor->getPoolOfActiveAudioTrack();
	}

	QScriptValue AudioOutputCollectionPrototype::getLength()
	{
		return this->_tracks->size();
	}

	QScriptValue AudioOutputCollectionPrototype::add(int inputTrackIndex, QScriptValue encoder)
	{
		PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();
		AudioEncoder *encoderObject = qobject_cast<AudioEncoder*>(encoder.toQObject());

		if (audioTracks->size() == 0)
		{
			return this->throwError(
					   "The source video doesn't contain a valid audio track to use for encoding.");
		}

		QScriptValue result = this->validateNumber(
								  "inputTrackIndex", inputTrackIndex, 0, audioTracks->size() - 1);

		if (!result.isUndefined())
		{
			return result;
		}

		if (encoderObject != NULL && encoderObject->isEncoderUsed())
		{
			return this->throwError("Audio encoder is already being used by another audio output.");
		}

		int index = this->_tracks->size();
		this->_tracks->addTrack(inputTrackIndex, audioTracks->at(inputTrackIndex));

		if (encoderObject != NULL)
		{
			encoderObject->useEncoderForAudioOutput(this->_tracks->atEditable(index));
			this->_editor->updateDefaultAudioTrack();
		}

		return index;
	}

	QScriptValue AudioOutputCollectionPrototype::add(QString audioFile, QScriptValue encoder)
	{
		AudioEncoder *encoderObject = qobject_cast<AudioEncoder*>(encoder.toQObject());
		PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();

		if (encoderObject != NULL && encoderObject->isEncoderUsed())
		{
			return this->throwError("Audio encoder is already being used by another audio output.");
		}

		int index = this->_tracks->size();
		this->_editor->addExternalAudioTrack(audioFile.toUtf8().constData());
		this->_tracks->addTrack(audioTracks->size() - 1, audioTracks->at(audioTracks->size() - 1));

		if (encoderObject != NULL)
		{
			encoderObject->useEncoderForAudioOutput(this->_tracks->atEditable(index));
			this->_editor->updateDefaultAudioTrack();
		}

		return index;
	}

	void AudioOutputCollectionPrototype::clear()
	{
		this->_editor->clearAudioTracks();
	}

	void AudioOutputCollectionPrototype::insert(int index, int inputTrackIndex, QScriptValue encoder)
	{
		PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();
		AudioEncoder* encoderObject = qobject_cast<AudioEncoder*>(encoder.toQObject());

		if (index < 0 || index > this->_tracks->size())
		{
			this->throwError("Index is out of range");
			return;
		}

		if (audioTracks->size() == 0)
		{
			this->throwError(
				"The source video doesn't contain a valid audio track to use for encoding.");
			return;
		}

		QScriptValue result = this->validateNumber(
								  "inputTrackIndex", inputTrackIndex, 0, audioTracks->size() - 1);

		if (!result.isUndefined())
		{
			return;
		}

		if (encoderObject != NULL && encoderObject->isEncoderUsed())
		{
			this->throwError("Audio encoder is already being used by another audio output.");
			return;
		}

		this->_tracks->insertTrack(index, inputTrackIndex, audioTracks->at(inputTrackIndex));

		if (encoderObject != NULL)
		{
			encoderObject->useEncoderForAudioOutput(this->_tracks->atEditable(index));
			this->_editor->updateDefaultAudioTrack();
		}
	}

	void AudioOutputCollectionPrototype::insert(int index, QString externalAudioFile, QScriptValue encoder)
	{
		PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();
		AudioEncoder* encoderObject = qobject_cast<AudioEncoder*>(encoder.toQObject());

		if (encoderObject != NULL && encoderObject->isEncoderUsed())
		{
			this->throwError("Audio encoder is already being used by another audio output.");
			return;
		}

		this->_editor->addExternalAudioTrack(externalAudioFile.toUtf8().constData());
		this->_tracks->insertTrack(
			index, audioTracks->size() - 1, audioTracks->at(audioTracks->size() - 1));

		if (encoderObject != NULL)
		{
			encoderObject->useEncoderForAudioOutput(this->_tracks->atEditable(index));
			this->_editor->updateDefaultAudioTrack();
		}
	}

	void AudioOutputCollectionPrototype::removeAt(int index)
	{
		if (index < 0 || index >= this->_tracks->size())
		{
			this->throwError("Index is out of range");
			return;
		}

		this->_tracks->removeTrack(index);
		this->_editor->updateDefaultAudioTrack();
	}
}

#include <QtScript/QScriptClassPropertyIterator>

#include "AudioOutput.h"
#include "AudioOutputCollection.h"
#include "AudioOutputCollectionPrototype.h"

namespace ADM_qtScript
{
	class AudioOutputCollectionClassPropertyIterator : public QScriptClassPropertyIterator
	{
	public:
		AudioOutputCollectionClassPropertyIterator(IEditor *editor, const QScriptValue &object);
		~AudioOutputCollectionClassPropertyIterator();

		bool hasNext() const;
		void next();

		bool hasPrevious() const;
		void previous();

		void toFront();
		void toBack();

		QScriptString name() const;
		uint id() const;

	private:
		IEditor *_editor;
		int _index;
		int _last;
	};

	AudioOutputCollection::AudioOutputCollection(QScriptEngine *engine, IEditor *editor) :
		QObject(engine), QScriptClass(engine)
	{
		this->_editor = editor;
		this->_prototype = engine->newQObject(
							   new AudioOutputCollectionPrototype(this, editor), QScriptEngine::ScriptOwnership,
							   QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods |
							   QScriptEngine::ExcludeSuperClassProperties);
	}

	QString AudioOutputCollection::name() const
	{
		return QLatin1String("AudioOutputCollection");
	}

	QScriptClassPropertyIterator* AudioOutputCollection::newIterator(const QScriptValue &object)
	{
		return new AudioOutputCollectionClassPropertyIterator(this->_editor, object);
	}

	QScriptValue AudioOutputCollection::property(
		const QScriptValue &object,	const QScriptString &name, uint id)
	{
		ActiveAudioTracks* tracks = this->_editor->getPoolOfActiveAudioTrack();

		qint32 pos = id;

		if ((pos < 0) || (pos >= tracks->size()))
		{
			return QScriptValue();
		}

		return this->engine()->newQObject(
				   new AudioOutput(this->_editor, tracks->atEditable(pos)), QScriptEngine::ScriptOwnership);
	}

	QScriptValue::PropertyFlags AudioOutputCollection::propertyFlags(
		const QScriptValue &object, const QScriptString &name, uint id)
	{
		return QScriptValue::Undeletable;
	}

	QScriptValue AudioOutputCollection::prototype() const
	{
		return this->_prototype;
	}

	QScriptClass::QueryFlags AudioOutputCollection::queryProperty(
		const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
	{
		ActiveAudioTracks* tracks = this->_editor->getPoolOfActiveAudioTrack();
		bool isArrayIndex;
		qint32 pos = name.toArrayIndex(&isArrayIndex);

		if (!isArrayIndex)
		{
			return 0;
		}

		*id = pos;

		if (pos >= tracks->size())
		{
			return 0;
		}

		return flags;
	}

	AudioOutputCollectionClassPropertyIterator::AudioOutputCollectionClassPropertyIterator(
		IEditor* editor, const QScriptValue &object) : QScriptClassPropertyIterator(object)
	{
		this->_editor = editor;
		this->toFront();
	}

	AudioOutputCollectionClassPropertyIterator::~AudioOutputCollectionClassPropertyIterator() {	}

	bool AudioOutputCollectionClassPropertyIterator::hasNext() const
	{
		ActiveAudioTracks* tracks = _editor->getPoolOfActiveAudioTrack();

		return _index < tracks->size();
	}

	void AudioOutputCollectionClassPropertyIterator::next()
	{
		_last = _index;
		++_index;
	}

	bool AudioOutputCollectionClassPropertyIterator::hasPrevious() const
	{
		return (_index > 0);
	}

	void AudioOutputCollectionClassPropertyIterator::previous()
	{
		--_index;
		_last = _index;
	}

	void AudioOutputCollectionClassPropertyIterator::toFront()
	{
		_index = 0;
		_last = -1;
	}

	void AudioOutputCollectionClassPropertyIterator::toBack()
	{
		ActiveAudioTracks* tracks = _editor->getPoolOfActiveAudioTrack();

		_index = tracks->size();
		_last = -1;
	}

	QScriptString AudioOutputCollectionClassPropertyIterator::name() const
	{
		return object().engine()->toStringHandle(QString::number(_last));
	}

	uint AudioOutputCollectionClassPropertyIterator::id() const
	{
		return _last;
	}
}

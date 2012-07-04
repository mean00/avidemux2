#include <QtScript/QScriptClassPropertyIterator>

#include "Segment.h"
#include "SegmentCollection.h"
#include "SegmentCollectionPrototype.h"

namespace ADM_qtScript
{
	class SegmentCollectionClassPropertyIterator : public QScriptClassPropertyIterator
	{
	public:
		SegmentCollectionClassPropertyIterator(IEditor *editor, const QScriptValue &object);
		~SegmentCollectionClassPropertyIterator();

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
		uint _index;
		int _last;
	};

	SegmentCollection::SegmentCollection(QScriptEngine *engine, IEditor *editor) :
		QObject(engine), QScriptClass(engine)
	{
		this->_editor = editor;
		this->_prototype = engine->newQObject(
							   new SegmentCollectionPrototype(this, editor), QScriptEngine::ScriptOwnership,
							   QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods |
							   QScriptEngine::ExcludeSuperClassProperties);
	}

	QString SegmentCollection::name() const
	{
		return QLatin1String("SegmentCollection");
	}

	QScriptClassPropertyIterator* SegmentCollection::newIterator(const QScriptValue &object)
	{
		return new SegmentCollectionClassPropertyIterator(this->_editor, object);
	}

	QScriptValue SegmentCollection::property(
		const QScriptValue &object,	const QScriptString &name, uint id)
	{
		if (id >= this->_editor->getNbSegment())
		{
			return QScriptValue();
		}

		return this->engine()->newQObject(
				   new Segment(this->_editor, this->_editor->getSegment(id)), QScriptEngine::ScriptOwnership);
	}

	QScriptValue::PropertyFlags SegmentCollection::propertyFlags(
		const QScriptValue &object, const QScriptString &name, uint id)
	{
		return QScriptValue::Undeletable;
	}

	QScriptValue SegmentCollection::prototype() const
	{
		return this->_prototype;
	}

	QScriptClass::QueryFlags SegmentCollection::queryProperty(
		const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
	{
		bool isArrayIndex;
		quint32 pos = name.toArrayIndex(&isArrayIndex);

		if (!isArrayIndex)
		{
			return 0;
		}

		*id = pos;

		if (pos >= this->_editor->getNbSegment())
		{
			return 0;
		}

		return flags;
	}

	SegmentCollectionClassPropertyIterator::SegmentCollectionClassPropertyIterator(
		IEditor* editor, const QScriptValue &object) : QScriptClassPropertyIterator(object)
	{
		this->_editor = editor;
		this->toFront();
	}

	SegmentCollectionClassPropertyIterator::~SegmentCollectionClassPropertyIterator() {	}

	bool SegmentCollectionClassPropertyIterator::hasNext() const
	{
		return _index < this->_editor->getNbSegment();
	}

	void SegmentCollectionClassPropertyIterator::next()
	{
		_last = _index;
		++_index;
	}

	bool SegmentCollectionClassPropertyIterator::hasPrevious() const
	{
		return (_index > 0);
	}

	void SegmentCollectionClassPropertyIterator::previous()
	{
		--_index;
		_last = _index;
	}

	void SegmentCollectionClassPropertyIterator::toFront()
	{
		_index = 0;
		_last = -1;
	}

	void SegmentCollectionClassPropertyIterator::toBack()
	{
		_index = this->_editor->getNbSegment();
		_last = -1;
	}

	QScriptString SegmentCollectionClassPropertyIterator::name() const
	{
		return object().engine()->toStringHandle(QString::number(_last));
	}

	uint SegmentCollectionClassPropertyIterator::id() const
	{
		return _last;
	}
}
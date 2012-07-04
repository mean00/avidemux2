#include <QtScript/QScriptClassPropertyIterator>

#include "VideoFilter.h"
#include "VideoFilterCollection.h"
#include "VideoFilterCollectionPrototype.h"

extern BVector <ADM_VideoFilterElement> ADM_VideoFilters;

namespace ADM_qtScript
{
    class VideoFilterCollectionClassPropertyIterator : public QScriptClassPropertyIterator
    {
    public:
        VideoFilterCollectionClassPropertyIterator(const QScriptValue &object);
        ~VideoFilterCollectionClassPropertyIterator();

        bool hasNext() const;
        void next();

        bool hasPrevious() const;
        void previous();

        void toFront();
        void toBack();

        QScriptString name() const;
        uint id() const;

    private:
        unsigned int _index;
        unsigned int _last;
    };

    VideoFilterCollection::VideoFilterCollection(QScriptEngine *engine, IEditor *editor) :
        QObject(engine), QScriptClass(engine)
    {
        this->_editor = editor;
		this->_prototype = engine->newQObject(
							   new VideoFilterCollectionPrototype(this, editor), QScriptEngine::ScriptOwnership,
							   QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods |
							   QScriptEngine::ExcludeSuperClassProperties);
    }

	QString VideoFilterCollection::name() const
	{
		return QLatin1String("VideoFilterCollection");
	}

	QScriptClassPropertyIterator* VideoFilterCollection::newIterator(const QScriptValue &object)
	{
		return new VideoFilterCollectionClassPropertyIterator(object);
	}

	QScriptValue VideoFilterCollection::property(
		const QScriptValue &object,	const QScriptString &name, uint id)
	{
		if (id >= ADM_VideoFilters.size())
		{
			return QScriptValue();
		}

        return this->engine()->newQObject(
				   new VideoFilter(this->engine(), this->_editor, &(ADM_VideoFilters[id])), QScriptEngine::ScriptOwnership);
	}

	QScriptValue::PropertyFlags VideoFilterCollection::propertyFlags(
		const QScriptValue &object, const QScriptString &name, uint id)
	{
		return QScriptValue::Undeletable;
	}

	QScriptValue VideoFilterCollection::prototype() const
	{
		return this->_prototype;
	}

	QScriptClass::QueryFlags VideoFilterCollection::queryProperty(
		const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
	{
		bool isArrayIndex;
		quint32 pos = name.toArrayIndex(&isArrayIndex);

		if (!isArrayIndex)
		{
			return 0;
		}

		*id = pos;

		if (pos >= ADM_VideoFilters.size())
		{
			return 0;
		}

		return flags;
	}

    VideoFilterCollectionClassPropertyIterator::VideoFilterCollectionClassPropertyIterator(
        const QScriptValue &object) : QScriptClassPropertyIterator(object)
    {
        this->toFront();
    }

    VideoFilterCollectionClassPropertyIterator::~VideoFilterCollectionClassPropertyIterator() { }

    bool VideoFilterCollectionClassPropertyIterator::hasNext() const
    {
        return _index < ADM_VideoFilters.size();
    }

    void VideoFilterCollectionClassPropertyIterator::next()
    {
        _last = _index;
        ++_index;
    }

    bool VideoFilterCollectionClassPropertyIterator::hasPrevious() const
    {
        return (_index > 0);
    }

    void VideoFilterCollectionClassPropertyIterator::previous()
    {
        --_index;
        _last = _index;
    }

    void VideoFilterCollectionClassPropertyIterator::toFront()
    {
        _index = 0;
        _last = -1;
    }

    void VideoFilterCollectionClassPropertyIterator::toBack()
    {
        _index = ADM_VideoFilters.size();
        _last = -1;
    }

    QScriptString VideoFilterCollectionClassPropertyIterator::name() const
    {
        return object().engine()->toStringHandle(QString::number(_last));
    }

    uint VideoFilterCollectionClassPropertyIterator::id() const
    {
        return _last;
    }
}

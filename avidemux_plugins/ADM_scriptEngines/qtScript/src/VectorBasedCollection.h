#ifndef ADM_qtScript_VectorBasedCollection
#define ADM_qtScript_VectorBasedCollection

#include <vector>
#include <QtScript/QScriptClass>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptClassPropertyIterator>

#include "ADM_editor/include/IEditor.h"
#include "VectorBasedCollectionPrototype.h"

namespace ADM_qtScript
{
	template <class ItemType>
	class VectorBasedCollectionClassPropertyIterator : public QScriptClassPropertyIterator
	{
	private:
		const std::vector<ItemType*>& _items;
		unsigned int _index;
		unsigned int _last;

	public:
		VectorBasedCollectionClassPropertyIterator(const std::vector<ItemType*>& items, const QScriptValue &object) :
			QScriptClassPropertyIterator(object), _items(items)
		{
			this->toFront();
		}

		~VectorBasedCollectionClassPropertyIterator() {	}

		bool hasNext() const
		{
			return _index < this->_items.size();
		}

		void next()
		{
			_last = _index;
			_index++;
		}

		bool hasPrevious() const
		{
			return (_index > 0);
		}

		void previous()
		{
			_index--;
			_last = _index;
		}

		void toFront()
		{
			_index = 0;
			_last = -1;
		}

		void toBack()
		{
			_index = this->_items.size();
			_last = -1;
		}

		QScriptString name() const
		{
			return object().engine()->toStringHandle(QString::number(_last));
		}

		uint id() const
		{
			return _last;
		}
	};

	template <class ItemType>
	class VectorBasedCollection : public QScriptClass
	{
	private:
		IEditor *_editor;
		std::vector<ItemType*>& _items;
		QScriptValue _prototype;

	public:		
		VectorBasedCollection(QScriptEngine *engine, std::vector<ItemType*>& items, VectorBasedCollectionPrototype<ItemType>* prototype) :
		  QScriptClass(engine), _items(items)
		{
			this->_prototype = engine->newQObject(
								   prototype, QScriptEngine::ScriptOwnership,
								   QScriptEngine::SkipMethodsInEnumeration | QScriptEngine::ExcludeSuperClassMethods |
								   QScriptEngine::ExcludeSuperClassProperties);
		}

		QScriptClassPropertyIterator* newIterator(const QScriptValue &object)
		{
			return new VectorBasedCollectionClassPropertyIterator<ItemType>(this->_items, object);
		}

		QScriptValue property(
			const QScriptValue &object,	const QScriptString &name, uint id)
		{
			if (id >= this->_items.size())
			{
				return QScriptValue();
			}

			return this->engine()->newQObject(this->_items.at(id), QScriptEngine::ScriptOwnership);
		}

		QScriptValue::PropertyFlags propertyFlags(
			const QScriptValue &object, const QScriptString &name, uint id)
		{
			return QScriptValue::Undeletable;
		}

		QScriptValue prototype() const
		{
			return this->_prototype;
		}

		QScriptClass::QueryFlags queryProperty(
			const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id)
		{
			bool isArrayIndex;
			quint32 pos = name.toArrayIndex(&isArrayIndex);

			if (!isArrayIndex)
			{
				return 0;
			}

			*id = pos;

			if (pos >= this->_items.size())
			{
				return 0;
			}

			return flags;
		}
	};
}
#endif

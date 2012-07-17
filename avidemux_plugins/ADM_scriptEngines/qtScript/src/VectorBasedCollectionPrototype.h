#ifndef ADM_qtScript_VectorBasedCollectionPrototype
#define ADM_qtScript_VectorBasedCollectionPrototype

#include <vector>
#include <QtScript/QScriptable>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>

namespace ADM_qtScript
{
	template <class ItemType>
	class VectorBasedCollectionPrototype : public QObject, protected QScriptable
	{
	protected:
		std::vector<ItemType*>& _items;

		VectorBasedCollectionPrototype(std::vector<ItemType*>& items) : _items(items) { }
		
		QScriptValue add(QScriptValue item)
		{
			ItemType *itemObject = qobject_cast<ItemType*>(item.toQObject());

			if (itemObject == NULL)
			{
				return this->context()->throwError("Invalid object");
			}
			else
			{
				itemObject->setParent(this->parent());
				this->_items.push_back(itemObject);
			}

			return (uint)this->_items.size() - 1;
		}

		void clear()
		{
			this->_items.clear();
		}

		QScriptValue getLength()
		{
			return (uint)this->_items.size();
		}

		void insert(uint index, QScriptValue item)
		{
			ItemType* itemObject = qobject_cast<ItemType*>(item.toQObject());

			if (itemObject == NULL)
			{
				this->context()->throwError("Invalid object");
				return;
			}

			if (index > this->_items.size())
			{
				this->context()->throwError("Index is out of range");
				return;
			}

			itemObject->setParent(this->parent());
			this->_items.insert(this->_items.begin() + index, itemObject);
		}

		void removeAt(uint index)
		{
			if (index >= this->_items.size())
			{
				this->context()->throwError("Index is out of range");
				return;
			}

			this->_items.erase(this->_items.begin() + index);
		}
	};
}
#endif

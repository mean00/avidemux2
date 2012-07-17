#ifndef ADM_qtScript_AudioOutputCollectionPrototype
#define ADM_qtScript_AudioOutputCollectionPrototype

#include "ComboBoxItem.h"
#include "VectorBasedCollectionPrototype.h"

namespace ADM_qtScript
{
	/** \brief The ComboBoxItemCollection %class represents a collection of ComboBoxItem objects
	 * that can be individually accessed by index.
	 */
	class ComboBoxItemCollectionPrototype /*% ComboBoxItemCollection %*/ : public VectorBasedCollectionPrototype<ComboBoxItem>
	{
		Q_OBJECT

	public:
		/** \cond */
		ComboBoxItemCollectionPrototype(std::vector<ComboBoxItem*>& items) : VectorBasedCollectionPrototype<ComboBoxItem>(items) { }
		/** \endcond */

		/** \brief Returns the number of ComboBoxItem objects in the collection.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ length READ getLength());

		/** \brief Adds a ComboBoxItem object to the end of the combobox list.
		 * \return Returns the index of the newly added ComboBoxItem in the collection.
		 */
		Q_INVOKABLE QScriptValue /*% Number %*/ add(QScriptValue /*% ComboBoxItem %*/ item)
		{
			return VectorBasedCollectionPrototype<ComboBoxItem>::add(item);
		}

		/** \brief Adds an item with the given title and user data to the end of the combobox list.
		 * \return Returns the index of the newly added ComboBoxItem in the collection.
		 */
		Q_INVOKABLE QScriptValue /*% Number %*/ add(const QString& /*% String %*/ title, const QString& /*% String %*/ value)
		{ 
			ComboBoxItem *item = new ComboBoxItem(title, value);

			item->setParent(this->parent());
			this->_items.push_back(item);

			return (uint)this->_items.size() - 1;
		}

		/** \brief Removes all ComboBoxItem objects from the collection.
		 */
		Q_INVOKABLE void clear()
		{
			VectorBasedCollectionPrototype<ComboBoxItem>::clear();
		}

		/** \brief Inserts a new ComboBoxItem object into the collection at the specified index.
		 */
		Q_INVOKABLE void insert(uint /*% Number %*/ index, QScriptValue /*% ComboBoxItem %*/ item)
		{
			VectorBasedCollectionPrototype<ComboBoxItem>::insert(index, item);
		}

		/** \brief Inserts a new item with the given title and user data into the collection at the specified index.
		 */
		Q_INVOKABLE void insert(uint /*% Number %*/ index, const QString& /*% String %*/ title, const QString& /*% String %*/ value = "")
		{
			if (index > this->_items.size())
			{
				this->context()->throwError("Index is out of range");
				return;
			}

			ComboBoxItem *item = new ComboBoxItem(title, value);

			item->setParent(this->parent());
			this->_items.insert(this->_items.begin() + index, item);
		}

		/** \brief Removes the ComboBoxItem object at the specified index.
		 */
		Q_INVOKABLE void removeAt(uint /*% Number %*/ index)
		{
			VectorBasedCollectionPrototype<ComboBoxItem>::removeAt(index);
		}

		/** \brief Gets the ComboBoxItem object at the specified index.
		 */
		; /*% ComboBoxItem operator[](Number i); %*/
	};
}
#endif

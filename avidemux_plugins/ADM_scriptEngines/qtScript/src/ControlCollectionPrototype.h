#ifndef ADM_qtScript_AudioOutputCollectionPrototype
#define ADM_qtScript_AudioOutputCollectionPrototype

#include "Control.h"
#include "VectorBasedCollectionPrototype.h"

namespace ADM_qtScript
{
	/** \brief The ControlCollection %class represents a collection of Control objects
	 * that can be individually accessed by index.
	 */
	class ControlCollectionPrototype /*% ControlCollection %*/ : public VectorBasedCollectionPrototype<Control>
	{
		Q_OBJECT

	public:
		/** \cond */
		ControlCollectionPrototype(std::vector<Control*>& controls) : VectorBasedCollectionPrototype<Control>(controls) { }
		/** \endcond */

		/** \brief Returns the number of Control objects in the collection.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ length READ getLength());

		/** \brief Adds a Control object to the end of the collection.
		 * \return Returns the index of the newly added Control in the collection.
		 */
		Q_INVOKABLE QScriptValue /*% Number %*/ add(QScriptValue /*% Control %*/ item) { return VectorBasedCollectionPrototype<Control>::add(item); }

		/** \brief Removes all Control objects from the collection.
		 */
		Q_INVOKABLE void clear() { VectorBasedCollectionPrototype<Control>::clear(); }

		/** \brief Inserts a new Control object into the collection at the specified index.
		 */
		Q_INVOKABLE void insert(uint /*% Number %*/ index, QScriptValue /*% Control %*/ item) { VectorBasedCollectionPrototype<Control>::insert(index, item); }

		/** \brief Removes the Control object at the specified index.
		 */
		Q_INVOKABLE void removeAt(uint /*% Number %*/ index) { VectorBasedCollectionPrototype<Control>::removeAt(index); };

		/** \brief Gets the Control object at the specified index.
		 */
		; /*% Control operator[](Number i); %*/
	};
}
#endif

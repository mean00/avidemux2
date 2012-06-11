#ifndef ADM_qtScript_VideoFilterCollectionPrototype
#define ADM_qtScript_VideoFilterCollectionPrototype

#include "QtScriptObject.h"

namespace ADM_qtScript
{
    /** \brief The VideoFilterCollection %class represents a collection of VideoFilter objects
	 * that can be individually accessed by index.
	 */
    class VideoFilterCollectionPrototype /*% VideoFilterCollection %*/: public QtScriptObject
    {
        Q_OBJECT

    private:
        QScriptValue getLength();

    public:
        /** \cond */
        VideoFilterCollectionPrototype(QObject* parent, IEditor* editor);
        /** \endcond */

        /** \brief Returns the number of VideoFilter objects in the collection.
		 */
        Q_PROPERTY(QScriptValue /*% Number %*/ length READ getLength());

        /** \brief Adds a VideoFilter object to the end of the video filter chain.
		 * \return Returns the index of the newly added VideoFilter in the collection.
		 */
        Q_INVOKABLE QScriptValue /*% Number %*/ add(QScriptValue /*% VideoFilter %*/ filter);

        /** \brief Removes all VideoFilter objects from the collection.
		 */
        Q_INVOKABLE void clear();

		/** \brief Inserts a new video filter in to the collection at the specified index.
		 */
		Q_INVOKABLE QScriptValue /*% void %*/ insert(int /*% Number %*/ index, QScriptValue /*% VideoFilter %*/ filter);

        /** \brief Removes the VideoFilter object at the specified index.
		 */
        Q_INVOKABLE void removeAt(int /*% Number %*/ index);

        /** \brief Gets the VideoFilter object at the specified index.
		 */
		; /*% VideoFilter operator[](Number i); %*/
    };
}
#endif

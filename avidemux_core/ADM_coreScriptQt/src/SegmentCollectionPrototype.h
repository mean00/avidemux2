#ifndef ADM_qtScript_SegmentCollectionPrototype
#define ADM_qtScript_SegmentCollectionPrototype

#include "QtScriptObject.h"

namespace ADM_qtScript
{
	/** \brief The SegmentCollection %class represents a collection of Segment objects
	 * that can be individually accessed by index.
	 */
	class SegmentCollectionPrototype /*% SegmentCollection %*/: public QtScriptObject
	{
		Q_OBJECT

	private:
		IEditor* _editor;
		QScriptValue getLength();
		QScriptValue getTotalDuration(void);

	public:
		/** \cond */
		SegmentCollectionPrototype(QObject* parent, IEditor* editor);
		/** \endcond */

		/** \brief Returns the number of Segment objects in the collection.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ length READ getLength());

		/** \brief Returns the total time (in milliseconds) of all video segments in the collection.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ totalDuration READ getTotalDuration());

        /** \brief Appends a segment of video to the end of the currently open video.
		 *
		 * The segment starting at the specified time (in milliseconds) and for the specified duration (in milliseconds) is sourced from the first open video unless a video index is specified.
		 *
		 * \return Returns the index of the newly added Segment in the collection.
		 */
        Q_INVOKABLE QScriptValue /*% Number %*/ add(QScriptValue /*% Number %*/ startTime, QScriptValue /*% Number %*/ duration, QScriptValue /*% Number %*/ videoIndex = 0);

		/** \brief Removes all Segment objects from the collection.
		 */
		Q_INVOKABLE void clear();

		/** \brief Gets the Segment object at the specified index.
		 */
		; /*% Segment operator[](Number i); %*/
	};
}
#endif

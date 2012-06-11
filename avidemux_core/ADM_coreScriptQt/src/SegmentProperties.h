#ifndef ADM_qtScript_SegmentProperties
#define ADM_qtScript_SegmentProperties

#include "ADM_inttype.h"
#include "QtScriptObject.h"

namespace ADM_qtScript
{
	/** \brief The SegmentProperties %class holds the properties of a video segment.
	 */
	class SegmentProperties : public QtScriptObject
	{
		Q_OBJECT

	private:
		uint32_t _videoIndex;
		uint64_t _duration, _absoluteStartTime, _relativeStartTime;

		QScriptValue getAbsoluteStartTime(void);
		QScriptValue getDuration(void);
		QScriptValue getRelativeStartTime(void);
		QScriptValue getVideoIndex(void);

	public:
		/** \cond */
		SegmentProperties(IEditor *editor, _SEGMENT* segment);
		/** \endcond */

		/** \brief Returns the start time (in milliseconds) of the video segment in the editor.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ absoluteStartTime READ getAbsoluteStartTime);

		/** \brief Returns the duration (in milliseconds) of the video segment.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ duration READ getDuration);

		/** \brief Returns the start time (in milliseconds) of the video segment relative to the source video file.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ relativeStartTime READ getRelativeStartTime);

		/** \brief Returns a zero-based index of the video file from which the video segment is sourced.
		 *
		 * \sa Editor.videoProperties
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ videoIndex READ getVideoIndex);
	};
}
#endif

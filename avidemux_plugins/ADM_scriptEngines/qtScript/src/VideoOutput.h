#ifndef ADM_qtScript_VideoOutput
#define ADM_qtScript_VideoOutput

#include "QtScriptObject.h"
#include "ADM_coreVideoFilter.h"

namespace ADM_qtScript
{
    /** \brief The VideoOutput %class holds basic properties of a video.
	 */
    class VideoOutput : public QtScriptObject
    {
        Q_OBJECT

    private:
        unsigned int _width, _height;
        uint64_t _duration;

        QScriptValue getHeight();
        QScriptValue getDuration();
        QScriptValue getWidth();

    public:
        /** \cond */
        VideoOutput(IEditor *editor, FilterInfo *info);
        /** \endcond */

        /** \brief Returns the duration (in milliseconds) of the video.
		 */
        Q_PROPERTY(QScriptValue /*% Number %*/ duration READ getDuration);

		/** \brief Returns the height of the video in pixels.
		 */
        Q_PROPERTY(QScriptValue /*% Number %*/ height READ getHeight);

		/** \brief Returns the width of the video in pixels.
		 */
        Q_PROPERTY(QScriptValue /*% Number %*/ width READ getWidth);
    };
}

#endif

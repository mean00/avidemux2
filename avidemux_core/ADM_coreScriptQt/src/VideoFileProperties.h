#ifndef ADM_qtScript_VideoProperties
#define ADM_qtScript_VideoProperties

#include <vector>
#include <QtCore/QString>

#include "QtScriptObject.h"
#include "AudioProperties.h"

namespace ADM_qtScript
{
	/** \brief The VideoFileProperties %class holds the properties of a video file.
	 */
	class VideoFileProperties : public QtScriptObject
	{
		Q_OBJECT

	private:
		_VIDEOS *_video;

		std::vector<AudioProperties*> _audioProps;
		uint32_t _duration, _frameRate;
		uint32_t _height, _width;
		uint32_t _parHeight, _parWidth;
		QString _fourCC, _name;

		QScriptValue getAudioProperties();
		QScriptValue getAudioProperties(int audioIndex);
		QScriptValue getDuration(void);
		QScriptValue getFourCC(void);
		QScriptValue getFrameRate(void);
		QScriptValue getHeight(void);
		QScriptValue getName(void);
		QScriptValue getParHeight(void);
		QScriptValue getParWidth(void);
		QScriptValue getWidth(void);
		void initialiseAudioProperties();

	public:
		/** \cond */
		VideoFileProperties(IEditor* editor, _VIDEOS* video);
		~VideoFileProperties();
		/** \endcond */

		/** \brief Gets extended information about the video file's audio tracks.
		 *
		 * \return Returns an array of AudioProperties objects if a video is open in the editor and contains at least one audio track; otherwise, null.
		 */
		Q_PROPERTY(QScriptValue /*% AudioProperties[] %*/ audioProperties READ getAudioProperties);

		/** \brief Returns the duration (in milliseconds) of the video.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ duration READ getDuration);

		/** \brief Returns the four character code that identifies the video format.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ fourCC READ getFourCC);

		/** \brief Returns the video frame rate in frames per thousand seconds.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ frameRate READ getFrameRate);

		/** \brief Returns the height of the video in pixels.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ height READ getHeight);

		/** \brief Returns the name of the video file.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ name READ getName);

		/** \brief Returns the height term of the video's Pixel Aspect Ratio.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ parHeight READ getParHeight);

		/** \brief Returns the width term of the video's Pixel Aspect Ratio.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ parWidth READ getParWidth);

		/** \brief Returns the width of the video in pixels.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ width READ getWidth);
	};
}

#endif

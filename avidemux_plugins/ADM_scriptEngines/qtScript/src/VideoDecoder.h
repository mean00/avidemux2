#ifndef ADM_qtScript_VideoDecoder
#define ADM_qtScript_VideoDecoder

#include "QtScriptConfigObject.h"
#include "ADM_codec.h"

namespace ADM_qtScript
{
	/** \brief The VideoDecoder %class provides a basic interface for Avidemux video decoders.
	*/
	class VideoDecoder : public QtScriptConfigObject
	{
		Q_OBJECT

	private:
		_VIDEOS *_video;
		QString _videoFile;
		QScriptValue _configObject;

		void getConfCouple(CONFcouple **conf, const QString& containerName = QString());
		QScriptValue getConfiguration(void);
		QScriptValue getName(void);
		QScriptValue getVideoFileProperties(void);
		void setConfCouple(CONFcouple *conf, const QString& containerName = QString());
		bool verifyVideo();

	public:
		/** \cond */
		VideoDecoder(QScriptEngine *engine, IEditor* editor, _VIDEOS* video);
		/** \endcond */

		/** \brief Gets an object that holds parameters used to configure the video decoder.
		 *
		 * \sa resetConfiguration
		 */
		Q_PROPERTY(QScriptValue /*% Object %*/ configuration READ getConfiguration);

		/** \brief Gets the user friendly name of the video decoder.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ name READ getName);

		/** \brief Gets extended information about the video file being decoded.
		 *
		 * \sa Editor.videoFileProperties
		 */
		Q_PROPERTY(QScriptValue /*% VideoFileProperties %*/ videoFileProperties READ getVideoFileProperties);

		/** \brief Resets the video decoder back to its default configuration.
		*
		* \sa configuration
		*/
		Q_INVOKABLE void resetConfiguration(void);
	};
}
#endif

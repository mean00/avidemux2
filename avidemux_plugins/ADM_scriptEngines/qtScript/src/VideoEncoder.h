#ifndef ADM_qtScript_VideoEncoder
#define ADM_qtScript_VideoEncoder

#include "QtScriptConfigObject.h"
#include "ADM_coreVideoEncoderInternal.h"

namespace ADM_qtScript
{
	/** \brief The VideoEncoder %class is the base class of all Avidemux video encoders.
	 *
	 * Depending on the plugins that are packaged with Avidemux, the following child video encoder objects
	 * may be available to the ECMA scripting engine:
	 * \li CopyVideoEncoder
	 * \li Ffflv1VideoEncoder
	 * \li Ffmpeg2VideoEncoder
	 * \li Ffmpeg4VideoEncoder
	 * \li HuffyuvVideoEncoder
	 * \li MjpegVideoEncoder
	 * \li NullVideoEncoder
	 * \li PngVideoEncoder
	 * \li X264VideoEncoder
	 * \li Xvid4VideoEncoder
	 * \li Yv12encoderVideoEncoder
	 *
	 * Example usage:
	 * \code
	 * Xvid4VideoEncoder.resetConfiguration();
	 * Xvid4VideoEncoder.configuration.maxBFrame = 0;
	 * \endcode
	 */
	class VideoEncoder : public QtScriptConfigObject
	{
		Q_OBJECT

	private:
		QScriptValue _configObject;

		void getConfCouple(CONFcouple **conf, const QString& containerName = QString());
		QScriptValue getConfiguration(void);
		QScriptValue getName(void);
		void setConfCouple(CONFcouple *conf, const QString& containerName = QString());

	public:
		/** \cond */
		ADM_videoEncoder6 *encoderPlugin;

		VideoEncoder(QScriptEngine *engine, IEditor *editor, ADM_videoEncoder6* encoder);
		/** \endcond */

		/** \brief Gets an object that holds parameters used to configure the video encoder.
		 *
		 * \sa resetConfiguration
		 */
		Q_PROPERTY(QScriptValue /*% Object %*/ configuration READ getConfiguration);

		/** \brief Gets the user friendly name of the video encoder.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ name READ getName);

		/** \brief Resets the video encoder back to its default configuration.
		 *
		 * \sa configuration
		 */
		Q_INVOKABLE void resetConfiguration(void);
	};
}

#endif

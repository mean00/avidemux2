#ifndef ADM_qtScript_AudioEncoder
#define ADM_qtScript_AudioEncoder
#include "ADM_assert.h"
#include "QtScriptConfigObject.h"
#include "audioencoderInternal.h"

namespace ADM_qtScript
{
	/** \brief The AudioEncoder %class is the base class of all Avidemux audio encoders.
	 *
	 * Depending on the plugins that are packaged with Avidemux, the following audio encoder classes
	 * may be available to the ECMA scripting engine:
	 * \li AftenAudioEncoder
	 * \li CopyAudioEncoder
	 * \li FaacAudioEncoder
	 * \li LameAudioEncoder
	 * \li LavaacAudioEncoder
	 * \li Lavac3AudioEncoder
	 * \li Lavmp2AudioEncoder
	 * \li PcmAudioEncoder
	 * \li TwolameAudioEncoder
	 * \li VorbisAudioEncoder
	 *
	 * Example usage:
	 * \code
	 * var encoder = new LameAudioEncoder();
	 * encoder.configuration.bitrate = 160;
	 * \endcode
	 */
	class AudioEncoder : public QtScriptConfigObject
	{
		Q_OBJECT

	private:
		QScriptValue _configObject;
		CONFcouple* _encoderConf;
		EditableAudioTrack* _track;

		void getConfCouple(CONFcouple** conf, const QString& containerName = QString());
		QScriptValue getConfiguration(void);
		CONFcouple* getEncoderConfiguration();
		QScriptValue getName(void);
		void setConfCouple(CONFcouple* conf, const QString& containerName = QString());
		void setEncoderConfiguration(CONFcouple *couple);

	public:
		/** \cond */
		ADM_audioEncoder *encoderPlugin;
		int encoderIndex;

		AudioEncoder(
			QScriptEngine *engine, IEditor *editor, ADM_audioEncoder* encoder, int encoderIndex,
			EditableAudioTrack* track = NULL);
		~AudioEncoder();
		static QScriptValue constructor(QScriptContext *context, QScriptEngine *engine);
		bool isEncoderUsed();
		void useEncoderForAudioOutput(EditableAudioTrack* track);
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

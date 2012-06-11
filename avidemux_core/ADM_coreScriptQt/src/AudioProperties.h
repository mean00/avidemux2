#ifndef ADM_qtScript_AudioProperties
#define ADM_qtScript_AudioProperties

#include <QtCore/QString>
#include "QtScriptObject.h"

namespace ADM_qtScript
{
    /** \brief The AudioProperties %class holds the properties of an audio track.
     */
	class AudioProperties : public QtScriptObject
	{
		Q_OBJECT

		uint32_t _bitrate, _channels, _frequency;
		QString _codec;

		QScriptValue getBitrate(void);
		QScriptValue getChannels(void);
		QScriptValue getCodec(void);
		QScriptValue getFrequency(void);

	public:
		/** \cond */
		AudioProperties(IEditor* editor, ADM_audioStreamTrack* track);
		/** \endcond */

		/** \brief Returns the bitrate (in kilobits per second) of the audio track.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ bitrate READ getBitrate);

		/** \brief Returns the number of channels in the audio track.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ channels READ getChannels);

		/** \brief Returns the data format of the audio track.
		 */
		Q_PROPERTY(QScriptValue /*% String %*/ format READ getCodec);

		/** \brief Returns the sample rate in samples per second (hertz) of the audio track.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ sampleRate READ getFrequency);
	};
}

#endif

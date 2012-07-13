#ifndef ADM_qtScript_AudioOutput
#define ADM_qtScript_AudioOutput

#include "QtScriptObject.h"
#include "AudioEncoder.h"

namespace ADM_qtScript
{
    class AdmScriptMapper;

	/** \brief The AudioOutput %class provides an interface for configuring the output of an audio track.
	 */
	class AudioOutput : public QtScriptObject
	{
		Q_OBJECT
		Q_ENUMS(AudioInputType GainMode MixerMode StretchAudioMode)

	public:
		/** \brief Specifies the type of audio track that will be encoded
		 */
		enum AudioInputType
		{
			SourceVideo = 1, /**< An audio track from the currently open video will be encoded */
			ExternalFile = 2, /**< An external audio file will be encoded */
		};

		/** \brief Gain mode
		 */
		enum GainMode
		{
			NoGain = 0, /**< No gain */
			AutomaticGain = 1, /**< Automatic gain (maximum -3dB) */
			ManualGain = 2 /**< Manual gain */
		};

		/** \brief Specifies the audio mixing mode
		 */
		enum MixerMode
		{
			OriginalMix = 0, /**< No remixing of audio */
			MonoMix = 1, /**< Mono mix */
			StereoMix = 2, /**< Stereo mix */
			TwoFrontOneRearMix = 3, /**< Stereo front and rear-centre mix */
			TwoFrontTwoRearMix = 4, /**< Stereo front and stereo rear mix */
			ThreeFrontMix = 5, /**< Stereo front and centre mix */
			ThreeFrontOneRearMix = 6, /**< Stereo front, centre and rear-centre mix */
			ThreeFrontTwoRearMix = 7, /**< 5.0 channel mix */
			ThreeFrontTwoRearLfeMix = 8, /**< 5.1 channel mix */
			DolbyProLogicMix = 9, /**< Dolby Pro Logic mix */
			DolbyProLogicIIMix = 10 /**< Doly Pro Logic II mix */
		};

		/** \brief Specifies the mode for stretching audio
		 */
		enum StretchAudioMode
		{
			NoStretch = 0, /**< No audio stretching */
			FilmToPal = 1, /**< Shrinks audio from a 25fps video to 23.976fps */
			PalToFilm = 2 /**< Stretches audio from a 23.976fps video to 25fps */
		};

	private:
		EditableAudioTrack *_track;
		int _trackObjectId;
		AdmScriptMapper *_mapper;

		bool verifyTrack();

		QScriptValue getAudioInputFile();
		QScriptValue getAudioInputIndex();
		QScriptValue getAudioInputType();
		QScriptValue getEncoder();
		QScriptValue getGainMode();
		QScriptValue getGainValue();
		QScriptValue getMixer();
		QScriptValue getResample();
		QScriptValue getStretchAudioMode();
		QScriptValue getTimeShift();
		void setAudioInputFile(QScriptValue inputFile);
		void setAudioInputIndex(QScriptValue inputIndex);
		void setEncoder(QScriptValue encoder);
		void setGainMode(QScriptValue gainMode);
		void setGainValue(QScriptValue gainValue);
		void setMixer(QScriptValue mixer);
		void setResample(QScriptValue resample);
		void setStretchAudioMode(QScriptValue stretchAudioMode);
		void setTimeShift(QScriptValue timeShift);

	public:
		/** \cond */
		AudioOutput(IEditor *editor, EditableAudioTrack *track);
		~AudioOutput();
		/** \endcond */

		/** \brief Gets or sets the path of the external audio file if an external file is to be encoded.
		 * \sa audioInputType
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ audioInputFile READ getAudioInputFile WRITE setAudioInputFile);

		/** \brief Gets or sets the zero-based index of an audio track in the currently opened video
		 * if an internal track is to be encoded.
		 * \sa audioInputType
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ audioInputIndex READ getAudioInputIndex WRITE setAudioInputIndex);

		/** \brief Returns the type of audio track that will be encoded.
		 * \sa audioInputFile and audioInputIndex
		 */
		Q_PROPERTY(QScriptValue /*% AudioInputType %*/ audioInputType READ getAudioInputType);

		/** \brief Gets or sets the encoder that will be used on the input audio.
		 */
		Q_PROPERTY(QScriptValue /*% AudioEncoder %*/ encoder READ getEncoder WRITE setEncoder);

		/** \brief Gets or sets the gain mode of the Gain audio filter.
		 * \sa gainValue
		 */
		Q_PROPERTY(QScriptValue /*% GainMode %*/ gainMode READ getGainMode WRITE setGainMode);

		/** \brief Gets or sets the level of gain adjustment (in decibels) that will be applied to the input
		 * audio during encoding.
		 *
		 * The Gain audio filter amplifies (or attenuates) the sound level of the input audio. Setting this property
		 * will automatically set AudioOutput.gainMode to AudioOutput::ManualGain.
		 *
		 * \sa gainMode
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ gainValue READ getGainValue WRITE setGainValue);

		/** \brief Gets or sets the mixer that will be applied to the input audio during encoding.
		 */
		Q_PROPERTY(QScriptValue /*% MixerMode %*/ mixer READ getMixer WRITE setMixer);

		/** \brief Gets or sets the sampling rate (in hertz) for resampling the input audio.
		 *
		 * A value of zero indicates that resampling is disabled.
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ samplingRate READ getResample WRITE setResample);

		/** \brief Shortens or extends the encoded audio relative to the source audio.
		 */
		Q_PROPERTY(QScriptValue /*% StretchAudioMode %*/ stretchAudioMode READ getStretchAudioMode WRITE setStretchAudioMode);

		/** \brief Shifts (in milliseconds) the input audio forward (positive number) or backward
		 * (negative number).
		 */
		Q_PROPERTY(QScriptValue /*% Number %*/ timeShift READ getTimeShift WRITE setTimeShift);
	};
}

#endif

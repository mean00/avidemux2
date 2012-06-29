#ifndef AdmToScriptMapper_h
#define AdmToScriptMapper_h

#include "ADM_inttype.h"
#include "audiofilter_normalize_param.h"
#include "AudioOutput.h"

namespace ADM_qtScript
{
    class AdmScriptMapper
    {
    private:
        static const ADM_GAINMode _admGainModes[];
        static const AudioOutput::GainMode _scriptGainModes[];

        static const CHANNEL_CONF _admMixerModes[];
        static const AudioOutput::MixerMode _scriptMixerModes[];

        static const FILMCONV _admStretchAudioModes[];
        static const AudioOutput::StretchAudioMode _scriptStretchAudioModes[];

        template<typename TArray, size_t size>
        size_t getArrayLength(TArray(&)[size]);

        QString getClassName(const char* encoderName, const QString& classSuffix);

        template<typename TSourceValueType, typename TMappedValueType>
        TMappedValueType mapValue(
            const TSourceValueType sourceValue, const TSourceValueType sourceValues[], size_t sourceValueCount, const TMappedValueType mappedValues[]);

        template<typename TSourceValueType>
        QString toScriptValueName(TSourceValueType sourceValue, QMetaObject mapMetaObject, const char* enumName);

    public:
        QString getAudioEncoderClassName(const char* encoderName);
        QString getMuxerClassName(const char* encoderName);
        QString getVideoEncoderClassName(const char* encoderName);
        QString getVideoFilterClassName(const char* encoderName);

        ADM_GAINMode toAdmValue(AudioOutput::GainMode gainMode);
        CHANNEL_CONF toAdmValue(AudioOutput::MixerMode mixerMode);
        FILMCONV toAdmValue(AudioOutput::StretchAudioMode stretchAudioMode);

        AudioOutput::GainMode toScriptValue(ADM_GAINMode gainMode);
        AudioOutput::MixerMode toScriptValue(CHANNEL_CONF mixerMode);
        AudioOutput::StretchAudioMode toScriptValue(FILMCONV mixerMode);

        QString toScriptValueName(ADM_GAINMode gainMode);
        QString toScriptValueName(CHANNEL_CONF mixerMode);
        QString toScriptValueName(FILMCONV stretchAudioMode);
    };
}
#endif

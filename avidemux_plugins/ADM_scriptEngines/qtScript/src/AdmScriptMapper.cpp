#include <QtCore/QMetaEnum>

#include "AdmScriptMapper.h"

namespace ADM_qtScript
{
    const ADM_GAINMode AdmScriptMapper::_admGainModes[] = { ADM_NO_GAIN, ADM_GAIN_MANUAL, ADM_GAIN_AUTOMATIC };
    const AudioOutput::GainMode AdmScriptMapper::_scriptGainModes[] = { AudioOutput::NoGain, AudioOutput::ManualGain, AudioOutput::AutomaticGain };

    const CHANNEL_CONF AdmScriptMapper::_admMixerModes[] =
    {
        CHANNEL_INVALID, CHANNEL_MONO, CHANNEL_STEREO, CHANNEL_2F_1R, CHANNEL_3F, CHANNEL_3F_1R, CHANNEL_2F_2R, CHANNEL_3F_2R, CHANNEL_3F_2R_LFE,
        CHANNEL_DOLBY_PROLOGIC, CHANNEL_DOLBY_PROLOGIC2
    };

    const AudioOutput::MixerMode AdmScriptMapper::_scriptMixerModes[] =
    {
        AudioOutput::OriginalMix, AudioOutput::MonoMix, AudioOutput::StereoMix, AudioOutput::TwoFrontOneRearMix, AudioOutput::ThreeFrontMix,
        AudioOutput::ThreeFrontOneRearMix, AudioOutput::TwoFrontTwoRearMix, AudioOutput::ThreeFrontTwoRearMix, AudioOutput::ThreeFrontTwoRearLfeMix,
        AudioOutput::DolbyProLogicMix, AudioOutput::DolbyProLogicIIMix
    };

    const FILMCONV AdmScriptMapper::_admStretchAudioModes[] = { FILMCONV_NONE, FILMCONV_FILM2PAL, FILMCONV_PAL2FILM };

    const AudioOutput::StretchAudioMode AdmScriptMapper::_scriptStretchAudioModes[] =
    {
        AudioOutput::NoStretch, AudioOutput::FilmToPal, AudioOutput::PalToFilm
    };

    QString AdmScriptMapper::getAudioEncoderClassName(const char* encoderName)
    {
        return this->getClassName(encoderName, "AudioEncoder");
    }

    QString AdmScriptMapper::getMuxerClassName(const char* encoderName)
    {
        return this->getClassName(encoderName, "Muxer");
    }

    QString AdmScriptMapper::getVideoEncoderClassName(const char* encoderName)
    {
        return this->getClassName(encoderName, "VideoEncoder");
    }

    QString AdmScriptMapper::getVideoFilterClassName(const char* encoderName)
    {
        return this->getClassName(encoderName, "VideoFilter");
    }

    ADM_GAINMode AdmScriptMapper::toAdmValue(AudioOutput::GainMode gainMode)
    {
        return this->mapValue(gainMode, _scriptGainModes, getArrayLength(_scriptGainModes), _admGainModes);
    }

    CHANNEL_CONF AdmScriptMapper::toAdmValue(AudioOutput::MixerMode mixerMode)
    {
        return this->mapValue(mixerMode, _scriptMixerModes, getArrayLength(_scriptMixerModes), _admMixerModes);
    }

    FILMCONV AdmScriptMapper::toAdmValue(AudioOutput::StretchAudioMode stretchAudioMode)
    {
        return this->mapValue(stretchAudioMode, _scriptStretchAudioModes, getArrayLength(_scriptStretchAudioModes), _admStretchAudioModes);
    }

    AudioOutput::GainMode AdmScriptMapper::toScriptValue(ADM_GAINMode gainMode)
    {
        return this->mapValue(gainMode, _admGainModes, getArrayLength(_admGainModes), _scriptGainModes);
    }

    AudioOutput::MixerMode AdmScriptMapper::toScriptValue(CHANNEL_CONF mixerMode)
    {
        return this->mapValue(mixerMode, _admMixerModes, getArrayLength(_admMixerModes), _scriptMixerModes);
    }

    AudioOutput::StretchAudioMode AdmScriptMapper::toScriptValue(FILMCONV stretchAudioMode)
    {
        return this->mapValue(stretchAudioMode, _admStretchAudioModes, getArrayLength(_admStretchAudioModes), _scriptStretchAudioModes);
    }

    QString AdmScriptMapper::toScriptValueName(ADM_GAINMode gainMode)
    {
        return this->toScriptValueName(gainMode, AudioOutput::staticMetaObject, "GainMode");
    }

    QString AdmScriptMapper::toScriptValueName(CHANNEL_CONF mixerMode)
    {
        return this->toScriptValueName(mixerMode, AudioOutput::staticMetaObject, "MixerMode");
    }

    QString AdmScriptMapper::toScriptValueName(FILMCONV stretchAudioMode)
    {
        return this->toScriptValueName(stretchAudioMode, AudioOutput::staticMetaObject, "StretchAudioMode");
    }

    template<typename TArray, size_t size>
    size_t AdmScriptMapper::getArrayLength(TArray(&)[size])
    {
        return size;
    }

    QString AdmScriptMapper::getClassName(const char* encoderName, const QString& classSuffix)
    {
        QString className = QString(encoderName).toLower() + classSuffix;

        return className[0].toUpper() + className.mid(1);
    }

    template<typename TSourceValueType, typename TMappedValueType>
    TMappedValueType AdmScriptMapper::mapValue(
        const TSourceValueType sourceValue, const TSourceValueType sourceValues[], size_t sourceValueCount, const TMappedValueType mappedValues[])
    {
        TMappedValueType mappedValue = mappedValues[0];

        for (size_t index = 0; index < sourceValueCount; index++)
        {
            if (sourceValue == sourceValues[index])
            {
                mappedValue = mappedValues[index];
                break;
            }
        }

        return mappedValue;
    }

    template<typename TSourceValueType>
    QString AdmScriptMapper::toScriptValueName(TSourceValueType sourceValue, QMetaObject mapMetaObject, const char* mapEnumName)
    {
        int metaIndex = mapMetaObject.indexOfEnumerator(mapEnumName);

        ADM_assert(metaIndex != -1);

        QMetaEnum metaEnum = mapMetaObject.enumerator(metaIndex);
        QString className = mapMetaObject.className();

        return className.mid(className.indexOf("::") + 2) + "." + QString(mapEnumName) + "." + QString(metaEnum.valueToKey(this->toScriptValue(sourceValue)));
    }
}

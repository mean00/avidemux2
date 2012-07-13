#include "AdmScriptMapper.h"
#include "AudioOutput.h"
#include "ADM_editor/include/ADM_edAudioTrack.h"
#include "ADM_editor/include/ADM_edAudioTrackExternal.h"

extern BVector <ADM_audioEncoder *> ListOfAudioEncoder;

namespace ADM_qtScript
{
    AudioOutput::AudioOutput(IEditor* editor, EditableAudioTrack *track) : QtScriptObject(editor)
    {
        this->_track = track;
        this->_trackObjectId = track->objectId;
        this->_mapper = new AdmScriptMapper();
    }

    AudioOutput::~AudioOutput()
    {
        delete this->_mapper;
    }

    QScriptValue AudioOutput::getAudioInputFile()
    {
        QScriptValue inputType = this->getAudioInputType();

        if (inputType.isUndefined() || inputType.toNumber() != AudioOutput::ExternalFile)
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
        else
        {
            return this->_track->edTrack->castToExternal()->getMyName().c_str();
        }
    }

    QScriptValue AudioOutput::getAudioInputIndex()
    {
        QScriptValue inputType = this->getAudioInputType();

        if (inputType.isUndefined() || inputType.toNumber() != AudioOutput::SourceVideo)
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
        else
        {
            return this->_track->poolIndex;
        }
    }

    QScriptValue AudioOutput::getAudioInputType()
    {
        if (this->verifyTrack())
        {
            if (this->_track->edTrack->getTrackType() == ADM_EDAUDIO_EXTERNAL)
            {
                return AudioOutput::ExternalFile;
            }
            else
            {
                return AudioOutput::SourceVideo;
            }
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getEncoder()
    {
        if (this->verifyTrack())
        {
            return this->engine()->newQObject(
                       new AudioEncoder(
                           this->engine(), this->_editor, ListOfAudioEncoder[this->_track->encoderIndex],
                           this->_track->encoderIndex, this->_track), QScriptEngine::ScriptOwnership);
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getGainMode()
    {
        if (this->verifyTrack())
        {
            return this->_mapper->toScriptValue(this->_track->audioEncodingConfig.gainParam.mode);
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getGainValue()
    {
        if (this->verifyTrack())
        {
            if (this->_track->audioEncodingConfig.gainParam.mode == ADM_GAIN_MANUAL)
            {
                return ((double)this->_track->audioEncodingConfig.gainParam.gain10) / 10;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getMixer()
    {
        if (this->verifyTrack())
        {
            return _mapper->toScriptValue(this->_track->audioEncodingConfig.mixerConf);
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getResample()
    {
        if (this->verifyTrack())
        {
            if (this->_track->audioEncodingConfig.resamplerEnabled)
            {
                return this->_track->audioEncodingConfig.resamplerFrequency;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getStretchAudioMode()
    {
        if (this->verifyTrack())
        {
            return _mapper->toScriptValue(this->_track->audioEncodingConfig.film2pal);
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    QScriptValue AudioOutput::getTimeShift()
    {
        if (this->verifyTrack())
        {
            return this->_track->audioEncodingConfig.shiftInMs;
        }
        else
        {
            return QScriptValue(QScriptValue::UndefinedValue);
        }
    }

    void AudioOutput::setAudioInputFile(QScriptValue inputFile)
    {
        if (this->verifyTrack())
        {
            PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();
            const char* constInputFile = inputFile.toString().toUtf8().constData();

            for (int trackIndex = 0; trackIndex < audioTracks->size(); trackIndex++)
            {
                ADM_edAudioTrack* track = audioTracks->at(trackIndex);

                if (track->getTrackType() == ADM_EDAUDIO_EXTERNAL &&
                        track->castToExternal()->getMyName().compare(constInputFile))
                {
                    this->_track->edTrack = track;
                    return;
                }
            }

            if (this->_editor->addExternalAudioTrack(constInputFile))
            {
                this->_track->edTrack = audioTracks->at(audioTracks->size() - 1);
                this->_track->poolIndex = audioTracks->size() - 1;
            }
            else
            {
                this->throwError("Unable to add external audio file.");
            }
        }
    }

    void AudioOutput::setAudioInputIndex(QScriptValue inputIndex)
    {
        if (this->verifyTrack())
        {
            PoolOfAudioTracks* audioTracks = this->_editor->getPoolOfAudioTrack();

            if (audioTracks->size() == 0)
            {
                this->throwError(
                    "The source video doesn't contain a valid audio track to use for encoding.");
                return;
            }

            QScriptValue result = this->validateNumber(
                                      "inputIndex", inputIndex, 0, audioTracks->size() - 1);

            if (!result.isUndefined())
            {
                return;
            }

            this->_track->edTrack = audioTracks->at(inputIndex.toNumber());
            this->_track->poolIndex = inputIndex.toNumber();
        }
    }

    void AudioOutput::setEncoder(QScriptValue encoder)
    {
        if (this->verifyTrack())
        {
            AudioEncoder *encoderObject = qobject_cast<AudioEncoder*>(encoder.toQObject());

            if (encoderObject != NULL)
            {
                if (encoderObject->isEncoderUsed())
                {
                    this->throwError("Audio encoder is already being used by another audio output.");
                }
                else
                {
                    encoderObject->useEncoderForAudioOutput(this->_track);
                    this->_editor->updateDefaultAudioTrack();
                }
            }
        }
    }

    void AudioOutput::setGainMode(QScriptValue gainMode)
    {
        QScriptValue validateResult = this->validateNumber("gainMode", gainMode);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            this->_track->audioEncodingConfig.gainParam.mode = _mapper->toAdmValue((GainMode)gainMode.toNumber());
        }
    }

    void AudioOutput::setGainValue(QScriptValue gainValue)
    {
        QScriptValue validateResult = this->validateNumber("gainValue", gainValue, -10, 40);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            qsreal gain = gainValue.toNumber();

            if (gain == 0.0)
            {
                this->_track->audioEncodingConfig.gainParam.mode = ADM_NO_GAIN;
            }
            else
            {
                this->_track->audioEncodingConfig.gainParam.mode = ADM_GAIN_MANUAL;
            }

            this->_track->audioEncodingConfig.gainParam.gain10 = (int)(gain * 10);
        }
    }

    void AudioOutput::setMixer(QScriptValue mixer)
    {
        QScriptValue validateResult = this->validateNumber("mixer", mixer);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            this->_track->audioEncodingConfig.mixerConf = _mapper->toAdmValue((MixerMode)mixer.toNumber());
            this->_track->audioEncodingConfig.mixerEnabled = (this->_track->audioEncodingConfig.mixerConf != CHANNEL_INVALID);
        }
    }

    void AudioOutput::setResample(QScriptValue resample)
    {
        QScriptValue validateResult = this->validateNumber("resample", resample);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            int hz = resample.toNumber();

            if (hz == 0)
            {
                this->_track->audioEncodingConfig.resamplerEnabled = 0;
            }
            else
            {
                QScriptValue validateResult = this->validateNumber("resample", resample, 6000, 64000);

                if (validateResult.isUndefined())
                {
                    this->_track->audioEncodingConfig.resamplerEnabled = 1;
                    this->_track->audioEncodingConfig.resamplerFrequency = hz;
                }
            }
        }
    }

    void AudioOutput::setStretchAudioMode(QScriptValue stretchAudioMode)
    {
        QScriptValue validateResult = this->validateNumber("stretchAudioMode", stretchAudioMode);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            this->_track->audioEncodingConfig.film2pal = _mapper->toAdmValue((StretchAudioMode)stretchAudioMode.toNumber());
        }
    }

    void AudioOutput::setTimeShift(QScriptValue timeShift)
    {
        QScriptValue validateResult = this->validateNumber("timeShift", timeShift, -99999, 99999);

        if (this->verifyTrack() && validateResult.isUndefined())
        {
            this->_track->audioEncodingConfig.shiftInMs = timeShift.toNumber();
        }
    }

    bool AudioOutput::verifyTrack()
    {
        ActiveAudioTracks* tracks = this->_editor->getPoolOfActiveAudioTrack();
        bool found = false;

        for (unsigned int trackIndex = 0; trackIndex < tracks->size(); trackIndex++)
        {
            EditableAudioTrack *track = tracks->atEditable(trackIndex);

            if (this->_track == track && this->_trackObjectId == track->objectId)
            {
                found = true;
                break;
            }
        }

        return found;
    }
}

#include "QtScriptWriter.h"
#include "ADM_editor/include/ADM_edEditableAudioTrack.h"
#include "ADM_editor/include/ADM_edAudioTrackExternal.h"
#include "audioencoderInternal.h"
#include "VideoFilterShim.h"

extern void getCoupleFromString(CONFcouple **couples, const char *str, const ADM_paramList *tmpl);
extern const ADM_paramList FFcodecContext_param[];

namespace ADM_qtScript
{
	QtScriptWriter::QtScriptWriter()
	{
		this->_stream = NULL;
	}

    void QtScriptWriter::addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track)
    {
        *(this->_stream) << std::endl << "audioOutput = new " << _mapper.getAudioEncoderClassName(encoder->codecName).toUtf8().constData()
                         << "();" << std::endl;

		CONFcouple *defaultConfiguration = NULL;

		if (encoder->getDefaultConfiguration)
		{
			encoder->getDefaultConfiguration(&defaultConfiguration);
		}

        this->dumpConfCoupleDiff("audioOutput.configuration.", defaultConfiguration, track->encoderConf);
		delete defaultConfiguration;

		*(this->_stream) << "Editor.audioOutputs.add(";

		if (track->edTrack->getTrackType() == ADM_EDAUDIO_EXTERNAL)
		{
			*(this->_stream) << "\"" << track->edTrack->castToExternal()->getMyName() << "\"";
		}
		else
		{
			*(this->_stream) << track->poolIndex;
		}

		*(this->_stream) << ", audioOutput);" << std::endl;
    }

    void QtScriptWriter::addSegment(uint32_t videoIndex, uint64_t startTime, uint64_t duration)
    {
        *(this->_stream) << "Editor.segments.add(" << startTime << ", " << duration << ", " << videoIndex << ");" << std::endl;
    }

    void QtScriptWriter::addVideoFilter(ADM_vf_plugin *plugin, ADM_VideoFilterElement *element)
    {
        *(this->_stream) << std::endl << "videoFilter = new " << _mapper.getVideoFilterClassName(plugin->getInternalName()).toUtf8().constData()
                         << "();" << std::endl;

		CONFcouple *configuration, *defaultConfiguration;
		ADM_coreVideoFilter *filter = plugin->create(new VideoFilterShim(), NULL);

        filter->getCoupledConf(&defaultConfiguration);
		element->instance->getCoupledConf(&configuration);

        this->dumpConfCoupleDiff("videoFilter.configuration.", defaultConfiguration, configuration);

		delete defaultConfiguration;
		delete configuration;

        *(this->_stream) << "Editor.appliedVideoFilters.add(videoFilter);" << std::endl << std::endl;
    }

    void QtScriptWriter::appendVideo(const char* path)
    {
        *(this->_stream) << "Editor.appendVideo(\"" << path << "\");" << std::endl;
    }

    void QtScriptWriter::clearAudioTracks()
    {
        *(this->_stream) << "Editor.audioOutputs.clear();" << std::endl;
    }

    void QtScriptWriter::clearSegments()
    {
        *(this->_stream) << "Editor.segments.clear()" << std::endl;
    }

    void QtScriptWriter::closeVideo()
    {
        *(this->_stream) << "Editor.closeVideo();" << std::endl;
    }

    void QtScriptWriter::connectStream(std::iostream& stream)
    {
        this->_stream = &stream;
    }

    void QtScriptWriter::disconnectStream()
    {
        this->_stream = NULL;
    }

    void QtScriptWriter::loadVideo(const char* path)
    {
        *(this->_stream) << "Editor.openVideo(\"" << path << "\");" << std::endl;
    }

    void QtScriptWriter::setAudioGain(int trackIndex, ADM_GAINMode gainMode, uint32_t gainValue)
    {
        *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].gainMode = " << _mapper.toScriptValueName(gainMode).toUtf8().constData() << ";" << std::endl;

        if (gainMode == ADM_GAIN_MANUAL)
        {
            *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].gainValue = " << ((double)gainValue) / 10 << ";" << std::endl;
        }
    }
    void QtScriptWriter::setAudioDrc(int trackIndex, bool active)
    {
            *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].drc = " << (int)active << ";" << std::endl;
    }
    void QtScriptWriter::setAudioMixer(int trackIndex, CHANNEL_CONF mixer)
    {
        *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].mixer = " << _mapper.toScriptValueName(mixer).toUtf8().constData() << ";" << std::endl;
    }

    void QtScriptWriter::setAudioResample(int trackIndex, uint32_t resample)
    {
        *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].samplingRate = " << resample << ";" << std::endl;
    }

    void QtScriptWriter::setMarkers(uint64_t markerA, uint64_t markerB)
    {
        *(this->_stream) << "Editor.setMarkers(" << markerA << ", " << markerB << ");" << std::endl;
    }

    void QtScriptWriter::setMuxer(ADM_dynMuxer *muxer)
    {
        QString muxerClassName = _mapper.getMuxerClassName(muxer->name);
		CONFcouple *configuration, *defaultConfiguration;

        *(this->_stream) << std::endl;

		muxer->getConfiguration(&configuration);
		muxer->resetConfiguration();
		muxer->getConfiguration(&defaultConfiguration);
		muxer->setConfiguration(configuration);

        this->dumpConfCoupleDiff((muxerClassName + ".configuration.").toUtf8().constData(), defaultConfiguration, configuration);

		delete configuration;
		delete defaultConfiguration;

        *(this->_stream) << "Editor.currentMuxer = " << muxerClassName.toUtf8().constData() << ";" << std::endl;
    }

    void QtScriptWriter::setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv)
    {

    }

    void QtScriptWriter::setVideoEncoder(ADM_videoEncoder6* videoEncoder)
    {
        QString encoderClassName = _mapper.getVideoEncoderClassName(videoEncoder->desc->encoderName);

        *(this->_stream) << std::endl;

		if (videoEncoder->desc->getConfigurationData)
		{
			CONFcouple *configuration, *defaultConfiguration;

			videoEncoder->desc->getConfigurationData(&configuration);
			videoEncoder->desc->resetConfigurationData();
			videoEncoder->desc->getConfigurationData(&defaultConfiguration);
			videoEncoder->desc->setConfigurationData(configuration, true);

			this->dumpConfCoupleDiff((encoderClassName + ".configuration.").toUtf8().constData(), defaultConfiguration, configuration);

			delete configuration;
			delete defaultConfiguration;
		}

        *(this->_stream) << "Editor.currentVideoEncoder = " << encoderClassName.toUtf8().constData() << ";" << std::endl;
    }

    void QtScriptWriter::stretchAudio(int trackIndex, FILMCONV fps)
    {
        *(this->_stream) << "Editor.audioOutputs[" << trackIndex << "].stretchAudioMode = " << _mapper.toScriptValueName(fps).toUtf8().constData() << ";" << std::endl;
    }

	void QtScriptWriter::dumpConfCoupleDiff(const QString& prefix, CONFcouple *oldConf, CONFcouple *newConf)
	{
		if (!newConf)
		{
			return;
		}

		for (unsigned int index = 0; index < newConf->getSize(); index++)
		{
			char *name, *newValue;
			int oldIndex = -1;

			newConf->getInternalName(index, &name, &newValue);

			if (oldConf)
			{
				oldIndex = oldConf->lookupName(name);
			}

			if (oldIndex > -1)
			{
				char *oldValue;

				oldConf->getInternalName(oldIndex, &name, &oldValue);

				if (strcmp(oldValue, newValue) != 0)
				{
					if (strcmp(name, "lavcSettings") == 0)
					{
						CONFcouple *oldLavcConf;
						CONFcouple *newLavcConf;

						getCoupleFromString(&oldLavcConf, oldValue, FFcodecContext_param);
						getCoupleFromString(&newLavcConf, newValue, FFcodecContext_param);

						this->dumpConfCoupleDiff(prefix + "lavcSettings.", oldLavcConf, newLavcConf);

						delete oldLavcConf;
						delete newLavcConf;
					}
					else
					{
						*(this->_stream) << prefix.toUtf8().constData() << QString(name).replace('.', '_').toUtf8().constData() << " = \"" << newValue << "\";" << std::endl;
					}
				}
			}
			else
			{
				*(this->_stream) << prefix.toUtf8().constData() << QString(name).replace('.', '_').toUtf8().constData() << " = \"" << newValue << "\";" << std::endl;
			}
		}
	}
}

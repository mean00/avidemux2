#include "SpiderMonkeyScriptWriter.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerInternal.h"
#include "ADM_audiocodec/ADM_audiocodec.h"
#include "ADM_editor/include/ADM_edEditableAudioTrack.h"

SpiderMonkeyScriptWriter::SpiderMonkeyScriptWriter()
{
	this->_stream = NULL;
}

void SpiderMonkeyScriptWriter::addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track)
{
    *(this->_stream) << "adm.audioCodec(" << trackIndex << ", \"" << encoder->codecName << "\"";
    this->dumpConfCouple(track->encoderConf);
    *(this->_stream) << ");" << std::endl;
}

void SpiderMonkeyScriptWriter::addSegment(uint32_t videoIndex, uint64_t startTime, uint64_t duration)
{
    *(this->_stream) << "adm.addSegment(" << videoIndex << ", " << startTime << ", " << duration << ");" << std::endl;
}

void SpiderMonkeyScriptWriter::addVideoFilter(ADM_vf_plugin *plugin, ADM_VideoFilterElement *element)
{
    *(this->_stream) << "adm.addVideoFilter(\"" << plugin->getInternalName() << "\"";

	CONFcouple *configuration;

	element->instance->getCoupledConf(&configuration);
    this->dumpConfCouple(configuration);
	delete configuration;

    *(this->_stream) << ");" << std::endl;
}

void SpiderMonkeyScriptWriter::appendVideo(const char* path)
{
    *(this->_stream) << "adm.appendVideo(\"" << path << "\");" << std::endl;
}
void SpiderMonkeyScriptWriter::setAudioDrc(int trackIndex, bool active)
{

}
void SpiderMonkeyScriptWriter::clearAudioTracks()
{

}

void SpiderMonkeyScriptWriter::clearSegments()
{
    *(this->_stream) << "adm.clearSegments();" << std::endl;
}

void SpiderMonkeyScriptWriter::closeVideo()
{
    
}

void SpiderMonkeyScriptWriter::connectStream(std::iostream& stream)
{
    this->_stream = &stream;

    *(this->_stream) << "//AD  <- Needed to identify //" << std::endl;
    *(this->_stream) << "//--automatically built--\n" << std::endl << std::endl;
}

void SpiderMonkeyScriptWriter::disconnectStream()
{
    this->_stream = NULL;
}

void SpiderMonkeyScriptWriter::loadVideo(const char* path)
{
    *(this->_stream) << "adm.loadVideo(\"" << path << "\");" << std::endl;
}

void SpiderMonkeyScriptWriter::setAudioGain(int trackIndex, ADM_GAINMode gainMode, uint32_t gainValue)
{

}

void SpiderMonkeyScriptWriter::setAudioMixer(int trackIndex, CHANNEL_CONF mixer)
{
    const char *mixerString = NULL;

    for (unsigned int i = 0; i < NB_MIXER_DESC; i++)
    {
        if (mixer == mixerStringDescriptor[i].conf)
        {
            mixerString = mixerStringDescriptor[i].desc;
        }
    }

    *(this->_stream) << "adm.audioMuxer(" << trackIndex << ", \"" << mixerString << "\");" << std::endl;
}

void SpiderMonkeyScriptWriter::setAudioResample(int trackIndex, uint32_t resample)
{

}

void SpiderMonkeyScriptWriter::setMarkers(uint64_t markerA, uint64_t markerB)
{
    *(this->_stream) << "adm.markerA = " << markerA << ";" << std::endl;
    *(this->_stream) << "adm.markerB = " << markerB << ";" << std::endl;
}

void SpiderMonkeyScriptWriter::setMuxer(ADM_dynMuxer *muxer)
{
	CONFcouple *configuration;

	muxer->getConfiguration(&configuration);

	*(this->_stream) << "adm.setContainer(\"" << muxer->name << "\"";
    this->dumpConfCouple(configuration);
    *(this->_stream) << ");" << std::endl;

	delete configuration;
}

void SpiderMonkeyScriptWriter::setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv)
{
    *(this->_stream) << "adm.setPostProc(" << type << ", " << strength << ", " << swapUv << ");" << std::endl;
}

void SpiderMonkeyScriptWriter::setVideoEncoder(ADM_videoEncoder6* videoEncoder)
{
	CONFcouple *configuration = NULL;

	if (videoEncoder->desc->getConfigurationData)
	{
		videoEncoder->desc->getConfigurationData(&configuration);
	}

    *(this->_stream) << "adm.videoCodec(\"" << videoEncoder->desc->encoderName << "\"";
    this->dumpConfCouple(configuration);
	*(this->_stream) << ");" << std::endl;

	delete configuration;
}

void SpiderMonkeyScriptWriter::stretchAudio(int trackIndex, FILMCONV fps)
{

}

void SpiderMonkeyScriptWriter::dumpConfCouple(CONFcouple *c)
{
    if (!c)
    {
        return;
    }

    int count = 0;

    for (unsigned int j = 0; j < c->getSize(); j++)
    {
        char *name, *value;

        c->getInternalName(j, &name, &value);
        *(this->_stream) << ", \"" << name << "=" << value << "\"";

        // tinyPy does not like line > 1024 chars
        if (count >= 20)
        {
            *(this->_stream) << std::endl;
            count = 0;
        }

        count++;
    }
}

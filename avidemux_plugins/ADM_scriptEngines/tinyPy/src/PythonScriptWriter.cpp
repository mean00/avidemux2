#include "PythonScriptWriter.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerInternal.h"
#include "ADM_audiocodec/ADM_audiocodec.h"
#include "ADM_edEditableAudioTrack.h"
#include "ADM_edAudioTrackExternal.h"

PythonScriptWriter::PythonScriptWriter()
{
	this->_stream = NULL;
}

void PythonScriptWriter::setAudioPoolLanguage(int trackIndex, const char *lang) // ! from pool, not activeAudioTrack
{
     *(this->_stream) << "adm.setSourceTrackLanguage("<<trackIndex<<",\""  << lang << "\")" << std::endl;
}
void PythonScriptWriter::addExternalAudioTrack(int trackIndex,const char *file)
{
    
    *(this->_stream) << "adm.audioAddExternal(\"" << file << "\")" << std::endl;
}
void PythonScriptWriter::addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track)
{	
    *(this->_stream) << "adm.audioAddTrack(" << track->poolIndex << ")" << std::endl;
    *(this->_stream) << "adm.audioCodec(" << trackIndex << ", \"" << encoder->codecName << "\"";
    this->dumpConfCouple(track->encoderConf);
    *(this->_stream) << ");" << std::endl;
}

void PythonScriptWriter::addSegment(uint32_t videoIndex, uint64_t startTime, uint64_t duration)
{
    *(this->_stream) << "adm.addSegment(" << videoIndex << ", " << startTime << ", " << duration << ")" << std::endl;
}

void PythonScriptWriter::addVideoFilter(ADM_vf_plugin *plugin, ADM_VideoFilterElement *element)
{
    *(this->_stream) << "adm.addVideoFilter(\"" << plugin->getInternalName() << "\"";

	CONFcouple *configuration;

	element->instance->getCoupledConf(&configuration);
    this->dumpConfCouple(configuration);
	delete configuration;

    *(this->_stream) << ")" << std::endl;
}

void PythonScriptWriter::appendVideo(const char* path)
{
    *(this->_stream) << "adm.appendVideo(\"" << path << "\")" << std::endl;
}

void PythonScriptWriter::clearAudioTracks()
{
    *(this->_stream) << "adm.audioClearTracks()" << std::endl;
}

void PythonScriptWriter::clearSegments()
{
    *(this->_stream) << "adm.clearSegments()" << std::endl;
}

void PythonScriptWriter::closeVideo()
{
    
}

void PythonScriptWriter::connectStream(std::iostream& stream)
{
    this->_stream = &stream;

    *(this->_stream) << "#PY  <- Needed to identify #" << std::endl;
    *(this->_stream) << "#--automatically built--" << std::endl << std::endl;
    *(this->_stream) << "adm = Avidemux()" << std::endl;
}

void PythonScriptWriter::disconnectStream()
{
    this->_stream = NULL;
}

void PythonScriptWriter::loadVideo(const char* path)
{
    *(this->_stream) << "adm.loadVideo(\"" << path << "\")" << std::endl;
}

void PythonScriptWriter::setAudioGain(int trackIndex, ADM_GAINMode gainMode, uint32_t gainValue)
{
    *(this->_stream) << "adm.audioSetNormalize(" << trackIndex << ", " << gainMode << ", " << gainValue << ")" << std::endl;
}

void PythonScriptWriter::setAudioMixer(int trackIndex, CHANNEL_CONF mixer)
{
    const char *mixerString = NULL;

    for (unsigned int i = 0; i < NB_MIXER_DESC; i++)
    {
        if (mixer == mixerStringDescriptor[i].conf)
        {
            mixerString = mixerStringDescriptor[i].desc;
        }
    }

    *(this->_stream) << "adm.audioSetMixer(" << trackIndex << ", \"" << mixerString << "\");" << std::endl;
}
void PythonScriptWriter::setAudioDrc(int trackIndex, bool active)
{
    *(this->_stream) << "adm.audioSetDrc(" << trackIndex << ", " << active << ")" << std::endl;
}
void PythonScriptWriter::setAudioShift(int trackIndex, bool active,int32_t value)
{
    *(this->_stream) << "adm.audioSetShift(" << trackIndex << ", " << active << "," << value << ")" << std::endl;
}


void PythonScriptWriter::setAudioResample(int trackIndex, uint32_t resample)
{
    *(this->_stream) << "adm.audioSetResample(" << trackIndex << ", " << resample << ")" << std::endl;
}

void PythonScriptWriter::setMarkers(uint64_t markerA, uint64_t markerB)
{
    *(this->_stream) << "adm.markerA = " << markerA << std::endl;
    *(this->_stream) << "adm.markerB = " << markerB << std::endl;
}

void PythonScriptWriter::setMuxer(ADM_dynMuxer *muxer)
{
	CONFcouple *configuration;

	muxer->getConfiguration(&configuration);

	*(this->_stream) << "adm.setContainer(\"" << muxer->name << "\"";
    this->dumpConfCouple(configuration);
    *(this->_stream) << ")" << std::endl;

	delete configuration;
}

void PythonScriptWriter::setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv)
{
    *(this->_stream) << "adm.setPostProc(" << type << ", " << strength << ", " << swapUv << ")" << std::endl;
}

void PythonScriptWriter::setVideoEncoder(ADM_videoEncoder6* videoEncoder)
{
	CONFcouple *configuration = NULL;

	if (videoEncoder->desc->getConfigurationData)
    {
        videoEncoder->desc->getConfigurationData(&configuration);
    }

    *(this->_stream) << "adm.videoCodec(\"" << videoEncoder->desc->encoderName << "\"";
    this->dumpConfCouple(configuration);
	*(this->_stream) << ")" << std::endl;

	delete configuration;
}

void PythonScriptWriter::stretchAudio(int trackIndex, FILMCONV fps)
{
    switch (fps)
    {
        case FILMCONV_NONE:
            *(this->_stream) << "adm.audioSetPal2Film(" << trackIndex << ", 0)" << std::endl;
            *(this->_stream) << "adm.audioSetFilm2Pal(" << trackIndex << ", 0)" << std::endl;
            break;

        case FILMCONV_PAL2FILM:
            *(this->_stream) << "adm.audioSetPal2Film(" << trackIndex << ", 1)" << std::endl;
            break;

        case FILMCONV_FILM2PAL:
            *(this->_stream) << "adm.audioSetFilm2Pal(" << trackIndex << ", 1)" << std::endl;
            break;

		default:
			ADM_assert(0);
			break;
    }
}

void PythonScriptWriter::dumpConfCouple(CONFcouple *c)
{
    if (!c)
    {
        return;
    }

    std::string str;
    for (unsigned int j = 0; j < c->getSize(); j++)
    {
        char *name, *value;

        c->getInternalName(j, &name, &value);
        str=str+std::string(", \"")+std::string(name)+std::string("=")+std::string(value)+std::string("\"");
        
        // tinyPy does not like line > 1024 chars
        if (str.length() >= 200)
        {
            *(this->_stream) << str;
            *(this->_stream) << std::endl;
            str="";
        }
    }
    *(this->_stream) << str;
}

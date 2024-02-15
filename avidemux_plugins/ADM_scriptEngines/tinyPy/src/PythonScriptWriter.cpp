#include "PythonScriptWriter.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerInternal.h"
#include "ADM_audiocodec.h"
#include "ADM_edEditableAudioTrack.h"
#include "ADM_edAudioTrackExternal.h"

static char *escapeString(const char *in);

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
    char *escapedPath = escapeString(file);
    *(this->_stream) << "if not adm.audioAddExternal(\"" << escapedPath << "\"):" << std::endl;
    *(this->_stream) << "    raise(\"Cannot add external audio track from " << file << "\")" << std::endl;
    ADM_dealloc(escapedPath);
    escapedPath = NULL;
}
void PythonScriptWriter::addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track)
{
    *(this->_stream) << "if adm.audioTotalTracksCount() <= " << track->poolIndex << ":" << std::endl;
    *(this->_stream) << "    raise(\"Cannot add audio track " << track->poolIndex << ", total tracks: \" + str(adm.audioTotalTracksCount()))" << std::endl;
    *(this->_stream) << "adm.audioAddTrack(" << track->poolIndex << ")" << std::endl;
    *(this->_stream) << "adm.audioCodec(" << trackIndex << ", \"" << encoder->codecName << "\"";
    this->dumpConfCouple(track->encoderConf);
    *(this->_stream) << ")" << std::endl;
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
    char *escapedPath = escapeString(path);
    *(this->_stream) << "if not adm.appendVideo(\"" << escapedPath << "\"):" << std::endl;
    *(this->_stream) << "    raise(\"Cannot append " << path << "\")" << std::endl;
    ADM_dealloc(escapedPath);
    escapedPath = NULL;
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
    char *escapedPath = escapeString(path);
    *(this->_stream) << "if not adm.loadVideo(\"" << escapedPath << "\"):" << std::endl;
    *(this->_stream) << "    raise(\"Cannot load " << path << "\")" << std::endl;
    ADM_dealloc(escapedPath);
    escapedPath = NULL;
}

void PythonScriptWriter::setAudioGain(int trackIndex, ADM_GAINMode gainMode, int32_t gainValue, int32_t maxLevel)
{
    *(this->_stream) << "adm.audioSetNormalize2(" << trackIndex << ", " << gainMode << ", " << gainValue << ", " << maxLevel << ")" << std::endl;
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
void PythonScriptWriter::setAudioDrc(int trackIndex, bool active, int normalize, float nFloor, float attTime, float decTime, float ratio, float thresDB)
{
    *(this->_stream) << "adm.audioSetDrc2(" << trackIndex << ", " << active  << ", " << normalize << ", " << nFloor << ", " << attTime << ", " << decTime << ", " << ratio << ", " << thresDB << ")" << std::endl;
}
void PythonScriptWriter::setAudioEq(int trackIndex, bool active, float lo, float md, float hi, float lmcut, float mhcut)
{
    *(this->_stream) << "adm.audioSetEq(" << trackIndex << ", " << active  << ", " << lo << ", " << md << ", " << hi << ", " << lmcut << ", " << mhcut << ")" << std::endl;
}
void PythonScriptWriter::setAudioChannelGains(int trackIndex, float fL, float fR, float fC, float sL, float sR, float rL, float rR, float rC, float LFE)
{
    *(this->_stream) << "adm.audioSetChannelGains(" << trackIndex << ", " << fL  << ", " << fR << ", " << fC << ", " << sL << ", " << sR << ", " << rL << ", " << rR << ", " << rC << ", " << LFE << ")" << std::endl;
}
void PythonScriptWriter::setAudioChannelDelays(int trackIndex, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE)
{
    *(this->_stream) << "adm.audioSetChannelDelays(" << trackIndex << ", " << fL  << ", " << fR << ", " << fC << ", " << sL << ", " << sR << ", " << rL << ", " << rR << ", " << rC << ", " << LFE << ")" << std::endl;
}
void PythonScriptWriter::setAudioChannelRemap(int trackIndex, bool active, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE)
{
    *(this->_stream) << "adm.audioSetChannelRemap(" << trackIndex << ", " << active  << ", " << fL  << ", " << fR << ", " << fC << ", " << sL << ", " << sR << ", " << rL << ", " << rR << ", " << rC << ", " << LFE << ")" << std::endl;
}
void PythonScriptWriter::setAudioShift(int trackIndex, bool active,int32_t value)
{
    *(this->_stream) << "adm.audioSetShift(" << trackIndex << ", " << active << ", " << value << ")" << std::endl;
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

void PythonScriptWriter::setHDRConfig(uint32_t toneMappingMethod, float saturationAdjust, float boostAdjust, bool adaptiveRGB, uint32_t gamutMethod)
{
    *(this->_stream) << "adm.setHDRConfig(" << toneMappingMethod << ", " << saturationAdjust << ", " << boostAdjust << ", " << adaptiveRGB << ", " << gamutMethod << ")" << std::endl;
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

void PythonScriptWriter::stretchAudioCustom(int trackIndex, double tempo, double pitch)
{
    *(this->_stream) << "adm.audioSetCustomFrameRate(" << trackIndex << "," << tempo << "," << pitch << ")" << std::endl;
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
        // name cannot contain characters in need of escaping, right?
        char *escaped = escapeString(value);
        str += ", \"";
        str += name;
        str += "=";
        str += escaped;
        str += "\"";
        ADM_dealloc(escaped);
        escaped = NULL;
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

/**
 * \fn escapeString
 * \brief Escape backslashes and double quotes.
 *        Returned pointer must be freed with ADM_dealloc().
 */
char *escapeString(const char *in)
{
    ADM_assert(in);

    int sz = (int)strlen(in);
    int outlen = sz;
    for(int i = 0; i < sz; i++)
    {
        switch(in[i])
        {
            case '\\':
            case '\"':
                outlen++;
            default:break;
        }
    }
    if(outlen == sz) // no escaping needed
        return ADM_strdup(in);

    char *out = (char *)ADM_alloc(outlen+1);
    int off = 0;
    int checked = 0;
    for(int i = 0; i < outlen; i++)
    {
        if(off > sz) break;
        if(!checked)
        {
            switch(in[off])
            {
                case '\\':
                case '\"':
                    out[i] = '\\';
                    checked = 1;
                    continue;
                default:break;
            }
        }
        checked = 0;
        out[i] = in[off];
        off++;
    }
    out[outlen] = '\0';
    return out;
}
// EOF

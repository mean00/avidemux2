#ifndef IScriptWriter_h
#define IScriptWriter_h

#include <iostream>

#include "ADM_inttype.h"
#include "audiofilter_normalize_param.h"
#include "ADM_audiodef.h"
#include "audioencoderInternal.h"
#include "ADM_coreVideoFilter.h"

class ADM_videoEncoder6;
class ADM_dynMuxer;
class EditableAudioTrack;

class IScriptWriter
{
public:
    virtual ~IScriptWriter() {}
    virtual void addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track) = 0;
    virtual void addSegment(uint32_t videoIndex, uint64_t startTime, uint64_t duration) = 0;
    virtual void addVideoFilter(ADM_vf_plugin *plugin, ADM_VideoFilterElement *element) = 0;
    virtual void appendVideo(const char* path) = 0;
    virtual void clearAudioTracks() = 0;
    virtual void clearSegments() = 0;
    virtual void closeVideo() = 0;
    virtual void connectStream(std::iostream& stream) = 0;
    virtual void disconnectStream() = 0;
    virtual void loadVideo(const char* path) = 0;
    virtual void setAudioGain(int trackIndex, ADM_GAINMode gainMode, uint32_t gainValue) = 0;
    virtual void setAudioMixer(int trackIndex, CHANNEL_CONF mixer) = 0;
    virtual void setAudioResample(int trackIndex, uint32_t resample) = 0;
    virtual void setMarkers(uint64_t markerA, uint64_t markerB) = 0;
    virtual void setMuxer(ADM_dynMuxer *muxer) = 0;
    virtual void setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv) = 0;
    virtual void setVideoEncoder(ADM_videoEncoder6* videoEncoder) = 0;
    virtual void stretchAudio(int trackIndex, FILMCONV fps) = 0;
    virtual void setAudioDrc(int trackIndex, bool active) = 0;
    virtual void setAudioShift(int trackIndex, bool active,int32_t shiftMs) = 0;
    virtual void setAudioPoolLanguage(int trackIndex, const char *lang)=0; // ! from pool, not activeAudioTrack
    virtual void addExternalAudioTrack(int trackIndex,const char *file)=0;
};

#endif

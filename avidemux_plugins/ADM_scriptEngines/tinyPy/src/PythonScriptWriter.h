#ifndef PythonScriptWriter_h
#define PythonScriptWriter_h

#include "IScriptWriter.h"

class PythonScriptWriter : public IScriptWriter
{
private:
    std::iostream* _stream;

    void dumpConfCouple(CONFcouple *c);

public:
	PythonScriptWriter();

    void addAudioOutput(int trackIndex, ADM_audioEncoder *encoder, EditableAudioTrack* track);
    void addSegment(uint32_t videoIndex, uint64_t startTime, uint64_t duration);
    void addVideoFilter(ADM_vf_plugin *plugin, ADM_VideoFilterElement *element);
    void appendVideo(const char* path);
    void clearAudioTracks();
    void clearSegments();
    void closeVideo();
    void connectStream(std::iostream& stream);
    void disconnectStream();
    void loadVideo(const char* path);
    void setAudioGain(int trackIndex, ADM_GAINMode gainMode, int32_t gainValue, int32_t maxLevel);
    void setAudioMixer(int trackIndex, CHANNEL_CONF mixer);
    void setAudioResample(int trackIndex, uint32_t resample);
    void setMarkers(uint64_t markerA, uint64_t markerB);
    void setMuxer(ADM_dynMuxer *muxer);
    void setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv);
    void setHDRConfig(uint32_t toneMappingMethod, float saturationAdjust, float boostAdjust, bool adaptiveRGB, uint32_t gamutMethod);
    void setVideoEncoder(ADM_videoEncoder6* videoEncoder);
    void stretchAudio(int trackIndex, FILMCONV fps);
    void stretchAudioCustom(int trackIndex, double tempo, double pitch);
    void setAudioDrc(int trackIndex, bool active, int normalize, float nFloor, float attTime, float decTime, float ratio, float thresDB);
    void setAudioEq(int trackIndex, bool active, float lo, float md, float hi, float lmcut, float mhcut);
    void setAudioFade(int trackIndex, float fadeIn, float fadeOut, bool videoFilterBridge);
    void setAudioChannelGains(int trackIndex, float fL, float fR, float fC, float sL, float sR, float rL, float rR, float rC, float LFE);
    void setAudioChannelDelays(int trackIndex, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE);
    void setAudioChannelRemap(int trackIndex, bool active, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE);
    void setAudioShift(int trackIndex, bool active,int32_t shiftMs);
    void setAudioPoolLanguage(int trackIndex, const char *lang); // ! from pool, not activeAudioTrack
    void addExternalAudioTrack(int trackIndex,const char *file);
};

#endif

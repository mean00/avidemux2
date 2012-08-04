#ifndef QtScriptWriter_h
#define QtScriptWriter_h

#include "IScriptWriter.h"
#include "AdmScriptMapper.h"

namespace ADM_qtScript
{
	class QtScriptWriter : public IScriptWriter
	{
	private:
		std::iostream* _stream;

        AdmScriptMapper _mapper;
		void dumpConfCoupleDiff(const QString& prefix, CONFcouple *oldConf, CONFcouple *newConf);

	public:
		QtScriptWriter();

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
		void setAudioGain(int trackIndex, ADM_GAINMode gainMode, uint32_t gainValue);
		void setAudioMixer(int trackIndex, CHANNEL_CONF mixer);
		void setAudioResample(int trackIndex, uint32_t resample);
		void setMarkers(uint64_t markerA, uint64_t markerB);
		void setMuxer(ADM_dynMuxer *muxer);
		void setPostProcessing(uint32_t type, uint32_t strength, uint32_t swapUv);
		void setVideoEncoder(ADM_videoEncoder6* videoEncoder);
		void stretchAudio(int trackIndex, FILMCONV fps);
        void setAudioDrc(int trackIndex, bool active);
	};
}
#endif

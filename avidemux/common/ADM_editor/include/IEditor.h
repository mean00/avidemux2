#ifndef IEditor_h
#define IEditor_h

#include "ADM_Video.h"
#include "ADM_confCouple.h"
#include "audiofilter_normalize_param.h"
/**
    \class IEditor
    \brief i/f to editor
*/
class EditableAudioTrack;
class ADM_audioStream;
/**
    \class IEditor
*/
class IEditor
{
public:
    virtual ~IEditor(void) {}
	virtual bool addSegment(uint32_t ref, uint64_t startRef, uint64_t duration) = 0;
	virtual int addVideoFilter(const char *filter, CONFcouple *c) = 0;
	virtual int appendFile(const char *name) = 0;
	virtual void clearFilters() = 0;
	virtual bool clearSegment(void) = 0;
	virtual void dumpSegment(int i) = 0;
	virtual void dumpSegments(void) = 0;
	virtual bool dumpRefVideos(void) = 0;
	virtual bool dumpTiming(void) = 0;
	virtual bool getDirectImageForDebug(uint32_t frameNum, ADMCompressedImage *img) = 0;
	virtual uint64_t getMarkerAPts() = 0;
	virtual uint64_t getMarkerBPts() = 0;
	virtual uint32_t getNbSegment(void) = 0;
	virtual uint32_t getPARWidth(void) = 0;
	virtual uint32_t getPARHeight(void) = 0;
	virtual char *getVideoCodec(void) = 0;
	virtual uint64_t getVideoDuration(void) = 0;
	virtual uint8_t	getVideoInfo(aviInfo *info) = 0;
	virtual bool getVideoPtsDts(uint32_t frame, uint32_t *flags, uint64_t *pts, uint64_t *dts) = 0;
	virtual int	openFile(const char *name) = 0;
	virtual bool rewind(void) = 0;
	virtual int saveImageBmp(const char *filename) = 0;
	virtual int saveImageJpg(const char *filename) = 0;
	virtual int saveFile(const char *name) = 0;
	virtual bool setContainer(const char *cont, CONFcouple *c) = 0;
	virtual bool setMarkerAPts(uint64_t pts) = 0;
	virtual bool setMarkerBPts(uint64_t pts) = 0;
	virtual uint8_t setPostProc(uint32_t type, uint32_t strength, bool swapuv) = 0;
	virtual int setVideoCodec(const char *codec, CONFcouple *c) = 0;
    /* Audio related */
    virtual bool    clearAudioTracks(void)=0; /// remove all audio tracks
    virtual bool    addAudioTrack(int poolIndex)=0; /// Add an audio track in the active tracks
    virtual bool    addExternalAudioTrack(const char *fileName)=0; /// add an external audio track to the pool
	virtual bool    setAudioCodec(int dex,const char *codec, CONFcouple *c) = 0;
	virtual bool    setAudioFilterFrameRate(int dex,FILMCONV conf) = 0;
	virtual bool    setAudioFilterNormalise(int dex,ADM_GAINMode mode, uint32_t gain) = 0;
	virtual int     setAudioMixer(int dex,const char *s) = 0;
	virtual void    setAudioResample(int dex,uint32_t newfq) = 0;
	virtual FILMCONV getAudioFilterFrameRate(int dex) = 0;
	virtual bool    getAudioFilterNormalise(int dex,ADM_GAINMode *mode, uint32_t *gain) = 0;
	virtual uint32_t getAudioResample(int dex) = 0;
	virtual void    resetAudioFilter(int dex) = 0;
	virtual int     saveAudio(int dex,const char *name) = 0;

    virtual bool    changeAudioStream(uint64_t xtime, uint32_t newstream) = 0;
    virtual bool    getAudioStreamsInfo(uint64_t xtime, uint32_t *nbStreams, audioInfo **infos) = 0;
	virtual uint32_t getCurrentAudioStreamNumber(uint64_t xtime) = 0;
    virtual EditableAudioTrack *getEditableAudioTrackAt(int i)=0;
    virtual ADM_audioStream    *getAudioStreamAt(int i)=0;


};
#endif

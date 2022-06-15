/***************************************************************************
     \file  IEditor.h
     \brief Editor Interface class
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

#include "ADM_Video.h"
#include "ADM_image.h"
#include "ADM_confCouple.h"
#include "audiofilter_normalize_param.h"
#include "ADM_segment.h"
#include "ADM_muxerInternal.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_edActiveAudioTracks.h"
#include "ADM_edPoolOfAudioTracks.h"

class ADM_audioStream;

/**
    \class IEditor
    \brief i/f to editor
 */
class IEditor {
public:

    virtual ~IEditor(void) {
    }
    virtual bool addSegment(uint32_t ref, uint64_t startRef, uint64_t duration) = 0;
    virtual int addVideoFilter(const char *filter, CONFcouple *c) = 0;
    virtual int appendFile(const char *name) = 0;
    virtual void clearFilters() = 0;
    virtual bool clearSegment(void) = 0;
    virtual void closeFile(void) = 0;
    virtual void dumpSegment(int i) = 0;
    virtual void dumpSegments(void) = 0;
    virtual bool dumpRefVideos(void) = 0;
    virtual bool dumpTiming(void) = 0;
    virtual bool getCurrentFrameFlags(uint32_t *flags, uint32_t *quantiser) = 0;
    virtual uint64_t getCurrentFramePts(void) = 0;
    virtual bool getDirectImageForDebug(uint32_t frameNum, ADMCompressedImage *img) = 0;
    virtual uint64_t getFrameIncrement(bool copy=false) = 0;
    virtual uint64_t getMarkerAPts() = 0;
    virtual uint64_t getMarkerBPts() = 0;
    virtual uint32_t getNbSegment(void) = 0;
    virtual uint32_t getPARWidth(void) = 0;
    virtual uint32_t getPARHeight(void) = 0;
    virtual bool getPKFramePTS(uint64_t *frameTime) = 0;
    virtual bool getNKFramePTS(uint64_t *frameTime) = 0;
    virtual uint8_t getPostProc(uint32_t *type, uint32_t *strength, bool *swapuv) = 0;
    virtual uint8_t getHDRConfig( uint32_t * toneMappingMethod, float * saturationAdjust, float * boostAdjust, bool * adaptiveRGB, uint32_t * gamutMethod) = 0;
    virtual _SEGMENT* getSegment(int i) = 0;
    virtual char *getVideoCodec(void) = 0;
    virtual int getVideoCount(void) = 0;
    virtual uint64_t getVideoDuration(void) = 0;
    virtual uint8_t getVideoInfo(aviInfo *info, aviColorInfo *colfo = NULL) = 0;
    virtual bool getTimeBase(uint32_t *scale, uint32_t *rate, bool copy=false) = 0;
    virtual _VIDEOS* getRefVideo(int videoIndex) = 0;
    virtual bool getVideoPtsDts(uint32_t frame, uint32_t *flags, uint64_t *pts, uint64_t *dts) = 0;
    virtual bool goToIntraTimeVideo(uint64_t time) = 0;
    virtual bool goToTimeVideo(uint64_t time) = 0;
    virtual bool isFileOpen(void) = 0;
    virtual bool nextPicture(ADMImage *image = NULL) = 0;
    virtual bool samePicture(ADMImage *image = NULL) = 0;
    virtual int openFile(const char *name) = 0;
    virtual bool rewind(void) = 0;
    virtual int saveImageBmp(const char *filename) = 0;
    virtual int saveImageJpg(const char *filename) = 0;
    virtual int saveImagePng(const char *filename) = 0;
    virtual int saveFile(const char *name) = 0;
    virtual ADM_dynMuxer* getCurrentMuxer() = 0;
    virtual bool setContainer(const char *cont, CONFcouple *c) = 0;
    virtual bool setCurrentFramePts(uint64_t pts) = 0;
    virtual bool setMarkerAPts(uint64_t pts) = 0;
    virtual bool setMarkerBPts(uint64_t pts) = 0;
    virtual uint8_t setPostProc(uint32_t type, uint32_t strength, bool swapuv) = 0;
    virtual uint8_t setHDRConfig( uint32_t toneMappingMethod, float saturationAdjust, float boostAdjust, bool adaptiveRGB, uint32_t gamutMethod) = 0;
    virtual ADM_videoEncoder6* getCurrentVideoEncoder() = 0;
    virtual int setVideoCodec(const char *codec, CONFcouple *c) = 0;
    virtual int changeVideoParam(const char *codec, CONFcouple *c) = 0;
    /* Audio related */
    virtual bool clearAudioTracks(void) = 0; /// remove all audio tracks
    virtual bool addAudioTrack(int poolIndex) = 0; /// Add an audio track in the active tracks
    virtual int getNumberOfActiveAudioTracks(void) = 0; // returns # of audio tracks
    virtual bool addExternalAudioTrack(const char *fileName) = 0; /// add an external audio track to the pool
    virtual bool setAudioCodec(int dex, const char *codec, CONFcouple *c) = 0;
    virtual bool setAudioFilterFrameRate(int dex, FILMCONV conf) = 0;
    virtual bool setAudioFilterNormalise(int dex, ADM_GAINMode mode, int32_t gain, int32_t maxvalue) = 0;
    virtual int setAudioMixer(int dex, const char *s) = 0;
    virtual bool setAudioDrc(int track, bool active, int normalize, float nFloor, float attTime, float decTime, float ratio, float thresDB) = 0;
    virtual bool getAudioDrc(int track, bool * active, int * normalize, float * nFloor, float * attTime, float * decTime, float * ratio, float * thresDB) = 0;
    virtual bool setAudioEq(int track, bool active, float lo, float md, float hi, float lmcut, float mhcut) = 0;
    virtual bool getAudioEq(int track, bool * active, float * lo, float * md, float * hi, float * lmcut, float * mhcut) = 0;
    virtual bool setAudioFade(int track, float fadeIn, float fadeOut, bool videoFilterBridge) = 0;
    virtual bool getAudioFade(int track, float * fadeIn, float * fadeOut, bool * videoFilterBridge) = 0;
    virtual bool setAudioChannelGains(int dex, float fL, float fR, float fC, float sL, float sR, float rL, float rR, float rC, float LFE) = 0;
    virtual bool getAudioChannelGains(int dex, float * fL, float * fR, float * fC, float * sL, float * sR, float * rL, float * rR, float * rC, float * LFE) = 0;
    virtual bool setAudioChannelDelays(int dex, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE) = 0;
    virtual bool getAudioChannelDelays(int dex, int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE) = 0;
    virtual bool setAudioChannelRemap(int dex, bool active, int fL, int fR, int fC, int sL, int sR, int rL, int rR, int rC, int LFE) = 0;
    virtual bool getAudioChannelRemap(int dex, bool * active, int * fL, int * fR, int * fC, int * sL, int * sR, int * rL, int * rR, int * rC, int * LFE) = 0;
    virtual bool setAudioShift(int track, bool mode, int32_t value) = 0;
    virtual bool getAudioShift(int track, bool *mode, int32_t *value) = 0;
    virtual void setAudioResample(int dex, uint32_t newfq) = 0;
    virtual FILMCONV getAudioFilterFrameRate(int dex) = 0;
    virtual bool getAudioFilterNormalise(int dex, ADM_GAINMode *mode, int32_t *gain, int32_t *maxvalue) = 0;
    virtual bool getAudioFilterCustomFrameRate(int dex, double * tempo, double * pitch) = 0;
    virtual bool setAudioFilterCustomFrameRate(int dex, double tempo, double pitch) = 0;
    virtual uint32_t getAudioResample(int dex) = 0;
    virtual void resetAudioFilter(int dex) = 0;
    virtual int saveAudio(int dex, const char *name) = 0;
    virtual bool changeAudioStream(uint64_t xtime, uint32_t newstream) = 0;
    virtual bool getAudioStreamsInfo(uint64_t xtime, uint32_t *nbStreams, audioInfo **infos) = 0;
    virtual uint32_t getCurrentAudioStreamNumber(uint64_t xtime) = 0;
    virtual EditableAudioTrack *getEditableAudioTrackAt(int i) = 0;
    virtual ADM_audioStream *getAudioStreamAt(int i) = 0;
    virtual ActiveAudioTracks* getPoolOfActiveAudioTrack(void) = 0;
    virtual PoolOfAudioTracks* getPoolOfAudioTrack(void) = 0;
    virtual void updateDefaultAudioTrack(void) = 0;
    virtual bool seekFrame(int count) = 0;
    virtual bool seekKeyFrame(int count) = 0;
    virtual void seekBlackFrame(int count) = 0;
    virtual uint32_t getFrameSize(int count) = 0;
    virtual int  setVideoCodecProfile(const char *codec, const char *profile)=0;
    virtual bool audioSetAudioPoolLanguage(int poolIndex, const char *language)=0;
    // var
    virtual bool setVar(const char *key, const char *value)=0;
    virtual const char *getVar(const char *key)=0;
    virtual bool  printEnv(void)=0;
};

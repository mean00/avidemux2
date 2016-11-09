/***************************************************************************
     \file  ADM_segment.h
     \brief Handle segment

    (C) 2002-2009 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_SEGMENT_H
#define ADM_SEGMENT_H
#include <vector>
class ADM_audioStream;
class ADM_Audiocodec;
class decoders;
class COL_Generic2YV12;
class EditorCache;
#include "ADM_Video.h"
/**
    \class ADM_audioStreamTack
    \brief Place Holder for demuxer audio tracks.
*/
class ADM_audioStreamTrack
{
public:
    ADM_audioStream  *stream;
    audioInfo        *info;
    ADM_Audiocodec   *codec;
    WAVHeader        wavheader;
    bool             vbr;
    uint64_t         duration;
    uint64_t         size;

public:
    ADM_audioStreamTrack() {
                stream=NULL;
                info=NULL;
                codec=NULL;
                duration=size=0;
                vbr=false;
                memset(&wavheader,0,sizeof(wavheader));
        }
    ~ADM_audioStreamTrack();
};
/**
    \struct _VIDEOS
    \brief The _VIDEOS struct is a video we have loaded.
*/
class _VIDEOS
{
public:
      vidHeader *_aviheader; /// Demuxer
      decoders *decoder; /// Video codec
      ADMColorScalerSimple *color; /// Color conversion if needed
      bool              dontTrustBFramePts;
      /* Audio part */
      uint32_t currentAudioStream;
      std::vector <ADM_audioStreamTrack *>  audioTracks;
      

      uint32_t _nb_video_frames; /// Really needed ?
      EditorCache *_videoCache; /// Decoded video cache

      /* Timeing info */

      uint32_t lastSentFrame; /// Last frame read/sent to decoder
      uint64_t lastDecodedPts; /// Pts of last frame out of decoder
      uint64_t lastReadPts; /// Pts of the last frame we read
      uint64_t timeIncrementInUs; /// in case the video has no PTS, time increment (us)

      uint64_t firstFramePts; /// Pts of firstFrame

    _VIDEOS()
    {
        currentAudioStream=0;
        _aviheader=NULL;
        decoder=NULL;
        color=NULL;
        _nb_video_frames=0;
        _videoCache=NULL;
        lastSentFrame=0;
        lastDecodedPts=0;
        lastReadPts=0;
        timeIncrementInUs=0;
        firstFramePts=0;
    }

};

/**
    \struct _SEGMENT
    \brief The video is a collection of segment.
            Each segment refers to its source (the reference) and the part of the source the segment is made of.
*/


class _SEGMENT
{
public:
  typedef enum
  {
    ADM_NO_DROP                 =0,
    ADM_DROPPING                =2,
    ADM_DROP_MAYBE_AFER_SWITCH  =1
  }SegState;
  
public:
        uint32_t _reference; /// Reference video
        uint64_t _refStartTimeUs; /// Starting time in reference
        uint64_t _startTimeUs; /// Start time in current (=sum(_duration of previous seg))
        uint64_t _durationUs; ///
        SegState _dropBframes; /// Internal state machine to know is we should drop bframe that are orphean
        uint64_t _refStartDts;
        void clear(void) 
        {
            _reference=0;
            _refStartTimeUs=0;
            _startTimeUs=0;
            _durationUs=0;
            _dropBframes=ADM_NO_DROP;
            _refStartDts=0;
        }
        _SEGMENT() {clear();}
};
/*
    Use vectors to store our videos & segments
*/
typedef std::vector <_VIDEOS>  ListOfVideos;
typedef std::vector <_SEGMENT> ListOfSegments;

/**
    \class ADM_EditorSegment
*/
class ADM_EditorSegment
{
protected:
        ListOfSegments segments;
        ListOfSegments clipboard;
        ListOfVideos   videos;
        bool           updateStartTime(void);


public:
            void        dump(void);
            void        dumpSegment(int i);

            void        dumpRefVideos(void);
                        ADM_EditorSegment(void);
                        ~ADM_EditorSegment();

            bool        addReferenceVideo(_VIDEOS *ref);
            bool        halfFps(void);
            bool        updateRefVideo(void);
            bool        deleteAll(void);

            bool        resetSegment(void);
            bool        deleteSegments(void);
            bool        addSegment(_SEGMENT *seg);
            bool        isEmpty(void);

            bool        removeEmptySegments(void);

            _VIDEOS     *getRefVideo(int i);
            int         getNbRefVideos(void);

            _SEGMENT    *getSegment(int i);
            int         getNbSegments(void);

            ListOfSegments getSegments(void);
            bool        setSegments(ListOfSegments segm);

            uint64_t    getTotalDuration(void);
            uint32_t    getNbFrames(void);

            bool        getRefFromTime(uint64_t time,uint32_t *refVideo);

            bool        convertLinearTimeToSeg(  uint64_t frameTime, uint32_t *seg, uint64_t *segTime);
            bool        convertSegTimeToLinear(  uint32_t seg,uint64_t segTime, uint64_t *frameTime);


            uint32_t    intraTimeToFrame(uint32_t refVideo,uint64_t seekTime);       
            bool        isKeyFrameByTime(uint32_t refVideo,uint64_t seekTime);

            bool        removeChunk(uint64_t from, uint64_t to);
            bool        truncateVideo(uint64_t from);
            bool        dtsFromPts(uint32_t refVideo,uint64_t pts,uint64_t *dts);

            bool        LinearToRefTime(int segNo,uint64_t linear,uint64_t *refTime);
            
            bool        copyToClipBoard(uint64_t startTime, uint64_t endTime);
            bool        pasteFromClipBoard(uint64_t currentTime);
            bool        appendFromClipBoard(void);
            bool        dumpClipBoard();
protected:
            void        dumpSegmentsInternal(ListOfSegments &l);
};

#endif
//EOF

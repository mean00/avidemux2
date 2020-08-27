/***************************************************************************
                          ADM_mkv.h  -  description
                             -------------------

    Matroska demuxer

    copyright            : (C) 2008 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef ADM_MKV_H
#define ADM_MKV_H

#include "ADM_Video.h"
#include "ADM_audioStream.h"
#include "ADM_aacLatm.h"
#include "ADM_ebml.h"
#include <BVector.h>
#define MKV_MAX_REPEAT_HEADER_SIZE 16
/**
    \struct mkvIndex
    \brief defines a frame, audio or video
*/
typedef struct
{
    uint64_t pos;
    uint32_t size;
    uint32_t flags;
    uint64_t Dts;   // Dts in us-seconds
    uint64_t Pts;   // Pts in us
}mkvIndex;

typedef BVector <mkvIndex > mkvListOfIndex;
/**
    \struct mkvTrak
    \brief Hold information about a give track, the track #0  is always video.
*/
class mkvTrak
{
public:
  mkvTrak()
  {
        streamIndex=0;
        duration=0;
        memset(&wavHeader,0,sizeof(wavHeader));
        nbPackets=0;
        nbFrames=0;
        length=0;
        extraData=NULL;
        extraDataLen=0;
        infoCache=NULL;
        infoCacheSize=0;
        paramCache=NULL;
        paramCacheSize=0;
        headerRepeatSize=0;
        _sizeInBytes=0;
        _defaultFrameDuration=0;
        language=ADM_UNKNOWN_LANGUAGE;
        _needBuffering=0;
  }
  /* Index in mkv */
  uint32_t  streamIndex;
  uint64_t  duration;  // Duration in us (timecode of the last frame)
  /* Used for audio */
  WAVHeader wavHeader;
  uint32_t  nbPackets; // number of blocks (used for audio)
  uint32_t  nbFrames;  // number of distinct frames
  uint32_t  length;    // Number o;f bytes seen

  /* Used for both */
  uint8_t    *extraData;
  uint32_t   extraDataLen;
  uint8_t    *infoCache; // decoded SPS & friends
  uint32_t   infoCacheSize;
  uint8_t    *paramCache; // raw SPS data
  uint32_t   paramCacheSize;
  /* */
  uint32_t   headerRepeatSize;
  uint8_t    headerRepeat[MKV_MAX_REPEAT_HEADER_SIZE];
  mkvListOfIndex  index;

  uint32_t  _sizeInBytes; // Approximate size in bytes of that stream
  uint32_t  _defaultFrameDuration; // Duration of ONE frame in us!
  std::string language;
  int       _needBuffering;
};

#define MKV_MAX_LACES 101 // ?
#define MKV_MUX_LATM 0x1602
/**
 */
class mkvAccess;
class mkvAccessBuffered : public ADM_audioAccess
{
public:
            mkvAccessBuffered(mkvAccess *ac,int maxSize);
            ~mkvAccessBuffered();
                virtual bool      isCBR(void) { return false;}
                virtual bool      canSeekTime(void) {return true;};
                virtual bool      canSeekOffset(void) {return false;};
                virtual bool      canGetDuration(void) {return true;};
                virtual uint64_t  getDurationInUs(void);
                virtual bool      goToTime(uint64_t timeUs);
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                virtual bool      getExtraData(uint32_t *l, uint8_t **d);  
protected:
            mkvAccess   *_son;
            int         _maxSize;
            uint8_t     *_buffer;
            int         _inBuffer;
            int         _offset;
};

class mkvAccessLatm : public ADM_audioAccess
{
public:
            mkvAccessLatm(mkvAccess *ac,int maxSize);
            ~mkvAccessLatm();
                virtual bool      isCBR(void) { return false; }
                virtual bool      canSeekTime(void) { return true; };
                virtual bool      canSeekOffset(void) { return false; };
                virtual bool      canGetDuration(void) { return true; };
                virtual uint64_t  getDurationInUs(void);
                virtual bool      goToTime(uint64_t timeUs);
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                virtual bool      getExtraData(uint32_t *l, uint8_t **d);
protected:
            ADM_latm2aac latm;
            mkvAccess   *_son;
            int         _maxSize;
            uint8_t     *_buffer;
            bool        updateExtraData(uint64_t timeUs);
};

/**
    \class mkvAccess
    \brief Matroska audio demuxer
*/
class mkvAccess : public ADM_audioAccess
{
protected:
    mkvTrak                     *_track;
    ADM_ebml_file               *_parser;

    uint32_t                    _currentBlock;
    uint32_t                    _currentLace;
    uint32_t                    _maxLace;
    uint32_t                    _Laces[MKV_MAX_LACES];
    uint64_t                    _laceIncrementUs;
    uint64_t                    _lastDtsBase;

    uint8_t                     goToBlock(uint32_t x);
    bool                        initLaces(uint32_t nbLaces,uint64_t time);
     int              readAndRepeat(uint8_t *buffer, uint32_t len, uint32_t max)
                        {
                             uint32_t rpt=_track->headerRepeatSize;
                             if(rpt+len>max)
                               {
                                 ADM_error("Overflow in reading  mkv audio : %u (%u) max was %d\n",rpt+len,rpt,max);
                                 ADM_assert(0);
                               }
                              _parser->readBin(buffer+rpt,len);
                              if(rpt)
                                memcpy(buffer,_track->headerRepeat,rpt);
                              return len+rpt;
                        }

public:
                                  mkvAccess(const char *name,mkvTrak *track);
                virtual           ~mkvAccess();
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return false;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return true;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return true;};
                                    ///
                virtual uint64_t  getDurationInUs(void);
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs);
                                    /// Grab extra data
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                virtual bool      getExtraData(uint32_t *l, uint8_t **d);
};



#define ADM_MKV_MAX_TRACKS 20
/**
    \class mkvHeader
    \brief Matroska demuxer
*/
class mkvHeader         :public vidHeader
{
  protected:
    uint64_t                 _timeBase;  // Time base in us, default is 1000=1 ms
    ADM_audioAccess         **_access;
    ADM_audioStream         **_audioStreams;
    ADM_ebml_file           *_parser;
    char                    *_filename;
    mkvTrak                 _tracks[ADM_MKV_MAX_TRACKS+1];

    BVector <mkvIndex    >   _clusters;
    BVector <uint64_t>       _cueTime;

    std::vector <uint64_t>  _sortedPts;
    std::vector <uint32_t>  _framesNoPts;

    uint32_t                _nbAudioTrack;
    uint32_t                _currentAudioTrack;

    uint64_t                _segmentPosition;
    uint64_t                _cuePosition;
    uint64_t                _trackPosition;
    uint32_t                _H264Recovery;

    uint8_t                 checkHeader(void *head,uint32_t headlen);
    bool                    analyzeTracks(ADM_ebml_file *parser);
    uint8_t                 analyzeOneTrack(void *head,uint32_t headlen);
    uint8_t                 walk(void *seed);
    uint64_t                walkAndFind(void *seed,MKV_ELEM_ID searched);
    int                     searchTrackFromTid(uint32_t tid);
    bool                    readSeekHead(ADM_ebml_file *body,uint64_t *nexthead=NULL);
    //
    //                      
    int                     isBufferingNeeded(mkvTrak *trk); // Split audio blocks into smaller pieces if needed
    uint8_t                 reformatVorbisHeader(mkvTrak *trk);
    // Indexers

    uint8_t                 addIndexEntry(uint32_t track,ADM_ebml_file *parser,uint64_t where, uint32_t size,uint32_t flags,
                                            uint32_t timecodeMS);
    uint8_t                 videoIndexer(ADM_ebml_file *parser);
    bool                    readCue(ADM_ebml_file *parser);
    uint8_t                 indexClusters(ADM_ebml_file *parser);
    uint8_t                 indexBlock(ADM_ebml_file *parser,uint32_t count,uint32_t timecodeMS);

    uint8_t                 rescaleTrack(mkvTrak *track,uint32_t durationMs);

    bool                    delayTrack(int index,mkvTrak *track, uint64_t value);

    bool                    ComputeDeltaAndCheckBFrames(uint32_t *minDeltaX, uint32_t *maxDeltaX, bool *bFramePresent);
    bool                    updateFlagsWithCue(void); // in case we can trust it, update KEY_FRAME_FLAGS
    bool                    dumpVideoIndex(int maxIndex);
    bool                    goBeforeAtomAtPosition(ADM_ebml_file *parser, uint64_t position,uint64_t &outputLen,
                                                MKV_ELEM_ID searchedId,const char *txt);
    int                     checkDeviation(int num, int den);
    bool                    enforceFixedFrameRate(int num, int den);
    bool                    setDtsFromListOfSortedPts(void);
  public:


    virtual   void          Dump(void);

            mkvHeader( void );
   virtual  ~mkvHeader(  ) ;
// AVI io
    virtual uint8_t  open(const char *name);
    virtual uint8_t  close(void) ;
    int              readAndRepeat(int index,uint8_t *buffer, uint32_t len)
                        {
                             uint32_t rpt=_tracks[index].headerRepeatSize;
                              _parser->readBin(buffer+rpt,len);
                              if(rpt)
                                memcpy(buffer,_tracks[index].headerRepeat,rpt);
                              return len+rpt;
                        }
  //__________________________
  //  Info
  //__________________________
 //__________________________
  //				 Audio
  //__________________________

virtual 	WAVHeader              *getAudioInfo(uint32_t i )  ;
virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio);
virtual     uint8_t                 getNbAudioStreams(void);
// Frames
  //__________________________
  //  video
  //__________________________

    virtual uint8_t  setFlag(uint32_t frame,uint32_t flags);
    virtual uint32_t getFlags(uint32_t frame,uint32_t *flags);
    virtual uint8_t  getFrameSize(uint32_t frame,uint32_t *size);
    virtual uint8_t  getFrame(uint32_t framenum,ADMCompressedImage *img);
    virtual uint64_t getTime(uint32_t frameNum);

    virtual uint64_t getVideoDuration(void);
    virtual	uint8_t	 getExtraHeaderData(uint32_t *len, uint8_t **data);

virtual   bool       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);

        // Temporary buffer for indexing
        uint8_t *readBuffer;
        int     readBufferSize;
};
#endif

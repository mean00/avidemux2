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
#include "ADM_ebml.h"

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


/**
    \struct mkvTrak
    \brief Hold information about a give track, the track #0  is always video.
*/
typedef struct
{
  /* Index in mkv */
  uint32_t  streamIndex;
  uint64_t  duration;  // Duration in us (timecode of the last frame)
  /* Used for audio */
  WAVHeader wavHeader;
  uint32_t  nbPackets; // number of blocks (used for audio)
  uint32_t  nbFrames;  // number of distinct frames
  uint32_t  length;    // Number of bytes seen
  
  /* Used for both */
  uint8_t    *extraData;
  uint32_t   extraDataLen;
  mkvIndex  *_index;
  uint32_t  _nbIndex;  // current size of the index
  uint32_t  _indexMax; // Max size of the index
  uint32_t  _sizeInBytes; // Approximate size in bytes of that stream
  uint32_t  _defaultFrameDuration; // Duration of ONE frame in us!
}mkvTrak;

#define MKV_MAX_LACES 20 // ?
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

    uint8_t                     goToBlock(uint32_t x);
 
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
    mkvAccess               **_access;
    ADM_audioStream         **_audioStreams;
    ADM_ebml_file           *_parser;
    char                    *_filename;
    mkvTrak                 _tracks[ADM_MKV_MAX_TRACKS+1];

    mkvIndex                *_clusters;
    uint32_t                _nbClusters;
    uint32_t                _clustersCeil;

    uint32_t                _nbAudioTrack;
    uint32_t                _currentAudioTrack;
    uint32_t                _reordered;

    uint8_t                 checkHeader(void *head,uint32_t headlen);
    uint8_t                 analyzeTracks(void *head,uint32_t headlen);
    uint8_t                 analyzeOneTrack(void *head,uint32_t headlen);
    uint8_t                 walk(void *seed);
    int                     searchTrackFromTid(uint32_t tid);
    //
    uint8_t                 reformatVorbisHeader(mkvTrak *trk);
    // Indexers

    uint8_t                 addIndexEntry(uint32_t track,uint64_t where, uint32_t size,uint32_t flags,
                                            uint32_t timecodeMS);
    uint8_t                 videoIndexer(ADM_ebml_file *parser);
    uint8_t                 readCue(ADM_ebml_file *parser);
    uint8_t                 indexClusters(ADM_ebml_file *parser);
    uint8_t                 indexBlock(ADM_ebml_file *parser,uint32_t count,uint32_t timecodeMS);

    uint8_t                 rescaleTrack(mkvTrak *track,uint32_t durationMs);
  public:


    virtual   void          Dump(void);

            mkvHeader( void );
   virtual  ~mkvHeader(  ) ;
// AVI io
    virtual uint8_t  open(const char *name);
    virtual uint8_t  close(void) ;
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



};
#endif



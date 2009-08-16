/***************************************************************************
                          ADM_pics.h  -  description
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002/2009 by mean
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
 
#ifndef ADM_ASF_H
#define ADM_ASF_H
#include <vector>
using std::vector;
#include "ADM_Video.h"
#include "ADM_queue.h"
#include "ADM_asfPacket.h"

#define ASF_MAX_AUDIO_TRACK 8



typedef struct 
{
  uint32_t packetNb;
  uint32_t frameLen;
  uint32_t segNb;
  uint32_t flags;
  uint64_t dts;
  uint64_t pts;
  uint32_t audioSeen[ASF_MAX_AUDIO_TRACK];
  uint64_t audioDts[ASF_MAX_AUDIO_TRACK];
}asfIndex;

typedef  vector <asfIndex> AsfVectorIndex;

typedef enum 
{
  ADM_CHUNK_HEADER_CHUNK ,
  ADM_CHUNK_FILE_HEADER_CHUNK,
  ADM_CHUNK_NO_AUDIO_CONCEAL,
  ADM_CHUNK_STREAM_HEADER_CHUNK,
  ADM_CHUNK_STREAM_GROUP_ID,
  ADM_CHUNK_DATA_CHUNK,
  ADM_CHUNK_HEADER_EXTENSION_CHUNK,
  ADM_CHUNK_CLOCK_TYPE_EX,
  ADM_CHUNK_LANGUAGE_LIST_EX,
  ADM_CHUNK_EXTENDED_STREAM_PROP,
  ADM_CHUNK_UNKNOWN_CHUNK
}ADM_KNOWN_CHUNK;

typedef struct 
{
  const char *name;
  uint32_t len;
  uint8_t val[16];
  ADM_KNOWN_CHUNK id; 
}chunky;

class asfChunk
{
  protected:
    FILE        *_fd;
    
    
  public:
  uint32_t  _chunkStart;
            asfChunk(FILE *f);
            ~asfChunk();
  uint8_t   dump(void);
  uint8_t   guId[16];
  uint64_t  chunkLen;
  
  uint8_t   readChunkPayload(uint8_t *data, uint32_t *dataLen);
  uint8_t   nextChunk(int shortChunk=0);
  uint8_t   skipChunk(void);
  uint64_t  read64(void);
  uint32_t  read32(void);
  uint32_t  read16(void);
  uint8_t   read8(void);
  uint8_t   read(uint8_t *where, uint32_t how);
  const chunky    *chunkId(void);
  uint8_t   skip(uint32_t skip);
};

typedef struct 
{
  uint32_t     streamIndex;
  uint32_t     extraDataLen;
  uint8_t      *extraData;
  uint32_t     nbPackets;
  uint32_t     length;
  WAVHeader    wavHeader;
}asfAudioTrak;

/**
    \class asfAudioAccess
    \brief Audio access class for asf/wmv
*/
class asfAudioAccess : public ADM_audioAccess
{
  protected:
    uint32_t                _myRank;
    char                    *myName;
    uint32_t                _streamId;
    uint32_t                _dataStart;
    asfPacket               *_packet;
    FILE                    *_fd;
    ADM_queue               readQueue;
    uint32_t                _packetSize;
    class asfHeader         *_father;
    asfAudioTrak            *_track;
  public:
                                asfAudioAccess(asfHeader *father,uint32_t rank);
    virtual                     ~asfAudioAccess();

    virtual bool      canSeekTime(void) {return true;};
    virtual bool      canSeekOffset(void) {return true;};
    virtual bool      canGetDuration(void) {return true;};
    
    virtual uint32_t  getLength(void) {return _track->length;}
    virtual bool      goToTime(uint64_t timeUs) ;
    virtual bool      isCBR(void) {return true;};
    
    
    virtual uint64_t  getPos(void);
    virtual bool      setPos(uint64_t pos);
    virtual uint64_t  getDurationInUs(void) ;

    virtual bool   getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
    
};

/**
    \class asfHeader
    \brief Asf Demuxer
*/  

class asfHeader         :public vidHeader
{
  protected:
    uint8_t                 getHeaders( void);
    uint8_t                 buildIndex(void);
    uint8_t                 loadVideo(asfChunk *s);
    
    ADM_queue               readQueue;
    uint32_t                curSeq;
    asfPacket               *_packet;
    //uint32_t                _currentAudioStream;
    uint64_t                _duration;  // Duration 100 ns
  protected:
                                
    FILE                    *_fd;

    int32_t                 _videoIndex;    
    uint32_t                _videoStreamId;

    uint8_t                 close(void);

    
  public: // Shared with audio track
    char                    *myName;
    
    uint32_t                nbImage;
    AsfVectorIndex          _index;
    uint32_t                _packetSize;
    uint32_t                _dataStartOffset;
    uint32_t                _nbAudioTrack;
    asfAudioAccess          *_audioAccess[ASF_MAX_AUDIO_TRACK];
    asfAudioTrak             _allAudioTracks[ASF_MAX_AUDIO_TRACK];
    ADM_audioStream         *_audioStreams[ASF_MAX_AUDIO_TRACK];
    uint32_t                 _nbPackets;
    
    
    // / Shared
  public:
                                        asfHeader(void);
   virtual                              ~asfHeader();
                uint8_t                 open(const char *name);
      //__________________________
      //				 Audio
      //__________________________

    virtual 	WAVHeader              *getAudioInfo(uint32_t i )  ;
    virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio);
    virtual     uint8_t                 getNbAudioStreams(void);
    // Frames
      //__________________________
      //				 video
      //__________________________

    virtual 	uint8_t                 setFlag(uint32_t frame,uint32_t flags);
    virtual 	uint32_t                getFlags(uint32_t frame,uint32_t *flags);			
    virtual 	uint8_t                 getFrameSize(uint32_t frame,uint32_t *size);
    virtual 	uint8_t                 getFrame(uint32_t framenum,ADMCompressedImage *img);

    virtual   void                       Dump(void);
    virtual   uint64_t                   getTime(uint32_t frameNum);
    virtual   uint64_t                   getVideoDuration(void);

    // Return true if the container provides pts informations
    virtual   bool                       providePts(void) {return false;};
    //
    virtual   bool                       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
    virtual   bool                       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);
 

};
#endif



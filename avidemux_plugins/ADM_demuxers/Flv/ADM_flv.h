/***************************************************************************
                         ADM_FLV
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
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
 


#ifndef ADM_FLV_H
#define ADM_FLV_H

#include "ADM_Video.h"
#include "ADM_audioStream.h"

typedef struct 
{
    uint64_t pos;       // Absolute position in bytes
    uint32_t size;      // Size in bytes
    uint32_t flags;
    uint64_t dtsUs;  // Time code in us from start
    uint64_t ptsUs;  // Time code in us from start
}flvIndex;
//**********************************************
class flvTrak 
{
public:
          flvTrak(int nb);
          ~flvTrak();
  uint8_t grow(void);
  //
  uint32_t  streamIndex;
  uint32_t  length;
  uint8_t    *extraData;
  uint32_t   extraDataLen;
  flvIndex  *_index;
  uint32_t  _nbIndex;  // current size of the index
  uint32_t  _indexMax; // Max size of the index
  uint32_t  _sizeInBytes; // Approximate size in bytes of that stream
  uint32_t  _defaultFrameDuration; // in us!
};
/**
    \fn ADM_flvAccess
*/
class ADM_flvAccess : public ADM_audioAccess
{
protected:
                      
                FILE             *_fd;
                flvTrak          *_track;
                uint32_t         currentBlock;
                bool             goToBlock(uint32_t block);


                
                
public:
                                  ADM_flvAccess(const char *name,flvTrak *trak); 
                virtual           ~ADM_flvAccess();
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return true;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return true;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return true;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void) ;
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs);
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);

                bool              getExtraData(uint32_t *l, uint8_t **d);
};


/**
    \Class flvHeader
    \brief Flash demuxer

*/
class flvHeader         :public vidHeader
{
  protected:
                                
    FILE                    *_fd;
    char                    *_filename;
    flvTrak                 *videoTrack;
    flvTrak                 *audioTrack;
    WAVHeader               wavHeader;
    ADM_audioStream         *_audioStream;
    ADM_flvAccess           *access;
    /* */
    uint32_t            metaWidth,metaHeight,metaFps1000;

    uint8_t     read(uint32_t len, uint8_t *where);
    uint8_t     read8(void);
    uint32_t    read16(void);
    uint32_t    read24(void);
    uint32_t    read32(void);
    uint8_t     Skip(uint32_t len);
    uint8_t     insertVideo(uint32_t pos,uint32_t size,uint32_t frameType,uint32_t dts,uint32_t pts);
    uint8_t     insertAudio(uint32_t pos,uint32_t size,uint32_t pts);
    uint8_t     setAudioHeader(uint32_t format,uint32_t fq,uint32_t bps,uint32_t channels);
    uint8_t     setVideoHeader(uint8_t codec,uint32_t *remaining);
    bool        extraHeader(flvTrak *trk,uint32_t *remain,bool haveCts,int32_t *cts);
    
    uint8_t     getFrameSize (uint32_t frame, uint32_t * size);
    char        *readFlvString(void);
    uint8_t     parseMetaData(uint32_t remaining);
    void        setProperties(const char *name,float value);
    uint32_t    searchMinimum(void);
  public:


    virtual   void          Dump(void);

            flvHeader( void );
   virtual  ~flvHeader(  ) ;
// AVI io
    virtual uint8_t  open(const char *name);
    virtual uint8_t  close(void) ;
  //__________________________
  //  Info
  //__________________________

  //__________________________
  //  Audio
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
    virtual uint8_t  getFrame(uint32_t framenum,ADMCompressedImage *img);
    virtual uint64_t getTime(uint32_t frame);
            uint8_t  getExtraHeaderData(uint32_t *len, uint8_t **data);
    virtual uint64_t getVideoDuration(void);

virtual   bool                    getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool                    setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);


};
#endif



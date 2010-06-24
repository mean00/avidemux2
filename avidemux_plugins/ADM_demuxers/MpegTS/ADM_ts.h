/***************************************************************************
                         ADM_PT
                             -------------------
    
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
 


#ifndef ADM_TS_H
#define ADM_TS_H

#include "ADM_Video.h"
#include "ADM_audioStream.h"
#include "dmx_io.h"
#include "ADM_indexFile.h"
#include "dmxTSPacket.h"
#include <vector>
#include "ADM_coreDemuxerMpeg.h"

using std::vector;

/**
    \fn ADM_psAccess
*/
class ADM_tsAccess : public ADM_audioAccess
{
protected:
                TS_PESpacket    *packet;
                tsPacket        demuxer;
                uint32_t         pid;
                uint64_t        dtsOffset;
                uint32_t        videoCodec;
                
public:
                bool            setTimeOffset(uint64_t of) {dtsOffset=of;return true;}
                vector          <ADM_mpgAudioSeekPoint >seekPoints;
                                  ADM_tsAccess(const char *name,uint32_t pid,bool append); 
                virtual           ~ADM_tsAccess();
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
                virtual bool      goToTime(uint64_t timeUs) ;
                                    /// Get a packet
                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);

                        bool      push(uint64_t at, uint64_t dts,uint32_t size);
                                    /// Convert raw timestamp to scaled timestamp in us
                uint64_t          timeConvert(uint64_t x);

                virtual uint32_t  getLength(void);

};
/**
    \class ADM_psTrackDescriptor
*/
class ADM_tsTrackDescriptor
{
public:
        ADM_audioStream *stream;
        ADM_tsAccess    *access;
        WAVHeader       header;
        ADM_tsTrackDescriptor()
            {
                stream=NULL;
                access=NULL;
            }
        ~ADM_tsTrackDescriptor()
            {
                if(stream) delete stream;
                stream=NULL;
                if(access) delete access;
                access=NULL;
            }


};

/**
    \Class psHeader
    \brief mpeg ps demuxer

*/
class tsHeader         :public vidHeader
{
  protected:
    
    bool    interlaced;
    bool    readVideo(indexFile *index);
    bool    readAudio(indexFile *index,const char *name);
    bool    readIndex(indexFile *index);

    bool    processVideoIndex(char *buffer);
    bool    processAudioIndex(char *buffer);

    std::vector <dmxFrame *> ListOfFrames;      
    fileParser      parser;
    uint32_t       lastFrame;
    tsPacketLinear *tsPacket;
    uint64_t        timeConvert(uint64_t x);
    bool            updatePtsDts(void);
    bool            updateIdr(void);
protected:
    vector <ADM_tsTrackDescriptor *>listOfAudioTracks;
    uint32_t        videoPid;             
  public:


    virtual   void          Dump(void);

            tsHeader( void ) ;
   virtual  ~tsHeader(  ) ;
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
//            uint8_t  getExtraHeaderData(uint32_t *len, uint8_t **data);
    virtual uint64_t getVideoDuration(void);
    virtual uint8_t  getFrameSize(uint32_t frame,uint32_t *size) ;
virtual   bool       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);

};
#endif



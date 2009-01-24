/***************************************************************************
                         ADM_AMV
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
 


#ifndef ADM_AMV_H
#define ADM_AMV_H

#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"


typedef struct 
{
    uint32_t pos;
    uint32_t size;
    uint32_t flags;
}amvIndex;

class amvTrack 
{
  public:
    amvIndex *index;
    uint32_t nbIndex;
    uint32_t indexRoof;
};


//**********************************************


class amvAudio : public AVDMGenericAudioStream
{
  protected:
    FILE                        *_fd;
    amvTrack                     *_track;
    uint32_t                      curIndex;
    uint32_t                      curOffset;
  public:
                                amvAudio(const char *name,amvTrack *track,WAVHeader *hdr);
                                
                                
    virtual                     ~amvAudio();
    virtual uint32_t            read(uint32_t len,uint8_t *buffer);
    virtual uint8_t             goTo(uint32_t newoffset);
   // virtual uint8_t             getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples);
    virtual uint8_t             extraData(uint32_t *l,uint8_t **d);
            
};

//*****************************************************
class amvHeader         :public vidHeader
{
  protected:
                                
    FILE                    *_fd;
    amvAudio                *_audio;
    char                    *_filename;
    amvTrack                 videoTrack;
    amvTrack                 audioTrack;
    
    WAVHeader               wavHeader;
    uint8_t                 changeAudioStream(uint32_t newstream);
    uint32_t                getCurrentAudioStreamNumber(void);
    uint8_t                 getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos);
    //flvAudio                *_audioStream;
    /* */
    
    uint8_t     read(uint32_t len, uint8_t *where);
    uint8_t     read8(void);
    uint32_t    read16(void);
    uint32_t    read24(void);
    uint32_t    read32(void);
    uint8_t     Skip(uint32_t len);
    uint8_t     insertVideo(uint32_t pos,uint32_t size,uint32_t frameType,uint32_t pts);
    uint8_t     insertAudio(uint32_t pos,uint32_t size,uint32_t pts);
    uint8_t     setAudioHeader(uint32_t format,uint32_t fq,uint32_t bps,uint32_t channels);
    uint8_t     setVideoHeader(uint8_t codec,uint32_t *remaining);
    
    
    uint8_t     getFrameSize (uint32_t frame, uint32_t * size);
    //
    uint8_t     readHeader(void);
    uint8_t     readTrack(int nb);
    uint8_t     index(void);
  public:


    virtual   void          Dump(void);

            amvHeader( void );
   virtual  ~amvHeader(  ) ;
// AVI io
    virtual uint8_t  open(const char *name);
    virtual uint8_t  close(void) ;
  //__________________________
  //  Info
  //__________________________

  //__________________________
  //  Audio
  //__________________________

    virtual   WAVHeader *getAudioInfo(void ) ;
    virtual uint8_t getAudioStream(AVDMGenericAudioStream **audio);


// Frames
  //__________________________
  //  video
  //__________________________

    virtual uint8_t  setFlag(uint32_t frame,uint32_t flags);
    virtual uint32_t getFlags(uint32_t frame,uint32_t *flags);
    virtual uint8_t  getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img);

            uint8_t  getExtraHeaderData(uint32_t *len, uint8_t **data);

};
#endif



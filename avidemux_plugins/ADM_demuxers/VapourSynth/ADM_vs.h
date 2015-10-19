/***************************************************************************
                         ADM_vs
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
#pragma once 

extern "C"
{
#define oldcplusplus __cplusplus
#undef __cplusplus
#include "VSScript.h"
#include "VSHelper.h"
#define __cplusplus oldcplusplus
}
#include "ADM_Video.h"
#include "ADM_audioStream.h"

/**
    \Class vsHeader
    \brief Flash demuxer

*/
class vsHeader         :public vidHeader
{
  protected:
                                
    FILE              *_fd;
    char              *_filename;
public:


    virtual void     Dump(void);

                     vsHeader( void );
    virtual          ~vsHeader(  ) ;
// AVI io
    virtual uint8_t  open(const char *name);
    virtual uint8_t  close(void) ;
  //__________________________
  //  Info
  //__________________________

  //__________________________
  //  Audio
  //__________________________
virtual     WAVHeader *getAudioInfo(uint32_t i )  ;
virtual     uint8_t    getAudioStream(uint32_t i,ADM_audioStream  **audio);
virtual     uint8_t    getNbAudioStreams(void);
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

virtual   bool       getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
virtual   bool       setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts);
          uint8_t    getFrameSize (uint32_t frame, uint32_t * size);

protected:
    VSScript       *_script;
    int            _outputIndex;
    VSNodeRef       *_node;
    int             _nbFrames;
    uint64_t        getTimeForFrame(int frame);
};



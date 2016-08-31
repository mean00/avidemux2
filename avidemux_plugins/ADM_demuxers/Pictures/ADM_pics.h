/***************************************************************************
                          ADM_pics.h  -  description
                             -------------------
    
    copyright            : (C) 2002-2008 by mean/gruntster
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
#include <vector>
#include "ADM_Video.h"
#include "ADM_audioStream.h"
#include "ADM_imageLoader.h"

/**
    \class picHeader
    \brief Demuxers for images (PNG/BMP/...)

*/
class picHeader:public vidHeader
{
protected:
    int         _nbFiles;
    std::string _filePrefix;    
    int         _first;
    uint32_t    _w, _h;
    int         _bmpHeaderOffset;
    std::vector<uint32_t>    _imgSize;
    ADM_PICTURE_TYPE    _type;
    uint32_t    read32 (FILE * fd);
    uint16_t    read16 (FILE * fd);
    uint8_t     read8 (FILE * fd);
    FILE        *openFrameFile (uint32_t frameNum);

public:
    virtual void Dump (void)
  {
  };

                picHeader (void);
                ~picHeader ()
                {
                };
// AVI io
  virtual uint8_t open (const char *name);
  virtual uint8_t close (void);
  
  virtual WAVHeader *getAudioInfo (uint32_t i)
  {
    return NULL;
  }
  virtual uint8_t getAudioStream (uint32_t i, ADM_audioStream ** audio)
  {
    *audio = NULL;
    return 1;
  }
  virtual uint8_t getNbAudioStreams (void)
  {
    return 0;
  }

  virtual uint8_t   setFlag (uint32_t frame, uint32_t flags);
  virtual uint32_t  getFlags (uint32_t frame, uint32_t * flags);
  virtual uint8_t   getFrame (uint32_t framenum, ADMCompressedImage *);

  virtual uint64_t  getTime (uint32_t frameNum);
  virtual uint64_t  getVideoDuration (void);
  virtual uint8_t   getFrameSize (uint32_t frame, uint32_t * size);

  virtual bool      getPtsDts (uint32_t frame, uint64_t * pts, uint64_t * dts);
  virtual bool      setPtsDts (uint32_t frame, uint64_t pts, uint64_t dts);


};

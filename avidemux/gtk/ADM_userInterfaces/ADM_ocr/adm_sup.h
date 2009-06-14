//
// C++ Implementation: Interface to decode .SUP (projectX) subtitles
// Author: Mean, fixounet@free.fr
// (C) 2007 MEAN
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADMVideoSupSub_H
#define ADMVideoSupSub_H

typedef struct
{
  uint32_t pos;
  uint32_t size;
  uint32_t pts;
}SUPIndex;

class SUPIndexArray
{
public:
  SUPIndexArray(uint32_t nb);
  ~SUPIndexArray();
  
  SUPIndex *index;
  uint32_t nbIndex;
  uint32_t indexCeil;
  
  uint32_t insert(uint32_t pos, uint32_t size, uint32_t pts);
};

class ADMVideoSupSub
{
  protected:
      FILE      *_fd;
      uint8_t   parse(void);
      uint8_t   decode(uint32_t size, uint8_t *data);
      uint8_t   decodeRLE(uint32_t off,uint32_t start,uint32_t end,uint8_t *data,uint32_t _dataSize);
      uint8_t   setPalette( uint32_t *palette );
      uint8_t   buildYUV( int16_t *palette ); 
      
      
      vobSubBitmap *bitmap;
      uint32_t currentImage;
      
      uint32_t _colors[4];
      uint32_t _subW,_subH,_alpha[4];
      int16_t  _YUVPalette[16];
  public:
      ADMVideoSupSub(char *supfile,uint32_t ix);
      ~ADMVideoSupSub();
      SUPIndexArray *index;
      uint32_t getNbImage(void);
      vobSubBitmap *getBitmap(
          uint32_t nb,uint32_t *start, uint32_t *end,uint32_t *first,uint32_t *last);
  
};

#endif
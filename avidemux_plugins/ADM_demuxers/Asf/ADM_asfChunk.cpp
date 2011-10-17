/***************************************************************************
    copyright            : (C) 2006 by mean
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

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_assert.h"

#include "fourcc.h"


#include "ADM_asf.h"

// http://www.thozie.de/dnn/AVIMaster.aspx?PageContentID=4

static const chunky mychunks[]=
{
  {"Header Chunk",0,    {0x30,0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c},ADM_CHUNK_HEADER_CHUNK},
  {"File Header",0,  {0xa1,0xdc,0xab,0x8c,0x47,0xa9,0xcf,0x11,0x8e,0xe4,0x00,0xc0,0x0c,0x20,0x53,0x65},ADM_CHUNK_FILE_HEADER_CHUNK},
  {"No audio conceal",0,
  {0x40,0x52,0xd1,0x86,0x1d,0x31,0xd0,0x11,0xa3,0xa4,0x00,0xa0,0xc9,0x03,0x48,0xf6},ADM_CHUNK_NO_AUDIO_CONCEAL},
  {"Stream Header",0,   {0x91,0x07,0xdc,0xb7,0xb7,0xa9,0xcf,0x11,0x8e,0xe6,0x00,0xc0,0x0c,0x20,0x53,0x65},ADM_CHUNK_STREAM_HEADER_CHUNK},
  {"Stream Group Id",0, {0xce,0x75,0xf8,0x7b,0x8d,0x46,0xd1,0x11,0x8d,0x82,0x00,0x60,0x97,0xc9,0xa2,0xb2},ADM_CHUNK_STREAM_GROUP_ID},
  {"Data Chunk",0,      {0x36,0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c},ADM_CHUNK_DATA_CHUNK},
  {"Header Extension",0,
    {0Xb5,0x03,0xbf,0x5f,0x2e,0xa9,0xcf,0x11,0x8e,0xe3,0x00,0xc0,0x0c,0x20,0x53,0x65},
              ADM_CHUNK_HEADER_EXTENSION_CHUNK},
  {"Clock Type  Ext",0,
  {0x11,0xd2,0xd3,0xab,0xba,0xa9,0xcf,0x11,0x8e,0xe6,0x00,0xc0,0x0c,0x20,0x53,0x65},ADM_CHUNK_CLOCK_TYPE_EX},
  {"Language List Ext",0,
  {0xa9,0x46,0x43,0x7c,0xe0,0xef,0xfc,0x4b,0xb2,0x29,0x39,0x3e,0xde,0x41,0x5c,0x85},ADM_CHUNK_LANGUAGE_LIST_EX},
  {"Compatibility List Ex",0,
  {0x5d,0x8b,0xf1,0x26,0x84,0x45,0xec,0x47,0x9f,0x5f,0x0e,0x65,0x1f,0x04,0x52,0xc9},ADM_CHUNK_UNKNOWN_CHUNK},
  {"Padding",0,
  {0x74,0xd4,0x06,0x18,0xdf,0xca,0x09,0x45,0xa4,0xba,0x9a,0xab,0xcb,0x96,0xaa,0xe8},ADM_CHUNK_UNKNOWN_CHUNK},
  {"Padding2",0,
  {0x94,0x1c,0x23,0x44,0x98,0x94,0xd1,0x49,0xa1,0x41,0x1d,0x13,0x4e,0x45,0x70,0x54},ADM_CHUNK_UNKNOWN_CHUNK},

  {"Extended Stream Property",0,
  {0xcb,0xa5,0xe6,0x14,0x72,0xc6,0x32,0x43,0x83,0x99,0xa9,0x69,0x52,0x06,0x5b,0x5a},ADM_CHUNK_EXTENDED_STREAM_PROP},
  {"MetaData Object",0,
  {0xea,0xcb,0xf8,0xc5,0xaf,0x5b,0x77,0x48,0x84,0x67,0xaa,0x8c,0x44,0xfa,0x4c,0xca},ADM_CHUNK_UNKNOWN_CHUNK},
  {"Content Desc",0,
  {0x33,0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c},ADM_CHUNK_CONTENT_DESC},
  {"Extended Content Desc",0,
  {0x40,0xa4,0xd0,0xd2,0x07,0xe3,0xd2,0x11,0x97,0xf0,0x00,0xa0,0xc9,0x5e,0xa8,0x50},ADM_CHUNK_EXT_CONTENT_DESC},
  {"zz",0,{0x30,0x26,0xb2,0x75,0x8e,0x66,0xcf,0x11,0xa6,0xd9,0x00,0xaa,0x00,0x62,0xce,0x6c},ADM_CHUNK_HEADER_CHUNK}
  
};
static const chunky nochunk=
{"Unknown",0,{0x10,0x20,0x30,0x40,0xde,0xad,0xde,0xad,0xbe,0xef,0xbe,0xef,0x00,0x62,0xce,0x6c},ADM_CHUNK_UNKNOWN_CHUNK};


 asfChunk::asfChunk(FILE *f)
{
  _fd=f;
  _chunkStart=ftello(f);;
  printf("Chunk created at %x\n",_chunkStart);
  ADM_assert(_fd);
  chunkLen=0;
}
 asfChunk::~asfChunk()
{
}
uint8_t   asfChunk::readChunkPayload(uint8_t *data, uint32_t *dataLen)
{
  uint32_t remaining;
  
  remaining=ftello(_fd);
  remaining-=_chunkStart;
  remaining=chunkLen-remaining;
  fread(data,remaining,1,_fd);
  *dataLen=remaining;
  return 1;
}
uint8_t   asfChunk::skip(uint32_t skip)
{
  fseeko(_fd,skip,SEEK_CUR);
  return 1; 
}
uint8_t   asfChunk::nextChunk(int shortChunk)
{
  uint32_t low,high;
  
  if(_chunkStart)
  {
    
    fseeko(_fd,_chunkStart+ chunkLen,SEEK_SET);
  }
  
  _chunkStart=ftello(_fd);
  fread(guId,16,1,_fd);
  if(shortChunk)
  {
    low=read16()+16;
    high=0;
  } 
  else
  {
    low=read32();
    high=read32();
  }
  chunkLen=high;
  chunkLen<<=32;
  chunkLen+=low;
  
  printf("Next chunk from %"LX" +%"LLX" to %"LLX"\n",_chunkStart,chunkLen,chunkLen+_chunkStart);
  
  return 1;
  
}
uint8_t   asfChunk::skipChunk(void)
{
  uint32_t go;
  go=_chunkStart+ chunkLen;
  printf("Pos 0x%"LLX"\n",ftello(_fd));
  fseeko(_fd,go,SEEK_SET);
  printf("Skipping to 0x%"LX"\n",go);
  
  return 1; 
}
uint64_t  asfChunk::read64(void)
{
  uint64_t lo,hi;
  lo=read32();
  hi=read32();
  return lo+(hi<<32); 
  
}
uint32_t   asfChunk::read32(void)
{
  uint8_t c[4];
  
  fread(c,4,1,_fd);
  
  return c[0]+(c[1]<<8)+(c[2]<<16)+(c[3]<<24);
  
}
uint32_t   asfChunk::read16(void)
{
  uint8_t c[2];
  
  fread(c,2,1,_fd);
  
  return c[0]+(c[1]<<8);
  
}

uint8_t   asfChunk::read8(void)
{
  uint8_t c[1];
  
  fread(c,1,1,_fd);
  
  return c[0];
  
}
uint8_t   asfChunk::read(uint8_t *where, uint32_t how)
{
 
  if(1!=fread(where,how,1,_fd))
  {
    printf("[AsfChunk] Read error\n");
    return 0; 
  }
  return 1;

  
}
uint8_t   asfChunk::dump(void)
{
  const chunky *id;
  id=chunkId();
  printf("Chunk type  : <<<<%s>>>>\n",id->name);
  printf("Chunk Start : %"LX"\n",_chunkStart);
  printf("Chunk Len   : %"LU"\n",(uint32_t)chunkLen);
  printf("%02x%02x%02x%02x-%02x%02x-xxxx",guId[3],guId[2],guId[1],guId[0],guId[5],guId[4]);
  for(int i=0;i<16;i++) printf("%02x ",guId[i]);
  printf("\n");
  return 1;
  
}
const chunky *asfChunk::chunkId(void)
{
  int mx=sizeof(mychunks)/sizeof(chunky);
  for(int i=0;i<sizeof(mychunks)/sizeof(chunky);i++)
  {
    if(!memcmp(mychunks[i].val,guId,16)) return &mychunks[i];
  }
  return &nochunk;
  
}


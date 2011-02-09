/***************************************************************************
                          riffWritter.cpp  -  description
                             -------------------
    
    copyright            : (C) 2011 by mean
    email                : fixounet@free.fr

	This class deals with RIFF files

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
#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"
#include "ADM_writeRiff.h"


/**
    \fn ctor
*/

riffWritter::riffWritter(const char *nameFcc, FILE * f)
{
    _fcc = fourCC::get((uint8_t *) nameFcc);
    ADM_assert(_fcc);
    _ff = f;
    ADM_assert(_ff);
    _begin=_end=0;
}
/**
    \fn begin
    \brief Mark the beginning of a subchunk
*/
bool riffWritter::begin(const char *subchunk)
{
    
    _begin=tell();
    write32((_fcc));
    write32((uint32_t) 0);	// size
    write32((fourCC::get((uint8_t *) subchunk)));
    return 1;
}

/**
    \fn end
    \brief end a subchunk
*/
bool riffWritter::end(void)
{
	uint64_t len, b, e;
    e=tell();
    fseeko(_ff,_begin,SEEK_SET);
    b=tell();
	len=e-b-8;	// is this causing trouble? 'list' content has to include any padding
    write32((_fcc));
    write32((len));
    fseeko(_ff,e,SEEK_SET);
    return 1;
}
/**
    \fn tellBegin
    \brief returns the beginning position of current subchunk
*/
uint64_t riffWritter::tellBegin(void)
{
    return _begin;

}

/**
    \fn tell
    \brief Where are we in the file
*/
uint64_t riffWritter::tell(void)
{
	return ftello(_ff);
}

/**
    \fn write
    \brief binary write
*/
bool riffWritter::write(uint8_t * p, uint32_t len)
{
        return fwrite(p,len,1,_ff);
}
/**
    \fn write32
    \brief write fourcc
*/
bool riffWritter::write32(uint8_t * c)
{
    uint32_t fcc;
    fcc = fourCC::get(c);
    ADM_assert(fcc);
    write32(fcc);
    return 1;
}
/**
    \fn writeChunk
    \brief write a whole riff chunk
*/
bool riffWritter::writeChunk(uint8_t * chunkid, uint32_t len, uint8_t * p)
{
    uint32_t fcc;

    fcc = fourCC::get(chunkid);
    ADM_assert(fcc);
    write32(fcc);
    write32(len);
    write(p, len);
    if (len & 1)
    {				// pad to be a multiple of 4, nicer ...
	  write(p, 1);
    }
    return 1;
}
/**
    \fn write64
    \brief little endian
*/
void riffWritter::write64(uint64_t val)
{
    ADM_assert(0);
}
/**
    \fn write16
    \brief little endian
*/

void riffWritter::write16(uint16_t val)
{
    uint8_t p[2];
        p[0]=val&0xff;
        p[1]=val>>8;
        write(p,2);
}
/**
    \fn write8
*/

void riffWritter::write8(uint8_t val)
{
        write((uint8_t *)&val,1);
}
/**
    \fn write32
    \brief little endian
*/

bool riffWritter::write32(uint32_t val)
{
  uint8_t p[4];
        p[0]=val&0xff;
        p[1]=val>>8;
        p[2]=val>>16;
        p[3]=val>>24;
        write(p,4);
        return 1;
}
/**
    \fn writeWavHeader
*/
bool riffWritter::writeWavHeader(const char *tag,WAVHeader *hdr)
{
uint32_t fcc;
    fcc = fourCC::get((uint8_t *)tag);
    ADM_assert(fcc);
    write32(fcc);
    write32(sizeof(*hdr));
#define w16(x) write16(hdr->x)
#define w32(x) write32(hdr->x)
    
    w16(	encoding);	
    w16(	channels);					/* 1 = mono, 2 = stereo */
    w32(	frequency);				/* One of 11025, 22050, or 44100 48000 Hz */
    w32(	byterate);					/* Average bytes per second */
    w16(	blockalign);				/* Bytes per sample block */
    w16(	bitspersample);		/* One of 8, 12, 16, or 4 for ADPCM */
    return true;
}
// EOF
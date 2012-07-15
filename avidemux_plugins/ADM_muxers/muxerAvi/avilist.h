
 /***************************************************************************
                          avilist.h  -  description
                             -------------------
    begin                : Thu Nov 15 2001
    copyright            : (C) 2001 by mean, 2005 (C) GMV
    email                : fixounet@free.fr

Deals with LIST in RIFF structured file
Especially AVI in our case
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __AVILIST__
#define __AVILIST__
#include "ADM_fileio.h"
/**
    \class AviList
    \brief small helper class to write tag/len/value chunks in avi
*/
class AviList
{
protected:
		ADMFile		*_ff;
		uint32_t _fcc;
		uint64_t _begin,_end;

public:
                     AviList(const char *name,ADMFile *ff);

        void        Write64(uint64_t val);
        void        Write16(uint16_t val);
        void        Write8(uint8_t val);
        uint8_t     Write32(uint32_t val) ;
        uint8_t     Write32(uint8_t *c);
        uint8_t     Write32(const char  *c) {Write32((uint8_t *)c);return 1;};
        uint8_t     Write(const uint8_t *p,uint32_t len);
        uint8_t     WriteChunk(uint8_t *chunkid,uint32_t len,const uint8_t *p);
        uint8_t     WriteChunk(uint32_t fcc,uint32_t len,const uint8_t *p);
        uint64_t    Tell(void );    		// glue for index.... ugly
        uint64_t    TellBegin(void );   // same story here
        uint8_t	 Begin( const char *subchunk );
        uint8_t	 Begin( void );
        uint8_t     Seek(uint64_t to);
        uint8_t	 End (void );
        ADMFile      *getFile() { return _ff;}
};
#include "avi_utils.h"
#endif

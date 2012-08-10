/***************************************************************************
                          ADM_writeRiff.h.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#ifndef ADM_WRITE_RIFF_H
#define ADM_WRITE_RIFF_H

#include "ADM_coreUtils6_export.h"

/**
    \class riffWritter
    \brief helper class to write riff files (Wav,Avi,...)
*/
class ADM_COREUTILS6_EXPORT riffWritter
{
protected:
		FILE    *_ff;
		uint32_t _fcc;
		uint64_t _begin,_end;

public:
        void write64(uint64_t val);
        void write16(uint16_t val);
        void write8(uint8_t val);
        bool write32(uint32_t val) ;
        bool write32(uint8_t *c);
        bool write32(const char  *c) {write32((uint8_t *)c);return 1;};
        bool write(uint8_t *p,uint32_t len);
        bool writeChunk(uint8_t *chunkid,uint32_t len,uint8_t *p);


				  riffWritter(const char *fcc,FILE *ff);
        uint64_t  tell(void ); 		// glue for index.... ugly
        uint64_t  tellBegin(void );   // same story here

        bool    begin( const char *subchunkfcc );
        bool    end (void );

        bool writeWavHeader(const char *tag,WAVHeader *hdr);

};

#endif

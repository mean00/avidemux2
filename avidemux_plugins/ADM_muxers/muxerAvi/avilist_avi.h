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
#ifndef __AVILIST_AVI__
#define __AVILIST_AVI__
#include "avilist.h"
#include "ADM_memio.h"
/**
    \class AviList
    \brief small helper class to write tag/len/value chunks in avi
*/
class AviListAvi: public AviList
{
protected:  

public:
        AviListAvi(const char *name,ADMFile *ff) : AviList(name,ff)
        {

        }
        bool    writeMainHeaderStruct(const MainAVIHeader &hdr);
        bool    writeStreamHeaderStruct(const AVIStreamHeader &hdr);

        bool  writeStrh(const AVIStreamHeader &hdr);
        bool  writeStrfBih(const ADM_BITMAPINFOHEADER &hdr, int extraLen, uint8_t *extraData);
        bool  writeStrfWav(const WAVHeader &hdr, int extraLen, uint8_t *extraData);
        bool  WriteMem(const ADMMemio &meme);        
        bool  writeDummyChunk(int size, uint64_t *pos);
        
};
#include "avi_utils.h"
#endif

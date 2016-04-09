/**
    \file  ADM_audioWrite
    \brief Writer

    copyright            : (C) 2011 by mean
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
#include "ADM_audioStream.h"
#include "ADM_audioWrite.h"
#include "ADM_audioWriteWav.h"
#include "ADM_audioWriteAAC.h"
/**

*/
ADM_audioWrite::ADM_audioWrite()
{
    _file=NULL;
}
/**
    \fn dtor
*/
ADM_audioWrite::~ADM_audioWrite()
{
    close();
}
/**

*/
bool ADM_audioWrite::close(void)
{
    if(_file) fclose(_file);
    _file=NULL;
    return true;
}
/**

*/
bool ADM_audioWrite::init(ADM_audioStream *stream, const char *fileName)
{
    _file=fopen(fileName,"wb");
    if(!_file) return false;
    return true;
}
/**

*/
bool ADM_audioWrite::write(uint32_t size, uint8_t *buffer)
{
      fwrite(buffer,size,1,_file);
      return true;
}
/**
    \fn admCreateAudioWriter
    \brief Spawn the right writter
*/
ADM_audioWrite *admCreateAudioWriter(ADM_audioStream *stream)
{
    WAVHeader *hdr=stream->getInfo();
    switch(hdr->encoding)
    {
        case WAV_PCM: return new ADM_audioWriteWav();break;
        case WAV_AAC: return new ADM_audioWriteAAC();break;
        default: return new ADM_audioWrite();break;
    }
}
// EOF
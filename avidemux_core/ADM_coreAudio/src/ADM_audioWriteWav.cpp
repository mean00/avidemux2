/**
    \file  ADM_audioWriteWav
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
#include "ADM_audioWriteWav.h"
/**
    \fn ctor
*/
ADM_audioWriteWav::ADM_audioWriteWav()
{
    writter=NULL;
    dataPosition=0;
}
/**
    \fn writeHeader
*/
bool ADM_audioWriteWav::writeHeader(ADM_audioStream *stream)
{
          writter = new riffWritter("RIFF", _file);
          writter->begin("WAVE");
          // Write wavheader...
          WAVHeader wh,*p;
          p=stream->getInfo();
          wh.encoding=WAV_PCM;
          wh.channels=p->channels;
          wh.blockalign=p->channels*2;
          wh.byterate=p->channels*p->frequency*2;
          wh.frequency=p->frequency;
          wh.bitspersample=16;

          writter->writeWavHeader("fmt ",&wh);
          writter->write32("data");
          dataPosition=writter->tell();
          writter->write32( (uint32_t )0);
          return true;
}
/**
    \fn updateHeader
*/

bool ADM_audioWriteWav::updateHeader(void)
{
        uint64_t theEnd=ftello(_file);
        fseeko(_file,dataPosition,SEEK_SET);
        writter->write32((uint32_t)(theEnd-dataPosition));
        return true;
}


/**
    \fn close
*/

bool ADM_audioWriteWav::close(void)
{
    if(_file)
    {
        updateHeader();
    }
    if(writter)
    {
        writter->end();
        delete writter;
        writter=NULL;
    }
    return ADM_audioWrite::close();
}
/**
    \fn init
    \brief write wavHeader
*/

bool ADM_audioWriteWav::init(ADM_audioStream *stream, const char *fileName)
{
    if(false==ADM_audioWrite::init(stream,fileName)) return false;
    return writeHeader(stream);
}
//EOF

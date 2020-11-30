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
    channels=0;
    bytesPerSample=0;
    swapBytes=false;
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
          if(!p->channels || p->channels > MAX_CHANNELS)
          {
              ADM_error("Invalid # of channels %u\n",p->channels);
              return false;
          }
          if(!p->bitspersample || (p->bitspersample & 7))
          {
              ADM_error("Invalid # of bits per sample %u\n",p->bitspersample);
              return false;
          }
          wh.encoding=WAV_PCM;
          wh.channels=p->channels;
          wh.blockalign=p->channels*p->bitspersample;
          wh.byterate=p->channels*p->frequency*p->bitspersample;
          wh.frequency=p->frequency;
          wh.bitspersample=p->bitspersample;

          channels = p->channels;
          bytesPerSample = p->bitspersample >> 3;
          swapBytes = p->encoding == WAV_LPCM;

          writter->writeWavHeader("fmt ",&wh);
          writter->write32("data");
          dataPosition=writter->tell();
          writter->write32( (uint32_t )0);
          return true;
}

/**
    \fn write
*/
bool ADM_audioWriteWav::write(uint32_t size, uint8_t *buffer)
{
    if(swapBytes)
    {
        if(size % (bytesPerSample * channels) || size < bytesPerSample * channels)
        {
            ADM_warning("Block not aligned, skipping.\n");
            return false; //FIXME: should be fatal
        }
        uint8_t *swapped = new uint8_t[size];
        uint8_t *src = buffer;
        uint8_t *dst = swapped;
        uint32_t remaining = size;
        while(remaining)
        {
            uint32_t off = bytesPerSample;
            while(off--)
            {
                uint8_t *s = src;
                *(dst++) = *(s + off);
            }
            src += bytesPerSample;
            remaining -= bytesPerSample;
        }
        bool r = ADM_audioWrite::write(size,swapped);
        delete [] swapped;
        swapped = NULL;
        return r;
    }
    return ADM_audioWrite::write(size,buffer);
}

/**
    \fn updateHeader
*/

bool ADM_audioWriteWav::updateHeader(void)
{
        uint64_t theEnd=ftello(_file);
        fseeko(_file,dataPosition,SEEK_SET);
        theEnd -= dataPosition;
        theEnd -= theEnd % (channels * bytesPerSample); // complete samples only
        writter->write32((uint32_t)theEnd);
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

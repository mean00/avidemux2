/**
    \file  ADM_audioWriteAAC
    \brief Write AAC packets inside ADTS container
    copyright            : (C) 2016 by mean
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
#include "ADM_audioWriteAAC.h"
/**
    \fn ctor
*/
ADM_audioWriteAAC::ADM_audioWriteAAC()
{
    dataPosition=0;
}
/**
    \fn writeHeader
*/
bool ADM_audioWriteAAC::writeHeader(ADM_audioStream *stream)
{

          return true;
}


/**
    \fn close
*/

bool ADM_audioWriteAAC::close(void)
{
    return ADM_audioWrite::close();
}
/**
    \fn init
    \brief write wavHeader
*/

bool ADM_audioWriteAAC::init(ADM_audioStream *stream, const char *fileName)
{
    if(false==ADM_audioWrite::init(stream,fileName)) return false;
    return true;
}
//EOF

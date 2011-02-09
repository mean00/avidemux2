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
    \fn writeHeader
*/
bool ADM_audioWriteWav::writeHeader(ADM_audioStream *stream)
{

}
/**
    \fn updateHeader
*/

bool ADM_audioWriteWav::updateHeader(void)
{

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

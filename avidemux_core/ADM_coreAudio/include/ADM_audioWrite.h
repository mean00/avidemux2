/**
    \file  ADM_audioWrite
    \brief Writer

    copyright            : (C) 2002/2009 by mean
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
#pragma once
#include "ADM_coreAudio6_export.h"
/**
 * \class class ADM_audioWrite

 */
class ADM_audioWrite
{
protected:
    FILE *_file;

public:
                    ADM_audioWrite();
virtual             ~ADM_audioWrite();
virtual      bool   close(void);
virtual      bool   init(ADM_audioStream *stream, const char *fileName);
virtual      bool   write(uint32_t size, uint8_t *buffer);
virtual      bool   canBeBuffered() {return true;}; // if false, it is packet oriented, else it is stream oriented like PCM
};

ADM_COREAUDIO6_EXPORT ADM_audioWrite *admCreateAudioWriter(ADM_audioStream *stream);

// EOF
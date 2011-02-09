/**
    \file  ADM_audioWriteWav
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
#ifndef ADM_AUDIO_WRITE_WAV_H
#define ADM_AUDIO_WRITE_WAV_H
#include "ADM_audioWrite.h"
class ADM_audioWriteWav: public ADM_audioWrite
{
protected:
             bool writeHeader(ADM_audioStream *stream);
             bool updateHeader(void);

public:
virtual      bool close(void);
virtual      bool init(ADM_audioStream *stream, const char *fileName);
};


#endif
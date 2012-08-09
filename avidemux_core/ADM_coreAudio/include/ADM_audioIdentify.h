/**
    \file  ADM_audioIdentify
    \brief Identify a codec used in a file

    \author copyright            : (C) 2012 by mean
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIO_CORE_IDENTIFY_H
#define AUDIO_CORE_IDENTIFY_H

#include "ADM_coreAudio6_export.h"

ADM_COREAUDIO6_EXPORT bool ADM_identifyAudioStream(int bufferSize,const uint8_t *buffer,WAVHeader &info,uint32_t &offset);

#endif
// EOF

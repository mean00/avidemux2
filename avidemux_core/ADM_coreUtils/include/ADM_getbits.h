/**
        \file ADM_getbits.h
        \brief Wrapper around ffmpeg getbits function

*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_GETBITS_H
#define ADM_GETBITS_H

#include "ADM_coreUtils6_export.h"

class ADM_COREUTILS6_EXPORT getBits
{
protected:
         void *ctx;
public:
                getBits(const getBits &source);
                getBits(int bufferSizeInBytes, uint8_t *buffer);
                ~getBits();
            int get(int nb);
            int getUEG(void);
            int getSEG(void);
            int getUEG31(void);
            int skip(int nb);
            int getConsumedBits(void);
            void align(void);
};


#endif


/***************************************************************************
                          \fn ADM_bitstream.h
                          \brief Class to handle compressed data
                             -------------------
    
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
#ifndef ADM_BITSTREAM_H
#define ADM_BITSTREAM_H
#define ADM_NO_TIMING 0xffffffff

#include "ADM_coreUtils6_export.h"

/*
    BIG WARNING : BUFFER SIZE MUST BE SET: SOME CODECS CHECK& USE IT
    ESPECIALLY LAVCODEC!
*/
/**
    \class ADMBitstream

*/
class ADM_COREUTILS6_EXPORT ADMBitstream
{
    public:
        uint32_t len;                   // Size of the payload
        uint32_t bufferSize;            // Total size of the data field
        uint8_t *data;                  // Where to store data
        uint32_t flags;                 // frame type (I/P/B/...)
        uint32_t in_quantizer;          // Quantizer asked
        uint32_t out_quantizer;         // Quantizer of the image, in case of encoding the real Q
        uint64_t pts;			        // in us
        uint64_t dts;			        // in us

        ADMBitstream (uint32_t buffersize=0);
        ~ADMBitstream ();
        void cleanup (uint32_t dts);

};
#endif

/***************************************************************************
                          ADM_audioStream.h  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#ifndef ADM_audioStreamConstantChunk_H
#define ADM_audioStreamConstantChunk_H

#include "ADM_audioStream.h"


/**
        \fn ADM_audioStreamConstantChunk
        \brief Class to handle MP3/MP3 streams

*/
class ADM_audioStreamConstantChunk : public ADM_audioStream
{
        protected:
                        uint32_t  chunkSize;
                        uint32_t  samplesPerChunk;
        public:
/// Default constructor
                       ADM_audioStreamConstantChunk(WAVHeader *header,ADM_audioAccess *access);  
/// Destructor
virtual                 ~ADM_audioStreamConstantChunk();
///  Get a packet
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,
                                uint32_t *nbSample,uint64_t *dts);
};
#endif
// EOF


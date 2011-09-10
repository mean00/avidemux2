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
#ifndef ADM_audioStreamMP3_H
#define ADM_audioStreamMP3_H

#include "ADM_audioStreamBuffered.h"
#include <BVector.h>

typedef struct
{
    uint64_t offset;
    uint64_t timeStamp;
}MP3_seekPoint;

/**
        \fn ADM_audioStreamMP3
        \brief Class to handle MP3/MP3 streams

*/
class ADM_audioStreamMP3 : public ADM_audioStreamBuffered
{
        protected:
                      BVector <MP3_seekPoint *> seekPoints;
        bool            buildTimeMap(void);
        public:
/// Default constructor
                       ADM_audioStreamMP3(WAVHeader *header,ADM_audioAccess *access, bool createMap=true);  
/// Destructor
virtual                 ~ADM_audioStreamMP3();
///  Get a packet
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,
                                uint32_t *nbSample,uint64_t *dts);
/// Go to a given time, in microseconds
virtual bool            goToTime(uint64_t nbUs);
/// Returns or compute duration. If the access cannot provide it, it will be computed here
        uint64_t        getDurationInUs(void) {return durationInUs;}
};
#endif
// EOF


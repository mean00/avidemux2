/***************************************************************************
                          ADM_audioStreamAC3.h  -  description
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
#ifndef ADM_audioStreamEAC3_H
#define ADM_audioStreamEAC3_H

#include "ADM_audioStreamBuffered.h"


/**
        \fn ADM_audioStreamEAC3
        \brief Class to handle EAC3/A52B streams

*/
class ADM_audioStreamEAC3 : public ADM_audioStreamBuffered
{
        protected:
        public:
/// Default constructor
                       ADM_audioStreamEAC3(WAVHeader *header,ADM_audioAccess *access);  
/// Destructor
virtual                 ~ADM_audioStreamEAC3();
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


/***************************************************************************
                          ADM_audioStreamPCM.h  -  description
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
#pragma once
#include "ADM_audioStreamBuffered.h"


/**
        \fn ADM_audioStreamPCM
        \brief Class to handle OCL streams

*/
class ADM_audioStreamPCM : public ADM_audioStream
{
        protected:
        public:
/// Default constructor
                       ADM_audioStreamPCM(WAVHeader *header,ADM_audioAccess *access);  
/// Destructor
virtual                 ~ADM_audioStreamPCM();
///  Get a packet
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,
                                uint32_t *nbSample,uint64_t *dts);
/// Go to a given time, in microseconds
virtual bool            goToTime(uint64_t nbUs);
/// Returns or compute duration. If the access cannot provide it, it will be computed here
        uint64_t        getDurationInUs(void) {return durationInUs;}
};


/**
        \fn ADM_audioStreamFloatPCM
        \brief Class to handle float PCM streams

*/
class ADM_audioStreamFloatPCM : public ADM_audioStreamPCM
{
public:
                   ADM_audioStreamFloatPCM(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStreamPCM(header,access)
                   {
                       
                   }
///  Get a packet
virtual uint8_t    getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,
                                uint32_t *nbSample,uint64_t *dts);
};

// EOF


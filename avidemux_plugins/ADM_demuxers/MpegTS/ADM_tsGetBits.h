/***************************************************************************
    \file ADM_tsGetBits
    \brief Simple getbits dedicated to TS packet. should be made more generic.
    \author mean (C) 2010 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_TS_GETBITS
#define ADM_TS_GETBITS
#include "dmxTSPacket.h"
#define MAX_GETBITS_BUFFER 64
/**
    \class tsGetBits
    
*/
class tsGetBits
{
protected:
    bool           refill(void);
    tsPacketLinear *pkt;
    int            consumed;
    int            stored; /// Nb bits stored in accumulator
    int            accumulator; /// bits storage 31 max!
public:
    uint8_t  data[MAX_GETBITS_BUFFER];
             tsGetBits(tsPacketLinear *p);
             ~tsGetBits();
    int      getConsumed(void) {return consumed;}
    uint32_t getBits(int n);
    uint32_t peekBits(int n);
};

#endif
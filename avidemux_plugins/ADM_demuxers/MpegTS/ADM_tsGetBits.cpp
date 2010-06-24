/***************************************************************************
    \file ADM_tsGetBits
    \brief Simple getbits dedicated to TS packet. should be made more generic.
    \author mean (C) 2010 fixounet@free.fr

    This is slow, but we dont care as it is not used much.
    The aim is to consume exactly what we need from the pkt.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_tsGetBits.h"
/**
    \fn ctor
*/
tsGetBits::tsGetBits(tsPacketLinear *p)
{
    pkt=p;
    consumed=0;
    accumulator=0;
    stored=0;
}
/**
    \fn dtor
*/
tsGetBits::~tsGetBits()
{


}
bool tsGetBits::refill(void)
{
    uint32_t nw;
    int shift=32-stored;
    accumulator>>=shift;
    accumulator<<=shift;
    
    nw=pkt->readi8();
    ADM_assert(consumed<MAX_GETBITS_BUFFER);
    data[consumed]=nw;
    nw<<=24-stored;
    accumulator+=nw;

    consumed++;
    stored+=8;
    return true;
}
/**
    \fn getBits
*/
uint32_t tsGetBits::getBits(int n)
{
    ADM_assert(n);
    if(n>23) ADM_assert(0);
again:
    if(n<=stored)
    {
        uint32_t out=accumulator;
        int shift=32-n;
        out>>=shift;
        accumulator<<=n;
        stored-=n;
        out&=(1<<n)-1;
        return out;
    }
    // make room
    refill();
    goto again;
}

/**
    \fn getBits
*/
uint32_t tsGetBits::peekBits(int n)
{
    ADM_assert(n);
    if(n>31) ADM_assert(0);
again:
    if(n<=stored)
    {
        uint32_t out=accumulator;
        int shift=32-n;
        out>>=shift;
        out&=(1<<n)-1;
        return out;
    }
    // make room
    refill();
    goto again;
}
//EOF

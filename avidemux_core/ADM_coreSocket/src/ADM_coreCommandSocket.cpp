/***************************************************************************
    \file ADM_coreCommandSocket.cpp
    \brief Handle socket network part 
    \author (C) 2007-2010 by mean  fixounet@free.fr

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
#include "ADM_coreCommandSocket.h"

/**
    \fn ctor
*/
ADM_commandSocket::ADM_commandSocket()  : ADM_socket()
{
    mySocket=0;
}
/**
    \fn ctor
*/
ADM_commandSocket::ADM_commandSocket(int newSocket) : ADM_socket(newSocket)
{
    
}

/**
    \fn dtor
*/
ADM_commandSocket::~ADM_commandSocket()
{
}
/**
    
*/
bool ADM_commandSocket::sendMessage(const ADM_socketMessage &msg)
{
    return false;
}
/**
    \fn 
*/
bool ADM_commandSocket::getMessage(ADM_socketMessage &msg)
{
    return false;
}
/**
    \fn getPayloadAsUint32_t
*/
bool     ADM_socketMessage::getPayloadAsUint32_t(uint32_t *v)
{
    if(payloadLength!=4)
    {
        ADM_error("payload is not uint32\n");
        return false;
    }
    uint32_t a=(payload[0])+(payload[1]<<8)+(payload[2]<<16)+(payload[4]<<24);
    *v=a;
    return true;
}
/**
    \fn setPayloadAsUint32_t
*/
bool     ADM_socketMessage::setPayloadAsUint32_t(uint32_t v)
{
    uint8_t *p=payload;
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    payloadLength=4;    
    return true;
}
// EOF
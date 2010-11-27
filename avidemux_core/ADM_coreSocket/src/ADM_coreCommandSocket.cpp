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


static bool     uint32_to_char(uint32_t v,uint8_t *p)
 {
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    *p++=(v&0xff);v>>=8;
    return true;
}
static bool     char_to_uint32(uint32_t *v,uint8_t *p)
 {
    uint32_t a=(p[0])+(p[1]<<8)+(p[2]<<16)+(p[4]<<24);
    *v=a;
    return true;
}

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
        \fn sendMessage
*/
bool ADM_commandSocket::sendMessage(const ADM_socketMessage &msg)
{

#define TXX(u,v) if(!txData(u,tmp)) {ADM_error(v" error sending data\n");return false;}

    uint8_t tmp[4];
    if(!mySocket) return false;
    // 1- Send command as uint8_t
    tmp[0]=(uint8_t)msg.command;
    TXX(1,"command");
    
    // 2- Send size as uint32_t
    uint32_to_char(msg.payloadLength,tmp);
    TXX(4,"payloadLength");
    if(msg.payloadLength)
    {
        if(!txData(msg.payloadLength,msg.payload))
        {
            ADM_error("Cannot send payload for command %d\n",msg.command);
            return false;
        }
    }
    return true;
}
/**
    \fn getMessage
*/
bool ADM_commandSocket::getMessage(ADM_socketMessage &msg)
{
#define RXX(u,v) if(!rxData(u,tmp)) {ADM_error(v" error rxing data\n");return false;}
    uint8_t tmp[4];
    if(!mySocket) return false;
    // 1- Send command as uint8_t
    RXX(1,"command");
    msg.command=(ADM_socketCommand)tmp[0];
    RXX(4,"payloadLength");
    char_to_uint32(&(msg.payloadLength),tmp);
    if(msg.payloadLength)
    {
        ADM_assert(msg.payloadLength<ADM_COMMAND_SOCKET_MAX_PAYLOAD);
        if(!rxData(msg.payloadLength,msg.payload)) 
        {
                ADM_error(" error rxing payload\n");
                return false;
        }

    }
    return true;
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

    char_to_uint32(v,payload);
    return true;
}
/**
    \fn setPayloadAsUint32_t
*/
bool     ADM_socketMessage::setPayloadAsUint32_t(uint32_t v)
{
    uint32_to_char(v,payload);
    payloadLength=4;    
    return true;
}

/**
    \fn waitForConnect
    \brief wait for incoming TCP connection...
    \return null if no connection
*/
ADM_commandSocket *ADM_commandSocket::waitForConnect(uint32_t timeoutMs)
{
#warning fixme badly
    return (ADM_commandSocket *)ADM_socket::waitForConnect(timeoutMs);
}
// EOF
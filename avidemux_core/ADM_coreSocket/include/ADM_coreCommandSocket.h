/***************************************************************************
    \file   ADM_coreSocket.h
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
#ifndef ADM_CORE_COMMAND_SOCKET_H
#define ADM_CORE_COMMAND_SOCKET_H

#include "ADM_coreSocket6_export.h"
#include "ADM_coreSocket.h"

#define ADM_COMMAND_SOCKET_VERSION 2

#define ADM_COMMAND_SOCKET_MAX_PAYLOAD 16

/**
    \enum ADM_socketCommand
*/
typedef enum
{
    ADM_socketCommand_Hello=1,
    ADM_socketCommand_End=2,
    ADM_socketCommand_Progress=3,
}ADM_socketCommand;

/**
        \struct ADM_socketMessage
*/
class ADM_CORESOCKET6_EXPORT ADM_socketMessage
{
public:
    ADM_socketCommand command;
    uint32_t payloadLength;
    uint8_t  payload[ADM_COMMAND_SOCKET_MAX_PAYLOAD];
    bool     getPayloadAsUint32_t(uint32_t *v);
    bool     setPayloadAsUint32_t(uint32_t v);
};


/**
    \class ADM_commandSocket
    \brief Wrapper around socket/tcp
*/
class ADM_CORESOCKET6_EXPORT ADM_commandSocket : public ADM_socket       
{
    protected:
    public:
        virtual ADM_commandSocket *waitForConnect(uint32_t timeoutMs);
        bool sendMessage(const ADM_socketMessage &msg);
        bool getMessage(ADM_socketMessage &msg);
        bool pollMessage(ADM_socketMessage &msg);
        bool handshake(void);
        bool isAlive(void)
                {
                    if(mySocket) return true;
                    return false;
                }
    public:
        ADM_commandSocket(int newSocket);
        ADM_commandSocket( void );
        ~ADM_commandSocket(  );
};
#endif
//EOF

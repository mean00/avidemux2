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
#ifndef ADM_CORE_SOCKET_H
#define ADM_CORE_SOCKET_H

#ifdef _WIN32
#include "winsock2.h"
#endif

#include "ADM_coreSocket6_export.h"
#include "ADM_threads.h"

/**
    \class ADM_socket
    \brief Wrapper around socket/tcp
*/
class ADM_CORESOCKET6_EXPORT ADM_socket       
{
    protected:
        int         mySocket;
        admMutex    lock;       
        bool        create(void);
    public:
virtual ADM_socket *waitForConnect(uint32_t timeoutMs);
        bool        createBindAndAccept(uint32_t *port);
        bool        connectTo(uint32_t port);
        bool        rxData(uint32_t howmuch, uint8_t *where);
        bool        txData(uint32_t howmuch, const uint8_t *where);
        bool        close(void);
        bool        isAlive(void);
        bool        setNoDelay(bool value);
    public:
        ADM_socket(int newSocket);
        ADM_socket( void );
virtual ~ADM_socket(  );
};
#endif
//EOF

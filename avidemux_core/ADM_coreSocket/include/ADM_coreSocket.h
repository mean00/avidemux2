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
#ifndef AVS_CORE_SOCKET_H
#define AVS_CORE_SOCKET_H
#include "ADM_threads.h"

/**
    \class ADM_Socket
*/
class ADM_Socket       
{
    protected:
        int         mySocket;
        admMutex    lock;
    public:
        bool     bindAndAccept(uint32_t *port);
        bool     connectTo(uint32_t port);
        bool     rxData(uint32_t howmuch, uint8_t *where);
        bool     txData(uint32_t howmuch, uint8_t *where);
        bool     close(void);
    public:

        ADM_Socket( void );
        ~ADM_Socket(  );
};
#endif
//EOF

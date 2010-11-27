/***************************************************************************
    \file ADM_coreSocket.cpp
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

#ifdef __MINGW32__
#include <windows.h>
#include <winbase.h>
#include <io.h>
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "ADM_default.h"

#include "ADM_coreSocket.h"

#define MAGGIC 0xDEADBEEF

#define aprintf(...) {}
//#define DEBUG_NET
/**
    \fn connectTo
*/
bool ADM_Socket::connectTo(uint32_t port)
{
 #ifdef __MINGW32__
 mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 #else
 mySocket = socket(PF_INET, SOCK_STREAM, 0);
 #endif
    if(mySocket==-1)
    {
        ADM_error("Socket creation failed\n");
        return 0;
    }
    struct sockaddr_in  service;
    service.sin_family = AF_INET;
#ifdef DEBUG_NET
    service.sin_addr.s_addr = inet_addr("192.168.0.21");
#else
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif    
    service.sin_port = htons(port);
    
// Set socket to lowdelay, else it will be choppy
    int flag = 1;
    setsockopt( mySocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) );


    if(connect(mySocket,(struct sockaddr *)&service,sizeof(service)))
    {
        ADM_error("[ADMSocket]Socket connect error %d on port %d\n",errno,port);
        return 0;
    }
    ADM_info("[ADMSocket]Connected to port %d, socket %d\n",port,mySocket);
    return 1;
}
/**
    \fn close
*/
bool ADM_Socket::close(void)
{
    if(mySocket)
    {
        int er;
#ifdef __MINGW32__
		er=shutdown(mySocket,SD_BOTH);
#else
        er=shutdown(mySocket,SHUT_RDWR);
#endif
        if(er) ADM_error("[ADMSocket]Error when socket shutdown  %d (socket %d)\n",er,mySocket);
        mySocket=0;
    }
    return 1;
}
/**
    \fn rxData
*/
bool ADM_Socket::rxData(uint32_t howmuch, uint8_t *where)
{
uint32_t got=0;
int rx;
    while(got<howmuch)
    {
        rx=recv(mySocket,(char *)where,howmuch-got,0);
        if(rx<0)
        {
          perror("RxData");
          return 0;
        }
        where+=rx;
        got+=rx;
    }
  return 1;
}
/**
    \fn txData
*/
bool ADM_Socket::txData(uint32_t howmuch, uint8_t *where)
{
uint32_t got=0,tx;
    while(got<howmuch)
    {
        tx=send(mySocket,(char *)where,howmuch-got,0);
         if(tx<0)
        {
          perror("TxData");
          return 0;
        }
        where+=tx;
        got+=tx;
    }
  return 1;
}
/**
    \fn bindAndAccept
    \brief bind to any port and accept incoming packets
*/
bool     ADM_Socket::bindAndAccept(uint32_t *port)
{

}
/**
    \fn ctor
*/
ADM_Socket::ADM_Socket()
{
    mySocket=0;
}
/**
    \fn dtor
*/
ADM_Socket::~ADM_Socket()
{
    close();
}
//EOF

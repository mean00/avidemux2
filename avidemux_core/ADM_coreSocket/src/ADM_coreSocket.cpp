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

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <ws2tcpip.h>
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
#define BIND_ADR "127.0.0.1"
#define aprintf(...) {}
//#define DEBUG_NET

#ifdef _WIN32
        #define SADDR    SOCKADDR
        #define SADDR_IN struct sockaddr_in
        #define SERROR   SOCKET_ERROR
        #define SNET     AF_INET
        #define SPROTO   IPPROTO_TCP
        #define SCLOSE   SD_BOTH
        #define SSOCKLEN int
#else
        #define SADDR struct    sockaddr
        #define SADDR_IN struct sockaddr_in    
        #define SERROR          -1
        #define SNET            PF_INET
        #define SPROTO          0
        #define SCLOSE          SHUT_RDWR
        #define SSOCKLEN        socklen_t
#endif


/**
    \fn connectTo
*/
bool ADM_socket::connectTo(uint32_t port)
{
    if(false==create())
    {
        ADM_error("Canno create socket\n");
        return false;
    }
    struct sockaddr_in  service;
    service.sin_family = SNET;
    service.sin_addr.s_addr = inet_addr(BIND_ADR);
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
bool ADM_socket::close(void)
{
    if(mySocket)
    {
        int er;
		er=shutdown(mySocket,SCLOSE);
        if(er) ADM_error("[ADMSocket]Error when socket shutdown  %d (socket %d)\n",er,mySocket);
        mySocket=0;
    }
    return 1;
}
/**
    \fn rxData
*/
bool ADM_socket::rxData(uint32_t howmuch, uint8_t *where)
{
uint32_t got=0;
int rx;
    while(got<howmuch)
    {
        rx=recv(mySocket,(char *)where,howmuch-got,0);
        if(rx<=0)
        {
          perror("RxData");
          close();
          return false;
        }
        where+=rx;
        got+=rx;
    }
  return true;
}
/**
    \fn txData
*/
bool ADM_socket::txData(uint32_t howmuch, const uint8_t *where)
{
uint32_t got=0;
int tx;
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
    \fn create
    \brief create a TCP socket
*/
bool ADM_socket::create(void)
{
      mySocket = socket(AF_INET, SOCK_STREAM, 0);
      if(mySocket<0) return false;
      int flag = 1;
      int result = setsockopt(mySocket,  IPPROTO_TCP,  TCP_NODELAY, (char *) &flag, sizeof(int));
      if(result<0)
      {
          ADM_warning("Cannot set TCP_NO_DELAY\n");
      }
      return true;

}
/**
    \fn createBindAndAccept
    \brief bind to any port and accept incoming packets
*/
bool     ADM_socket::createBindAndAccept(uint32_t *port)
{
    if(!create())
    {
        ADM_error("Cannot create socket\n");
        return false;
    }
#ifndef _WIN32
int enable = 1;
if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    ADM_error("Oops : setsockopt(SO_REUSEADDR) failed\n");
#endif

  ADM_info("Binding on %s:%d\n",BIND_ADR,*port);
  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_addr.s_addr = inet_addr(BIND_ADR);
  service.sin_port = htons(*port); // bind to any port

  if (bind( mySocket,  (SADDR *)&service, sizeof(service))) 
  {
		ADM_error("bind() failed  \n");
		fflush(stdout);
		close();
		return false;
  }
   // Get port 
    SSOCKLEN len=sizeof( service);
    if ( getsockname ( mySocket, (SADDR *)& service, &len ) < 0 ) 
    {
        ADM_error("Getsockname failed\n");
        fflush(stdout);
        close();
        return false;
    }
    *port= ntohs ( ((SADDR_IN *)&service)->sin_port ); 
     
   // Set high buffer + low delay
    ADM_info("Socket bound to port %u\n",(unsigned int)*port);

    int er=listen(mySocket,1);
	if(er)
	{
		ADM_error("Error in listen\n");
		fflush(stdout);
        return false;
	}

    return true;
}
/**
    \fn waitForConnect
    \brief wait for incoming TCP connection...
    \return null if no connection
*/
ADM_socket *ADM_socket::waitForConnect(uint32_t timeoutMs)
{
    //
        if(!mySocket)
        {
            ADM_error("Wait for connect called with no socket opened\n");
            return NULL;        
        }
    // Wait for connect...
        fd_set set;
        FD_ZERO(&set);
        FD_SET(mySocket,&set);
        struct timeval timeout; 

        uint32_t sec=timeoutMs/1000;

        timeout.tv_sec=sec;
        timeout.tv_usec=((timeoutMs-sec*1000))*1000; // us
        //ADM_info("Selecting\n");
        int evt=select(1+mySocket,&set,NULL,NULL,&timeout);
        if(evt<=0) 
        {
            ADM_error("Select failed\n");
            return NULL;
        }

        int workSocket = SERROR;
        ADM_info("Accepting...\n");
        workSocket = accept( mySocket, NULL, NULL);
        if(SERROR==workSocket) 
        {
            ADM_error("Accept failed\n");
            return NULL;
        }
        return new ADM_socket(workSocket);
}
/**
    \fn isAlive
*/
bool ADM_socket::isAlive(void)
{
    if(!mySocket) return false;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(mySocket,&set);
        struct timeval timeout; 

        timeout.tv_sec=0;
        timeout.tv_usec=100000; // 100 us check
        //ADM_info("Selecting\n");
        int evt=select(1+mySocket,&set,&set,&set,&timeout);
        if(evt<0) 
        {
            ADM_error("Select failed\n");
            return false;
        }

    return true;
}
/**
    \fn ctor
*/
ADM_socket::ADM_socket()
{
    mySocket=0;
}
/**
    \fn ctor
*/
ADM_socket::ADM_socket(int newSocket)
{
    mySocket=newSocket;
}

/**
    \fn dtor
*/
ADM_socket::~ADM_socket()
{
    close();
}
//EOF

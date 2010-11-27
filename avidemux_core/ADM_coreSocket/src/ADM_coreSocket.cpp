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

#ifdef __MINGW32__
        #define SADDR    SOCKADDR
        #define SADDR_IN SOCKADDR
        #define SERROR   SOCKET_ERROR
        #define SNET     AF_INET
        #define SPROTO   IPPROTO_TCP
        #define SCLOSE   SD_BOTH
#else
        #define SADDR struct    sockaddr
        #define SADDR_IN struct sockaddr_in    
        #define SERROR          -1
        #define SNET            PF_INET
        #define SPROTO          0
        #define SCLOSE          SHUT_RDWR
#endif


/**
    \fn connectTo
*/
bool ADM_socket::connectTo(uint32_t port)
{
 mySocket = socket(SNET, SOCK_STREAM, SPROTO);
    if(mySocket==-1)
    {
        ADM_error("Socket creation failed\n");
        return 0;
    }
    struct sockaddr_in  service;
    service.sin_family = SNET;
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
bool ADM_socket::txData(uint32_t howmuch, uint8_t *where)
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
    \fn create
    \brief create a TCP socket
*/
bool ADM_socket::create(void)
{
      mySocket = socket(AF_INET, SOCK_STREAM, 0);
      if(mySocket<0) return false;
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

  sockaddr_in service;
  service.sin_family = AF_INET;
#define BIND_ADR "127.0.0.1"
    service.sin_addr.s_addr = inet_addr(BIND_ADR);
	printf("Binding on %s\n",BIND_ADR);

  *port=0;
  service.sin_port = 0; // bind to any port


  int one=true;
  if (bind( mySocket,  (SADDR *)&service, sizeof(service))) 
  {
		ADM_error("bind() failed  \n");
		fflush(stdout);
		close();
		return false;
  }
   // Get port 
    socklen_t len=sizeof( service);
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

        timeout.tv_sec=timeoutMs/1000;
        timeout.tv_usec=((timeoutMs)-(timeout.tv_sec*1000))*1000;
        
        int evt=select(1+mySocket,&set,NULL,NULL,&timeout);
        if(evt<0) 
        {
            ADM_error("Select failed\n");
            return NULL;
        }

        int workSocket = SERROR;
        workSocket = accept( mySocket, NULL, NULL);
        if(SERROR==workSocket) 
        {
            ADM_error("Accept failed\n");
            return NULL;
        }
        return new ADM_socket(workSocket);
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

/**
	\file sket.cpp
	\brief Socket handling
	\author mean fixounet@free.fr

*/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.


//#define DEBUG
#include <stdio.h>
#include "sket.h"

Sket::Sket(void)
{
	mySocket=0;
	port=9999;

	 mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  sockaddr_in service;
  service.sin_family = AF_INET;
#ifdef DEBUG
	// Get the local host information
  hostent* localHost;
  char* localIP;
	localHost = gethostbyname("");
	localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
	service.sin_addr.s_addr = inet_addr(localIP);
	printf("Binding ...\n");
	int ip=service.sin_addr.s_addr;
	for(int i=0;i<4;i++)
	{
			printf("%d ",ip&0xff);
		    ip>>=8;
	}
	printf("\n");
#else
	#define BIND_ADR "127.0.0.1"
    service.sin_addr.s_addr = inet_addr(BIND_ADR);
	printf("Binding on %s\n",BIND_ADR);
#endif
  
  
  service.sin_port = htons(port);


  int one=true;
  if (bind( mySocket,   (SOCKADDR*) &service,  sizeof(service)) == SOCKET_ERROR) 
  {
	int opt=setsockopt(mySocket,
				SOL_SOCKET,
				SO_REUSEADDR,
				(char *)&one,sizeof(one));
	printf("Trying to use SO_REUSEADDR\n");
	if(opt) printf("Error in setsockopt:%d\n",opt);
  
	  // Retry with SO_REUSEADDR set...
	   if (bind( mySocket,   (SOCKADDR*) &service,  sizeof(service)) == SOCKET_ERROR) 
	   {
		printf("bind() failed to port %u \n",port);
		fflush(stdout);
		closesocket(mySocket);
		mySocket=0;
		exit(-1);
	  }
  }
   // Set high buffer + low delay
  printf("Socket bound to port %u\n",port);
  fflush(stdout);
}
Sket::~Sket()
{
	if(mySocket)
	{
		closesocket(mySocket);
		mySocket=0;
	}
}
uint8_t Sket::socketBind(void)
{
	
 
  return 1;
}
uint8_t Sket::waitConnexion(void)
{
	int er;
	er=listen(mySocket,1);
	if(er==SOCKET_ERROR)
	{
		printf("Error in lisent\n");
		fflush(stdout);
	}

  while(1) 
  {
		printf("Waiting for client to connect...\n");
		fflush(stdout);
		workSocket = (SOCKET)SOCKET_ERROR;
		while( workSocket == SOCKET_ERROR ) 
		{
			workSocket = accept( mySocket, NULL, NULL );
		}
		printf("Client connected.\n");
		fflush(stdout);
		break;
  }
  return 1;
}
uint8_t Sket::receive(uint32_t *cmd, uint32_t *frame,uint32_t *payload_size,uint8_t *payload)
{
	SktHeader header;
	memset(&header,0,sizeof(header));

	int rx=recv(workSocket,(char *)&header,sizeof(header),0);
	if( sizeof(header)!=rx)
	{
		printf("Error in receivedata: header, expected %d, received %d\n",(int) sizeof(header),rx);
		fflush(stdout);
		exit(-1);
	}
	*cmd=header.cmd;
	*payload_size=header.payloadLen;
	*frame=header.frame;
	if(header.magic!=(uint32_t)MAGGIC)
	{
		printf("Wrong magic\n");
		fflush(stdout);
		exit(-1);
	}
	if(header.payloadLen)
	{
		int togo=header.payloadLen;
		int chunk;
		while(togo)
		{
			chunk=recv(workSocket,(char *)payload,togo,0);
			if(chunk<0)
			{
				printf("Error in senddata: body\n");
				fflush(stdout);
				exit(-1);
			}
			togo-=chunk;
			payload+=chunk;
		}
	}

	return 1;
}

uint8_t Sket::sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload)
{
	SktHeader header;
	memset(&header,0,sizeof(header));

	header.cmd=cmd;
	header.payloadLen=payload_size;
	header.frame=frame;
	header.magic=(uint32_t)MAGGIC;
	
	if(sizeof(header)!= send(workSocket,(char *)&header,sizeof(header),0))
	{
		printf("Error in senddata: header\n");
		fflush(stdout);
		exit(-1);
	}
	int togo=payload_size;
	int chunk;
	while(togo)
	{
		chunk=send(workSocket,(char *)payload,togo,0);
		if(chunk<0)
		{
			printf("Error in senddata: body\n");
			fflush(stdout);
			exit(-1);
		}
		togo-=chunk;
		payload+=chunk;
	}
	return 1;


	return 0;
}

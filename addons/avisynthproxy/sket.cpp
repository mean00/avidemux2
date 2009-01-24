/*



*/

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
  service.sin_addr.s_addr = inet_addr("192.168.0.10");
#else
  service.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
  service.sin_port = htons(port);

   if (bind( mySocket,   (SOCKADDR*) &service,  sizeof(service)) == SOCKET_ERROR) 
   {
    printf("bind() failed to port %u \n",port);
	fflush(stdout);
    closesocket(mySocket);
	mySocket=0;
    exit(-1);
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


	if( sizeof(header)!=recv(workSocket,(char *)&header,sizeof(header),0))
	{
		printf("Error in receivedata: header\n");
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

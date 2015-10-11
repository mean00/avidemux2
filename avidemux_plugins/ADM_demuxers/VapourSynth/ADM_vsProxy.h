/***************************************************************************
                         ADM_vs
                             -------------------
    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once 

#include "VSScript.h"
#include "VSHelper.h"
#include "../../ADM_coreSocket/include/ADM_coreSocket.h"
#include "../AvsProxy/avsHeader.h"
#include "sys/socket.h"

#ifdef _WIN32
          #define SERROR   SOCKET_ERROR
#else
          #define SERROR   -1
#endif


/**
 */
typedef struct SktHeader
{
    uint32_t cmd;
    uint32_t frame;
    uint32_t payloadLen;
    uint32_t magic;
}SktHeader;
#define MAGGIC 0xDEADBEEF
/**
 */
class vsSocket : public ADM_socket
{
public:
    vsSocket()
    {
        mySocket=0;
    }
    vsSocket(int newSocket)
    {
        mySocket=newSocket;
    }

    bool receive(uint32_t *cmd,uint32_t *frame,uint32_t *payloadSize, uint8_t *payload)
    {
        SktHeader header;
	memset(&header,0,sizeof(header));

	int rx=::recv(mySocket,(char *)&header,sizeof(header),0);
	if( sizeof(header)!=rx)
	{
		printf("Error in receivedata: header, expected %d, received %d\n",(int) sizeof(header),rx);
		fflush(stdout);
		exit(-1);
	}
	*cmd=header.cmd;
	*payloadSize=header.payloadLen;
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
			chunk=recv(mySocket,(char *)payload,togo,0);
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
    vsSocket *waitForConnect(uint32_t timeoutMs)
    {
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
        return new vsSocket(workSocket);
    }
    
    uint8_t sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload)
    {
	SktHeader header;
	memset(&header,0,sizeof(header));

	header.cmd=cmd;
	header.payloadLen=payload_size;
	header.frame=frame;
	header.magic=(uint32_t)MAGGIC;
	
	if(sizeof(header)!= send(mySocket,(char *)&header,sizeof(header),0))
	{
		printf("Error in senddata: header\n");
		fflush(stdout);
		return false;
	}
	int togo=payload_size;
	int chunk;
	while(togo)
	{
		chunk=send(mySocket,(char *)payload,togo,0);
		if(chunk<0)
		{
			printf("Error in senddata: body\n");
			fflush(stdout);
			return false;
		}
		togo-=chunk;
		payload+=chunk;
	}
	return 1;
    }
};

/**
    \Class vsHeader
    \brief Flash demuxer

*/
class vapourSynthProxy      
{
public:
                      vapourSynthProxy();
                      ~vapourSynthProxy();
   bool              run(const char *name);
protected:

    uint8_t        *_buffer;

protected:
    VSScript       *_script;
    int            _outputIndex;
    VSNodeRef       *_node;
    avsyInfo        _info;
    bool            manageSlave(vsSocket *slave,const VSVideoInfo *vi);
    bool            packFrame( const VSVideoInfo *vi,const VSFrameRef *frame);
    bool            fillInfo( const VSVideoInfo *vi);
    void            abort(void);
};



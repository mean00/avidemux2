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

#ifdef _WIN32
        #include "winsock2.h"
        #define SERROR   SOCKET_ERROR
#else
        #include "sys/socket.h"
        #define SERROR   -1
#endif
#include "ADM_default.h"
#include "ADM_coreAvsProtocol.h"
/**
 * 
 */
avsSocket::avsSocket()
{
    mySocket=0;
}
/**
 * 
 * @param newSocket
 */
avsSocket::avsSocket(int newSocket)
{
    mySocket=newSocket;
}
/**
 * 
 * @param cmd
 * @param frame
 * @param payloadSize
 * @param payload
 * @return 
 */
bool avsSocket::receive(uint32_t *cmd,uint32_t *frame,uint32_t *payloadSize, uint8_t *payload)
{
    SktHeader header;
    memset(&header,0,sizeof(header));

    int rx=::recv(mySocket,(char *)&header,sizeof(header),0);
    if( sizeof(header)!=rx)
    {
        printf("Error in receivedata: header, expected %d, received %d\n",(int) sizeof(header),rx);
        fflush(stdout);
        return false;
    }
    *cmd=header.cmd;
    *payloadSize=header.payloadLen;
    *frame=header.frame;
    if(header.magic!=(uint32_t)MAGGIC)
    {
        printf("Wrong magic\n");
        fflush(stdout);
        return false;
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
                return false;
            }
            togo-=chunk;
            payload+=chunk;
        }
    }
    return true;
}
/**
 * 
 * @param timeoutMs
 * @return 
 */
avsSocket *avsSocket::waitForConnect(uint32_t timeoutMs)
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
    return new avsSocket(workSocket);
}
/**
 * 
 * @param cmd
 * @param frame
 * @param payload_size
 * @param payload
 * @return 
 */
bool avsSocket::sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload)
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
    return true;
}
//EOF
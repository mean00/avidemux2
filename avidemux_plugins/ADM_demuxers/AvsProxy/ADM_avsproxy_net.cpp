

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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "ADM_default.h"
#include "ADM_avsproxy_internal.h"
#include "ADM_avsproxy.h"

#define MAGGIC 0xDEADBEEF

#define aprintf(...) {}

uint8_t avsHeader::bindMe(uint32_t port)
{
 #ifdef __MINGW32__
 mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
 #else
 mySocket = socket(PF_INET, SOCK_STREAM, 0);
 #endif
    if(mySocket==-1)
    {
        printf("Socket failed\n");
        return 0;
    }
    struct sockaddr_in  service;
    service.sin_family = AF_INET;
#ifdef DEBUG_NET
    service.sin_addr.s_addr = inet_addr("192.168.0.10");
#else
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif    
    service.sin_port = htons(port);
    
    if(connect(mySocket,(struct sockaddr *)&service,sizeof(service)))
    {
        printf("[avsProxy]Socket connect error %d on port %d\n",errno,port);
        return 0;
    }
    printf("[avsproxy]Connected to avsproxy : port %d, socket %d\n",port,mySocket);
    return 1;
}
uint8_t avsHeader::close(void)
{
    if(mySocket)
    {
        int er;
#ifdef __MINGW32__
		er=shutdown(mySocket,SD_BOTH);
#else
        er=shutdown(mySocket,SHUT_RDWR);
#endif
        if(er) printf("[avsProxy]Error when socket shutdown  %d (socket %d)\n",er,mySocket);
        mySocket=0;
    }
    return 1;
}

uint8_t avsHeader::askFor(uint32_t cmd,uint32_t frame, uint32_t payloadsize,uint8_t *payload)
{
   
    if(!sendData(cmd,frame,0,NULL))
    {
        printf("[avsProxy]Send Cmd %u failed for frame %u\n",cmd,frame);
        return 0;
    }
    // Wait reply
    uint32_t size,reply,outframe;
    if(!receiveData(&reply,&outframe,&size,payload))
    {
        printf("[avsProxy]Rx Cmd %u failed for frame %u\n",cmd,frame);
        return 0;   
    }
  
    // Check!
    ADM_assert(outframe==frame);
    ADM_assert(reply==cmd+1);
    ADM_assert(size==payloadsize);
    aprintf("[avsProxy]Cmd %u on frame %u succeed\n",cmd,frame);
    return 1;
    
}
uint8_t avsHeader::rxData(uint32_t howmuch, uint8_t *where)
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
uint8_t avsHeader::txData(uint32_t howmuch, uint8_t *where)
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

uint8_t avsHeader::receiveData(uint32_t *cmd, uint32_t *frame,uint32_t *payload_size,uint8_t *payload)
{
        SktHeader header;
        memset(&header,0,sizeof(header));
        int rx;


        rx=rxData(sizeof(header),(uint8_t *)&header);
       
        *cmd=header.cmd;
        *payload_size=header.payloadLen;
        *frame=header.frame;
        if(header.magic!=(uint32_t)MAGGIC)
        {
            printf("[avsProxy]Wrong magic %x/%x\n",header.magic,MAGGIC);
            return 0;
        }
        if(header.payloadLen)
        {
            int togo=header.payloadLen;
            return rxData(togo,payload);
            
        }

        return 1;
}


uint8_t avsHeader::sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload)
{
        SktHeader header;
        memset(&header,0,sizeof(header));

        header.cmd=cmd;
        header.payloadLen=payload_size;
        header.frame=frame;
        header.magic=(uint32_t)MAGGIC;
        if(!txData(sizeof(header),(uint8_t *)&header))
        {
            printf("Error in senddata: header %d\n",sizeof(header));
            return 0;
        }
        int togo=payload_size;
        int chunk;
        return txData(togo,payload);
}

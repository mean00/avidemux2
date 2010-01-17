/***************************************************************************
    \file ADM_avsproxy_net.h
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
#ifndef AVS_PROXY_NET_H
#define AVS_PROXY_NET_H

typedef struct
{
    uint32_t size;
    uint32_t sizeMax;
    uint8_t  *buffer;
}avsNetPacket;

/**
    \class avsNet
*/
class avsNet       
{
    protected:
        int         mySocket;
    public:
        bool     bindMe(uint32_t port);
        bool     sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload);
        bool     receiveData(uint32_t *cmd, uint32_t *frame,uint32_t *payload_size,uint8_t *payload);
        
        bool     command(uint32_t cmd,uint32_t frame,avsNetPacket *in,avsNetPacket *out);
        bool     rxData(uint32_t howmuch, uint8_t *where);
        bool     txData(uint32_t howmuch, uint8_t *where);
        bool     close(void);
    public:


        virtual   void 				Dump(void) {};

        avsNet( void );
        ~avsNet(  );
};
#endif
//EOF

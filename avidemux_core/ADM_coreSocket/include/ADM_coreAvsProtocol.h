/** 
	\file avsHeader.h
	\brief Wrapper to deal with AvsProxy protocol

        \author (C) 2007-2015 by mean  fixounet@free.fr

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
#include "ADM_coreSocket6_export.h"
#include "ADM_coreSocket.h"



#define AVSHEADER_API_VERSION 2
#define MAGGIC 0xDEADBEEF

/**
 */
 enum AvsEnum
{
	AvsCmd_GetInfo=1,
	AvsCmd_SendInfo=2,
	AvsCmd_GetFrame=3,
	AvsCmd_SendFrame=4,
	AvsCmd_GetAudio=5,
	AvsCmd_SendAudio=6,
	AvsCmd_Quit=99
};
/**
 */
typedef struct avsInfo
{
	uint32_t version;
	uint32_t width;
	uint32_t height;
	uint32_t fps1000;
	uint32_t nbFrames;
	uint32_t frequency;
	uint32_t channels;
}avsyInfo;
/**
 */
typedef struct 
{
	uint32_t sizeInFloatSample;
	uint64_t startSample;       // -1 means continue
}avsAudioFrame;



/**
 */
typedef struct SktHeader
{
    uint32_t cmd;
    uint32_t frame;
    uint32_t payloadLen;
    uint32_t magic;
}SktHeader;

/**
 * \class avsSocket
 * \brief Class that simplifies connecting/receiving avsProxy data
 */
class ADM_CORESOCKET6_EXPORT avsSocket : public ADM_socket
{
public:
            avsSocket();
            avsSocket(int newSocket);
    bool    receive(uint32_t *cmd,uint32_t *frame,uint32_t *payloadSize, uint8_t *payload); // return false is general fatal error
    avsSocket *waitForConnect(uint32_t timeoutMs);
    bool    sendData(uint32_t cmd,uint32_t frame, uint32_t payload_size,uint8_t *payload);

};

//EOF



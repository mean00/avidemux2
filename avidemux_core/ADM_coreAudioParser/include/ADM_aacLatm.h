/***************************************************************************
  \file ADM_aacLatm.h
  \brief Extract AAC frames from LATM
  Derived from vlc file mpeg4audio.c
                Copyright (C) 2001, 2002, 2006 the VideoLAN team
                * $Id: 3e4561c09d213a64031992f729c671bed66e824c $
                *
                * Authors: Laurent Aimar <fenrir@via.ecp.fr>
                *          Gildas Bazin <gbazin@netcourrier.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_AAC_LATM
#define ADM_AAC_LATM

#include "ADM_audioParser6_export.h"
#include "ADM_getbits.h"
#include "ADM_byteBuffer.h"
#include <list>
#include "ADM_ptrQueue.h"
/**
    \class ADM_latm2aac
*/
#define AAC_LATM_MAX_EXTRA      10
#define LATM_MAX_LAYER          64
#define LATM_MAX_BUFFER_SIZE    8192
#define LATM_NB_BUFFERS         16

typedef struct
{
    int      nbLayers;
    int      payload[LATM_MAX_LAYER];
    int      frameLengthType[LATM_MAX_LAYER];
    int      audioMuxVersion;
    int      audioMuxVersionA;
    bool     allStreamsSameTimeFraming;
    bool     gotConfig;
}latmConf_t;

/**
 * \class latmBuffer
 */
class latmBuffer
{
public:    
        ADM_byteBuffer  buffer;
        uint32_t bufferLen;
        uint64_t dts;
        latmBuffer()
        {
                buffer.setSize(LATM_MAX_BUFFER_SIZE);
        }
};

class ADM_AUDIOPARSER6_EXPORT ADM_latm2aac
{
private:
                latmBuffer buffers[LATM_NB_BUFFERS];
                ADM_ptrQueue <latmBuffer > listOfFreeBuffers;
                ADM_ptrQueue <latmBuffer > listOfUsedBuffers;
                uint32_t extraLen;
                uint8_t  extraData[AAC_LATM_MAX_EXTRA];
                uint32_t fq,channels;
                bool     readAudioMux(uint64_t dts, getBits &bits );
                bool     demuxLatm(uint64_t date,uint8_t *start,uint32_t size);
                bool     AudioSpecificConfig(getBits &bits,int &bitsConsumed);
                bool     readStreamMuxConfig(getBits &bits);
                int      readPayloadInfoLength(getBits &bits);
                bool     readPayload(getBits &bits, uint64_t dts,int size);

                latmConf_t conf;
public:
                bool getExtraData(uint32_t *len,uint8_t **data);
                bool pushData (int incomingLen,uint8_t *intData,uint64_t date);
                bool empty(void);
                bool getData(uint64_t *date, uint32_t *len, uint8_t *data,uint32_t maxData);
                bool flush(void);
                int  getFrequency(void);
                int  getChannels(void);
                     ADM_latm2aac();
                    ~ADM_latm2aac();

};
#endif


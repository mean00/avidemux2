/***************************************************************************
  GPL License
  Get info from frames
  (C) Mean 2007
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VIDEO_INFO_EXTRACTOR
#define ADM_VIDEO_INFO_EXTRACTOR

#include "ADM_coreUtils6_export.h"

ADM_COREUTILS6_EXPORT uint8_t extractMpeg4Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h,uint32_t *time_inc);
ADM_COREUTILS6_EXPORT uint8_t extractH263Info(uint8_t *data,uint32_t dataSize,uint32_t *w,uint32_t *h);
ADM_COREUTILS6_EXPORT uint8_t extractH263FLVInfo (uint8_t * buffer, uint32_t len, uint32_t * w, uint32_t * h);
ADM_COREUTILS6_EXPORT uint8_t extractVopInfo(uint8_t *data, uint32_t len,uint32_t timeincbits,uint32_t *vopType,uint32_t *modulo, uint32_t *time_inc);
ADM_COREUTILS6_EXPORT bool    extractVolHeader(uint8_t *data,uint32_t dataSize,uint8_t **volStart, uint32_t *volLen);
/**
    \struct ADM_SPSinfo
*/
typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t fps1000;
    uint32_t darNum;
    uint32_t darDen;
    bool     hasStructInfo;
    uint32_t CpbDpbToSkip;
}ADM_SPSInfo;

ADM_COREUTILS6_EXPORT uint8_t extractSPSInfo(uint8_t *data, uint32_t len,ADM_SPSInfo *info);
ADM_COREUTILS6_EXPORT bool extractSPSInfo_mp4Header(uint8_t *data, uint32_t len,ADM_SPSInfo *info);

ADM_COREUTILS6_EXPORT uint8_t extractH264FrameType(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags);
ADM_COREUTILS6_EXPORT uint8_t extractH265FrameType(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags);
uint8_t extractH264FrameType_startCode(uint32_t nalSize,uint8_t *buffer,uint32_t len,uint32_t *flags);
ADM_COREUTILS6_EXPORT bool    ADM_getH264SpsPpsFromExtraData(uint32_t extraLen,uint8_t *extra,
                                    uint32_t *spsLen,uint8_t **spsData,
                                    uint32_t *ppsLen,uint8_t **ppsData); // return a copy of pps/sps extracted

typedef struct
{
    uint8_t  *start;
    uint32_t size;   // size of payload excluding nalu type
    uint8_t  nalu;
}NALU_descriptor;
ADM_COREUTILS6_EXPORT int ADM_splitNalu(uint8_t *start, uint8_t *end, uint32_t maxNalu,NALU_descriptor *desc);
ADM_COREUTILS6_EXPORT int ADM_findNalu(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc);
ADM_COREUTILS6_EXPORT int ADM_convertFromAnnexBToMP4(uint8_t *inData,uint32_t inSize, uint8_t *outData,uint32_t outMaxSize);

#define SHORT_START_CODE

#ifdef SHORT_START_CODE
    #define SearchStartCode ADM_findMpegStartCode
    #define START_CODE_LEN 4
#else
    #define SearchStartCode ADM_findH264StartCode
    #define START_CODE_LEN 5
#endif
/**
    \struct ADM_vopS
    \brief describe a vop inside a bitstream (mpeg4 SP/ASP)
*/
typedef struct 
{
	uint32_t offset;
	uint32_t type;
    uint32_t vopCoded;
    uint32_t modulo;
    uint32_t timeInc;
}ADM_vopS;


ADM_COREUTILS6_EXPORT uint32_t ADM_searchVop(uint8_t *begin, uint8_t *end,uint32_t *nb, ADM_vopS *vop,uint32_t *timeincbits);
ADM_COREUTILS6_EXPORT uint32_t ADM_unescapeH264 (uint32_t len, uint8_t * in, uint8_t * out);
ADM_COREUTILS6_EXPORT uint32_t ADM_escapeH264 (uint32_t len, uint8_t * in, uint8_t * out);


ADM_COREUTILS6_EXPORT bool ADM_VC1getFrameType(uint8_t *start, int size, int *frameType);
#endif
//EOF

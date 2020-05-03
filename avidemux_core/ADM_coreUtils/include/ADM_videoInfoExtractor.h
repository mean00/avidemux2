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
    bool     hasPocInfo;
    uint32_t CpbDpbToSkip;
    uint32_t log2MaxFrameNum;
    uint32_t log2MaxPocLsb;
    bool     frameMbsOnlyFlag;
    uint32_t refFrames;
}ADM_SPSInfo;

/**
    \struct ADM_SPSinfoH265
*/
class ADM_COREUTILS6_EXPORT ADM_SPSinfoH265
{
public:
    ADM_SPSinfoH265()
    {
      width=height=fps1000=0;
      log2_max_poc_lsb=0;
      separate_colour_plane_flag=0;
      num_extra_slice_header_bits=0;  //copied from pps
      dependent_slice_segments_enabled_flag=false; // from pps
      output_flag_present_flag=false; // from pps
      field_info_present=false;
      address_coding_length=0;
    }
    int     width;
    int     height;
    int     fps1000;
    unsigned int log2_max_poc_lsb;
    int     separate_colour_plane_flag;
    int     num_extra_slice_header_bits;
    bool    dependent_slice_segments_enabled_flag;
    bool    output_flag_present_flag;
    bool    field_info_present;
    int     address_coding_length;
};

#define MAX_H264_SPS_SIZE 2048

ADM_COREUTILS6_EXPORT bool    ADM_SPSannexBToMP4(uint32_t dataLen,uint8_t *incoming, uint32_t *outLen, uint8_t *outData);

ADM_COREUTILS6_EXPORT bool    extractSPSInfo(uint8_t *data, uint32_t len,ADM_SPSInfo *info);
ADM_COREUTILS6_EXPORT bool    extractSPSInfo_mp4Header(uint8_t *data, uint32_t len,ADM_SPSInfo *info);
ADM_COREUTILS6_EXPORT bool    extractSPSInfoFromData(uint8_t *data, uint32_t length, ADM_SPSInfo *spsinfo);
ADM_COREUTILS6_EXPORT uint32_t getRawH264SPS(uint8_t *data, uint32_t len, uint32_t nalSize, uint8_t *dest, uint32_t maxsize);
ADM_COREUTILS6_EXPORT uint32_t getRawH264SPS_startCode(uint8_t *data, uint32_t len, uint8_t *dest, uint32_t maxsize);
ADM_COREUTILS6_EXPORT uint32_t ADM_getNalSizeH264(uint8_t *extra, uint32_t len);

ADM_COREUTILS6_EXPORT uint8_t extractH264FrameType(uint8_t *buffer, uint32_t len, uint32_t nalSize, uint32_t *flags, int *pocLsb, ADM_SPSInfo *sps, uint32_t *recovery=NULL);
ADM_COREUTILS6_EXPORT uint8_t extractH264FrameType_startCode(uint8_t *buffer,uint32_t len,uint32_t *flags,int *pocLsb,ADM_SPSInfo *sps,uint32_t *recovery=NULL);
ADM_COREUTILS6_EXPORT bool    extractH264SEI(uint8_t *src, uint32_t inlen, uint32_t nalSize, uint8_t *dest, uint32_t bufsize, uint32_t *outlen); // dest may be NULL

ADM_COREUTILS6_EXPORT bool    ADM_getH264SpsPpsFromExtraData(uint32_t extraLen,uint8_t *extra,
                                    uint32_t *spsLen,uint8_t **spsData,
                                    uint32_t *ppsLen,uint8_t **ppsData); // return a copy of pps/sps extracted

ADM_COREUTILS6_EXPORT uint32_t ADM_getNalSizeH265(uint8_t *extra, uint32_t len);

ADM_COREUTILS6_EXPORT bool    extractSPSInfoH265_mp4Header(uint8_t *data, uint32_t len,ADM_SPSinfoH265 *info); 
ADM_COREUTILS6_EXPORT bool    extractSPSInfoH265(uint8_t *data, uint32_t len, ADM_SPSinfoH265 *info);
ADM_COREUTILS6_EXPORT bool    extractH265FrameType(uint8_t *buffer, uint32_t len, uint32_t nalSize, ADM_SPSinfoH265 *info, uint32_t *flags, int *poc);
ADM_COREUTILS6_EXPORT bool    extractH265FrameType_startCode(uint8_t *buffer, uint32_t len, ADM_SPSinfoH265 *info, uint32_t *flags, int *poc);

#define MAX_NALU_PER_CHUNK 60

typedef struct
{
    uint8_t  *start;
    uint32_t size;   // size of payload excluding nalu type
    uint8_t  nalu;
    bool     zerobyte; // is the startcode prefix preceded by zero byte (long startcode)?
}NALU_descriptor;

ADM_COREUTILS6_EXPORT int ADM_splitNalu(uint8_t *start, uint8_t *end, uint32_t maxNalu, NALU_descriptor *desc);
ADM_COREUTILS6_EXPORT int ADM_findNalu(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc);
ADM_COREUTILS6_EXPORT int ADM_convertFromAnnexBToMP4(uint8_t *inData,uint32_t inSize, uint8_t *outData,uint32_t outMaxSize);
ADM_COREUTILS6_EXPORT int ADM_convertFromAnnexBToMP4H265(uint8_t *inData, uint32_t inSize, uint8_t *outData, uint32_t outMaxSize);
ADM_COREUTILS6_EXPORT NALU_descriptor *ADM_findNaluH265(uint32_t nalu,uint32_t maxNalu,NALU_descriptor *desc);

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

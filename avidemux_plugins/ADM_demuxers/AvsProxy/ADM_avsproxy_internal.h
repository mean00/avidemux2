/** 
	\file avsHeader.h
	\brief Content of exchanges between avidemux and avsproxy


*/
#ifndef AVS_HEADER_H
#define AVS_HEADER_H
#define AVSHEADER_API_VERSION 2
typedef enum AvsEnum
{
	AvsCmd_GetInfo=1,
	AvsCmd_SendInfo=2,
	AvsCmd_GetFrame=3,
	AvsCmd_SendFrame=4,
	AvsCmd_GetAudio=5,
	AvsCmd_SendAudio=6,
	AvsCmd_Quit=99
};

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

typedef struct 
{
	uint32_t sizeInFloatSample;
	uint64_t startSample;       // -1 means continue
}avsAudioFrame;
#endif


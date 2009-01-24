/*



*/
#ifndef AVS_HEADER_H
#define AVS_HEADER_H
typedef enum AvsEnum
{
	AvsCmd_GetInfo=1,
	AvsCmd_SendInfo,
	AvsCmd_GetFrame,
	AvsCmd_SendFrame,
	AvsCmd_Quit
};

typedef struct avsInfo
{
	uint32_t width;
	uint32_t height;
	uint32_t fps1000;
	uint32_t nbFrames;
}avsyInfo;
#endif


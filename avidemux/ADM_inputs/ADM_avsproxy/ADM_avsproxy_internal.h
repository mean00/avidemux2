/*

(c) Mean 2006
*/
#ifndef AVS_PROXY_INTERNAL_H
typedef enum 
{
    AvsCmd_GetInfo=1,
    AvsCmd_SendInfo,
    AvsCmd_GetFrame,
    AvsCmd_SendFrame,
    AvsCmd_Quit
}AvsEnum;

typedef struct avsInfo
{
    uint32_t width;
    uint32_t height;
    uint32_t fps1000;
    uint32_t nbFrames;
}avsInfo;

typedef struct SktHeader
{
    uint32_t cmd;
    uint32_t frame;
    uint32_t payloadLen;
    uint32_t magic;
}SktHeader;


#endif

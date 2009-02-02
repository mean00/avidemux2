/**
    \file ADM_videoCopy
    \brief Wrapper 
    (c) Mean 2008/GPLv2

*/

#include "ADM_default.h"
#include "ADM_videoCopy.h"
#include "ADM_editor/ADM_edit.hxx"

extern ADM_Composer *video_body; // Fixme!
/**
    \fn ADM_videoStreamCopy
*/
ADM_videoStreamCopy::ADM_videoStreamCopy()
{
    aviInfo info;
    video_body->getVideoInfo(&info);
    width=info.width;
    height=info.height;
    fourCC=info.fcc;
    averageFps1000=info.fps1000;
    isCFR=false;
    start=0;
    end=100; // FIXME

}
/**
    \fn ADM_videoStreamCopy
*/
ADM_videoStreamCopy::~ADM_videoStreamCopy()
{

}
/**
    \fn getExtraData
*/
bool     ADM_videoStreamCopy::getExtraData(uint32_t *extraLen, uint8_t **extraData)
{

  return video_body->getExtraHeaderData(extraLen,extraData);
}
 
/**
    \fn getPacket
*/
bool  ADM_videoStreamCopy::getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,
                    uint64_t *pts,uint64_t *dts,
                    uint32_t *flags)
{
    image.data=data;
    if(false==video_body->getCompressedPicure(start,&image))
    {
            printf("[StreamCopy] Get packet failed for frame %d\n",start);
            return false;
    }
    *len=image.dataLength;
    ADM_assert(*len<maxLen);
    *pts=image.demuxerPts;
    *dts=image.demuxerDts;
    *flags=image.flags;
    start++;
    return true;
}
/**
    \fn getVideoDuration
*/
uint64_t        ADM_videoStreamCopy::getVideoDuration(void)
{
    return video_body->getVideoDuration();

}
     

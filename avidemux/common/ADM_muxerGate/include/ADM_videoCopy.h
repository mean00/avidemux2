/**

*/
#ifndef ADM_VIDEOCOPY_H
#define ADM_VIDEOCOPY_H
#include "ADM_muxer.h"
/**
    \class ADM_videoStream

*/
#include "ADM_compressedImage.h"

class ADM_videoStreamCopy: public ADM_videoStream
{
protected:
            uint64_t startTimePts,startTimeDts;
            uint64_t endTimePts;
            uint32_t currentFrame;
            ADMCompressedImage image;
            bool eofMet;
            uint64_t  rescaleTs(uint64_t in);
public:
             ADM_videoStreamCopy(uint64_t startTime,uint64_t endTime);
    virtual ~ADM_videoStreamCopy();

virtual     bool     getPacket(ADMBitstream *out);
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
virtual     bool     providePts(void);
virtual     uint64_t getVideoDuration(void);
virtual     uint64_t getStartTime(void);
};

#endif

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
            uint32_t start,end;
            ADMCompressedImage image;
public:
             ADM_videoStreamCopy();
    virtual ~ADM_videoStreamCopy();

virtual     bool     getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,uint64_t *pts,uint64_t *dts,
                                    uint32_t *flags);
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
virtual     bool     providePts(void) {return false;}
virtual     uint64_t getVideoDuration(void);
};
#endif

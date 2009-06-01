/**

*/
#ifndef ADM_VIDEOPROCESS_H
#define ADM_VIDEOPROCESS_H
#include "ADM_muxer.h"
/**
    \class ADM_videoStream

*/
#include "ADM_compressedImage.h"
#include "ADM_coreVideoEncoder.h"
/**
    \class ADM_videoStreamProcess
    \brief Wrapper around encoder

*/
class ADM_videoStreamProcess: public ADM_videoStream
{
protected:
            
            ADMBitstream         *bitstream;
            ADM_coreVideoEncoder *encoder;
public:
             ADM_videoStreamProcess(ADM_coreVideoEncoder *encoder);
    virtual ~ADM_videoStreamProcess();

virtual     bool     getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,uint64_t *pts,uint64_t *dts,
                                    uint32_t *flags);
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
virtual     bool     providePts(void) {return false;}
virtual     uint64_t getVideoDuration(void);
};

#endif

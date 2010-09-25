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
            uint64_t  rewindTime;

            bool      rewind(void);
public:
             ADM_videoStreamCopy(uint64_t startTime,uint64_t endTime);
    virtual ~ADM_videoStreamCopy();

virtual     bool     getPacket(ADMBitstream *out);
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
virtual     bool     providePts(void);
virtual     uint64_t getVideoDuration(void);
virtual     uint64_t getStartTime(void);
};
/**
        \fn ADM_videoStreamCopyFromAnnexB
        \brief Same as copy but does annexB->mp4/iso on the fly
*/
class ADM_videoStreamCopyFromAnnexB : public ADM_videoStreamCopy
{
protected:
#define ADM_COPY_FROM_ANNEX_B_SIZE (1920*1200*3)
        uint8_t         buffer[ADM_COPY_FROM_ANNEX_B_SIZE];
        ADMBitstream    *myBitstream;
        uint8_t         *myExtra;
        uint32_t        myExtraLen;
        bool            findNalu(int nalu,uint8_t *start,uint8_t *end,uint8_t **outPtr,uint32_t *outLen);
        bool            compactNalus(ADMBitstream *out);
        int             convertFromAnnexB(uint8_t *inData,uint32_t inSize,
                                                      uint8_t *outData,uint32_t outMaxSize);

public:
                        ADM_videoStreamCopyFromAnnexB(uint64_t startTime,uint64_t endTime);
        virtual         ~ADM_videoStreamCopyFromAnnexB();
        virtual bool    getPacket(ADMBitstream *out);
        virtual bool    getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
};

#endif

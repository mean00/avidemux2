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
            bool sanitizeDts;
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
        bool            _init;
        bool            h265;
        bool            compactNalus(ADMBitstream *out);
        int             convertFromAnnexB(uint8_t *inData,uint32_t inSize,
                                                      uint8_t *outData,uint32_t outMaxSize);

public:
                        ADM_videoStreamCopyFromAnnexB(uint64_t startTime,uint64_t endTime);
        virtual         ~ADM_videoStreamCopyFromAnnexB();
        virtual bool    getPacket(ADMBitstream *out);
        virtual bool    getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
        bool            initOk() {return _init;}
protected:
        bool            extractExtraDataH264();
        bool            extractExtraDataH265();
private:
        bool            extractExtraDataH264Internal(void);
        bool            extractExtraDataH265Internal(void);
};
/**
        \fn ADM_videoStreamCopyToAnnexB
        \brief Same as copy but does mp4->annexB on the fly
*/
class ADM_videoStreamCopyToAnnexB : public ADM_videoStreamCopy
{
protected:
#define ADM_COPY_FROM_ANNEX_B_SIZE (1920*1200*3)
        uint8_t         buffer[ADM_COPY_FROM_ANNEX_B_SIZE];
        ADMBitstream    *myBitstream;
        uint8_t         *myExtra;
        uint32_t        myExtraLen;
        void            *bsfContext;    
        void            *codecContext;

public:
                        ADM_videoStreamCopyToAnnexB(uint64_t startTime,uint64_t endTime);
        virtual         ~ADM_videoStreamCopyToAnnexB();
        virtual bool    getPacket(ADMBitstream *out);
        virtual bool    getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
protected:
        bool extractExtraDataH264();
        bool extractExtraDataH265();
};
/**
        \fn ADM_videoStreamCopyAudRemover
        \brief  Remove AUD Nalu units
*/
class ADM_videoStreamCopyAudRemover : public ADM_videoStreamCopy
{
protected:
        bool            h265;
        int             carryover;
        uint8_t         scratchpad[ADM_COPY_FROM_ANNEX_B_SIZE];
public:
                        ADM_videoStreamCopyAudRemover(uint64_t startTime,uint64_t endTime);
        virtual         ~ADM_videoStreamCopyAudRemover();
        virtual bool    getPacket(ADMBitstream *out);
};
/**
        \fn ADM_videoStreamCopySeiInjector
        \brief If available, inject SEI message containing x264 version into the first access unit of a H.264 stream
*/
class ADM_videoStreamCopySeiInjector : public ADM_videoStreamCopy
{
protected:
#define ADM_H264_MAX_SEI_LENGTH 2048
        uint8_t         seiBuf[ADM_H264_MAX_SEI_LENGTH];
        uint32_t        seiLen;
        uint32_t        nalSize;
public:
                        ADM_videoStreamCopySeiInjector(uint64_t startTime,uint64_t endTime);
        virtual         ~ADM_videoStreamCopySeiInjector();
        virtual bool    getPacket(ADMBitstream *out);
};
#endif

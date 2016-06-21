/**
   \file dmxtsPacket.h
    \brief demuxer for TS stream
    (C) Mean fixounet@free.fr 2003-2009

*/

#ifndef DMXtsPacket_H
#define DMXtsPacket_H
#include <vector>
using std::vector;
#include "dmxPacket.h"
#include "ADM_tsPatPmt.h"
#include "ADM_Video.h"


#define TS_MARKER       0x47
#define TS_PACKET_LEN   188
#define TS_PSI_MAX_LEN  1024
#define TS_PES_MAX_LEN  (10*1024)
/**
    \class TSpacketInfo
*/
class TSpacketInfo
{
public:
    uint32_t    pid;
    uint32_t    payloadSize;
    bool        payloadStart;
    uint32_t    continuityCounter;
    uint8_t     payload[TS_PACKET_LEN];
    uint64_t    startAt;
};


/**
    \class TS_PSIpacketInfo
*/
class TS_PSIpacketInfo
{
public:
    
    uint32_t    pid;
    uint32_t    payloadSize;
    uint8_t     payload[TS_PSI_MAX_LEN];
    uint32_t    count;
    uint32_t    countMax;
};

/**
    \class TS_PESpacket
    \brief Handle exactly one PES packet
            It is assumed PES packet starts & ends at TS packet boundaries
*/
#define TS_PES_PACKET_MIN_SIZE 32
class TS_PESpacket
{
public:
    uint32_t    pid;
    uint32_t    payloadSize;
    uint32_t    payloadLimit;
    uint32_t    offset;
    uint8_t     *payload;
    uint64_t    pts;
    uint64_t    dts;
    uint64_t    startAt;
    bool        fresh; // True if we just filled it with a new packet
                TS_PESpacket(uint32_t pid)
                {
#define PES_DEF_SIZE (5*1024)
                    payload=(uint8_t *)ADM_alloc(PES_DEF_SIZE);
                    payloadLimit=PES_DEF_SIZE;
                    offset=0;
                    payloadSize=0;
                    this->pid=pid;
                }
                ~TS_PESpacket()
                {
                    ADM_dealloc(payload);
                    payload=NULL;
                }
    bool        expandPayload()
		{
                        payloadLimit=payloadLimit*2+TS_PES_PACKET_MIN_SIZE;
			uint8_t *newPayload=(uint8_t *)ADM_alloc(payloadLimit);
			memcpy(newPayload,payload,payloadSize);
			ADM_dealloc(payload);
			payload=newPayload;
			return true;

		}
    bool        addData(uint32_t len,uint8_t *data)
                {
                    if((len+payloadSize+TS_PES_PACKET_MIN_SIZE)>payloadLimit)
                    {
				expandPayload();
                    }
                    memcpy(payload+payloadSize,data,len);
                    payloadSize+=len;
                    return true;
                }
     bool empty(void)
     {
            payloadSize=0;
            return true;
     }
     bool pushByte(uint8_t byte)
     {
            if((payloadSize+TS_PES_PACKET_MIN_SIZE)>=payloadLimit)
            {
			expandPayload();
            }
            payload[payloadSize++]=byte;
            return true;
     }
};

/**
    \class tsPacket
*/
class tsPacket : public ADMMpegPacket
{
protected:

    uint32_t            extraCrap;
public:
                        tsPacket(void);
    virtual            ~tsPacket();
    virtual bool        open(const char *filenames,FP_TYPE append);
    virtual bool        close(void);
    virtual bool        getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
            bool        getNextPid(int *pid);
    virtual uint64_t    getPos(void);
    virtual bool        setPos(uint64_t pos);
protected:
    
    bool                getSinglePacket(uint8_t *buffer);
    bool                decodePesHeader(TS_PESpacket *pes);
    virtual bool        updateStats(uint8_t *data);
public:
    virtual bool                getNextPacket_NoHeader(uint32_t pid,TSpacketInfo *pkt,bool psi);

    bool                getNextPSI(uint32_t pid,TS_PSIpacketInfo *psi);
    bool                getNextPES(TS_PESpacket *pes);
    
};
/**
    \class tsPacketLinear
*/
#define ADM_PACKET_LINEAR 10*1024
#define TS_PACKET_INLINE
class tsPacketLinear : public tsPacket
{
protected:
        TS_PESpacket *pesPacket;
        bool     eof;
        bool     refill(void);
        uint64_t oldStartAt;
        uint32_t oldBufferLen;
        uint64_t oldBufferPts;
        uint64_t oldBufferDts;
        uint32_t consumed;

public:
                tsPacketLinear(uint32_t pid);
                ~tsPacketLinear();
        uint32_t getConsumed(void);
        bool     stillOk(void) {return !eof;};
        bool     invalidatePtsDts(void);
#ifndef TS_PACKET_INLINE
        uint8_t  readi8();
        uint16_t readi16();

#else
/**
    \fn readi8
*/
uint8_t readi8(void)
{
    consumed++;
    if(pesPacket->offset<pesPacket->payloadSize)
    {
        return pesPacket->payload[pesPacket->offset++];
    }
    if(false==refill()) 
    {
        eof=1;
        return 0;
    }
    return pesPacket->payload[pesPacket->offset++];
    
}

/**
    \fn readi16
*/
uint16_t readi16(void)
{
    if(pesPacket->offset+1<pesPacket->payloadSize)
    {
        uint8_t *r=pesPacket->payload+pesPacket->offset;
        uint16_t v=(r[0]<<8)+r[1];;
        
        pesPacket->offset+=2;
        consumed+=2;
        return v;
    }
    return (readi8()<<8)+readi8();
}
#endif

        uint32_t readi32();
        bool     sync(uint8_t *pid);
        bool    read(uint32_t len, uint8_t *buffer);
        bool    forward(uint32_t v);
        
        bool    getInfo(dmxPacketInfo *info);
        bool    seek(uint64_t packetStart, uint32_t offset);
        bool    changePid(uint32_t pid) ;
        bool    setConsumed(uint32_t v);
};

/**
    \class packetTSStats
*/
typedef struct
{
    uint32_t pid;
    uint32_t count;
    uint32_t size;
    
    uint64_t startAt;
    uint32_t startCount;
    uint32_t startSize;
    uint64_t startDts;
}packetTSStats;

/**
        \class tsPacketLinearTracker
        \brieg similar to tsPacketLinear, but also keeps stat of others tracks. Needed to index audio
*/
class tsPacketLinearTracker : public tsPacketLinear
{
protected:
        
        TS_PESpacket *otherPes;
        packetTSStats *stats;
        uint32_t      totalTracks;
public:
                tsPacketLinearTracker(uint32_t videoPid,listOfTsAudioTracks *audioTracks);
                ~tsPacketLinearTracker();
        bool    getStats(uint32_t *nb,packetTSStats **stats);
virtual bool    updateStats(uint8_t *data);
        int     findStartCode(void);
        int     findStartCode2(bool &fourBytes);
};
#endif

/**
   \file dmxtsPacket.h
    \brief demuxer for TS stream
    (C) Mean fixounet@free.fr 2003-2009

*/

#ifndef DMXtsPacket_H
#define DMXtsPacket_H

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
                    payload=(uint8_t *)ADM_alloc(2048);
                    payloadLimit=2048;
                    offset=0;
                    payloadSize=0;
                    this->pid=pid;
                }
                ~TS_PESpacket()
                {
                    ADM_dealloc(payload);
                    payload=NULL;
                }
    bool        addData(uint32_t len,uint8_t *data)
                {
                    if(len+payloadSize>payloadLimit)
                    {
                        payloadLimit*=2;
                        payload=(uint8_t *)ADM_realloc(payload,payloadLimit);
                    }
                    memcpy(payload+payloadSize,data,len);
                    payloadSize+=len;
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
    virtual uint64_t    getPos(void);
    virtual bool        setPos(uint64_t pos);
protected:
    
    bool                getSinglePacket(uint8_t *buffer);
    bool                decodePesHeader(TS_PESpacket *pes);
public:
    bool                getNextPacket_NoHeader(uint32_t pid,TSpacketInfo *pkt,bool psi);

    bool                getNextPSI(uint32_t pid,TS_PSIpacketInfo *psi);
    bool                getNextPES(TS_PESpacket *pes);

};
/**
    \class tsPacketLinear
*/
#define ADM_PACKET_LINEAR 10*1024

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
        uint8_t  readi8();
        uint16_t readi16();
        uint32_t readi32();
        bool     sync(uint8_t *pid);
        bool    read(uint32_t len, uint8_t *buffer);
        bool    forward(uint32_t v);
        bool    stillOk(void) {return !eof;};
        bool    getInfo(dmxPacketInfo *info);
        bool    seek(uint64_t packetStart, uint32_t offset);
        bool    changePid(uint32_t pid) ;
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
        TS_PESpacket *pesPacket;
        TS_PESpacket *otherPes;
        packetTSStats *stats;
        uint32_t      totalTracks;
public:
                tsPacketLinearTracker(uint32_t nb,ADM_TS_TRACK *tracks);
                ~tsPacketLinearTracker();
        bool    getStats(uint32_t *nb,packetTSStats *stats);
};
#endif

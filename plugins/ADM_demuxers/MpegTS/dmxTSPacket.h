/**
   \file dmxtsPacket.h
    \brief demuxer for TS stream
    (C) Mean fixounet@free.fr 2003-2009

*/

#ifndef DMXtsPacket_H
#define DMXtsPacket_H

#include "dmxPacket.h"
#include "ADM_Video.h"

#define TS_MARKER       0x47
#define TS_PACKET_LEN   188
#define TS_PSI_MAX_LEN  1024
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
    \class tsPacket
*/
class tsPacket : public ADMMpegPacket
{
protected:
    uint32_t            extraCrap;
    uint8_t             getPacketInfo(uint8_t stream,uint8_t *substream,uint32_t *olen,uint64_t *opts,uint64_t *odts);
public:
                        tsPacket(void);
    virtual            ~tsPacket();
    virtual bool        open(const char *filenames,FP_TYPE append);
    virtual bool        close(void);
    virtual bool        getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
    virtual uint64_t    getPos(void);
    virtual bool        setPos(uint64_t pos);
protected:
    bool                getNextPacket_NoHeader(uint32_t pid,TSpacketInfo *pkt,bool psi);
    bool                getSinglePacket(uint8_t *buffer);
public:
    bool                getNextPSI(uint32_t pid,TS_PSIpacketInfo *psi);

};
/**
    \class tsPacketLinear
*/
#define ADM_PACKET_LINEAR 10*1024

class tsPacketLinear : public tsPacket
{
protected:
        uint8_t  myPid;
        uint64_t startAt;
        uint32_t bufferLen;
        uint64_t bufferPts;
        uint64_t bufferDts;
        uint32_t bufferIndex;
        uint8_t  buffer[ADM_PACKET_LINEAR];
        bool     eof;
        bool     refill(void);
        uint64_t oldStartAt;
        uint32_t oldBufferLen;
        uint64_t oldBufferPts;
        uint64_t oldBufferDts;
        uint32_t consumed;

public:
                tsPacketLinear(uint8_t pid);
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
    \class tsPacketLinearTracker
*/
typedef struct
{
    uint32_t count;
    uint32_t size;
    
    uint64_t startAt;
    uint32_t startCount;
    uint32_t startSize;
    uint64_t startDts;
}packetStats;

class tsPacketLinearTracker : public tsPacketLinear
{
protected:
      packetStats stats[256];

public:
                        tsPacketLinearTracker(uint8_t pid);
                        ~tsPacketLinearTracker();
         packetStats    *getStat(int intdex);
         bool           resetStats(void);
virtual  bool           getPacketOfType(uint8_t pid,uint32_t maxSize, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
};


#endif

/**
    dmxPSPacket.cpp

*/

#ifndef DMXPSPACKET_H
#define DMXPSPACKET_H

#include "dmxPacket.h"
#include "ADM_Video.h"

#define PS_PACKET_INLINE


/**
    \class psPacket
*/
class psPacket : public ADMMpegPacket
{
protected:
    uint8_t             getPacketInfo(uint8_t stream,uint8_t *substream,uint32_t *olen,uint64_t *opts,uint64_t *odts);
public:
                        psPacket(void);
    virtual            ~psPacket();
    virtual bool        open(const char *filenames,FP_TYPE append);
    virtual bool        close(void);
    virtual bool        getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
    virtual uint64_t    getPos(void);
    virtual bool        setPos(uint64_t pos);
};
/**
    \class psPacketLinear
*/
#define ADM_PACKET_LINEAR (300*1024) // TIVO

class psPacketLinear : public psPacket
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
                psPacketLinear(uint8_t pid);
                ~psPacketLinear();
        uint32_t getConsumed(void);
        bool     sync(uint8_t *pid);
        bool    read(uint32_t len, uint8_t *buffer);
        bool    forward(uint32_t v);
        bool    stillOk(void) {return !eof;};
        bool    getInfo(dmxPacketInfo *info);
        bool    seek(uint64_t packetStart, uint32_t offset);
        bool    changePid(uint32_t pid) ;
#ifndef PS_PACKET_INLINE
        uint8_t  readi8();
        uint16_t readi16();
        uint32_t readi32();
#else
        /**
            \fn readi8
        */
        uint8_t readi8(void)
        {
            consumed++;
            if(bufferIndex<bufferLen)
            {
                return buffer[bufferIndex++];
            }
            if(false==refill()) 
            {
                eof=1;
                return 0;
            }
            ADM_assert(bufferLen);
            bufferIndex=1;
            return buffer[0];
            
        }
        /**
            \fn readi16
        */
        uint16_t readi16(void)
        {
            if(bufferIndex+1<bufferLen)
            {
                uint16_t v=(buffer[bufferIndex]<<8)+buffer[bufferIndex+1];;
                bufferIndex+=2;
                consumed+=2;
                return v;
            }
            return (readi8()<<8)+readi8();
        }
        /**
            \fn readi32
        */
        uint32_t readi32(void)
        {
            if(bufferIndex+3<bufferLen)
            {
                uint8_t *p=buffer+bufferIndex;
                uint32_t v=(p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
                bufferIndex+=4;
                consumed+=4;
                return v;
            }
            return (readi16()<<16)+readi16();
        }
#endif
};
/**
    \class packetStats
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
/**
    \class psPacketLinearTracker
    \brief Same as psPacketLinear, but keep stats
*/
class psPacketLinearTracker : public psPacketLinear
{
protected:
        packetStats stats[256];
        uint64_t    lastVobuEnd;    // In 90 Khz tick
        uint64_t    nextVobuEnd;    // In 90 Khz tick
        uint64_t    lastVobuPosition; 
        uint64_t    nextVobuPosition; 
        bool        decodeVobuPCI(uint32_t size,uint8_t *data);
        bool        decodeVobuDSI(uint32_t size);


public:
        uint64_t        getLastVobuEndTime(void) {return lastVobuEnd;}
        uint64_t        getLastVobuPosition(void) {return lastVobuPosition;}
        uint64_t        getNextVobuPosition(void) {return nextVobuPosition;}
                        psPacketLinearTracker(uint8_t pid);
                        ~psPacketLinearTracker();
         packetStats    *getStat(int intdex);
         bool           resetStats(void);
virtual  bool           getPacketOfType(uint8_t pid,uint32_t maxSize, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
         int            findStartCode(void);
};


#endif

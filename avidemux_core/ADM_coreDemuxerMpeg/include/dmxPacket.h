/**
    \file dmxPacket.cpp

*/

#ifndef DMXPACKET_H
#define DMXPACKET_H

#include "dmx_io.h"

/**
    \struct dmxPacketInfo

*/
typedef struct
{
    uint64_t pts;
    uint64_t dts;
    uint64_t startAt;
    uint32_t offset;

}dmxPacketInfo;


/**
     \class ADMMpegPacket
     \brief Base class for mpeg packet
*/
class ADMMpegPacket
{
protected:
    int         doNoComplainAnyMore;
    fileParser  *_file;
    uint64_t    _size;


public:
                        ADMMpegPacket(void);
    virtual             ~ADMMpegPacket();
    virtual bool        open(const char *filenames,FP_TYPE append)=0;
    virtual bool        close(void)=0;
    virtual bool        getPacket(uint32_t maxSize, uint8_t *pid, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt)=0;
    virtual bool        getPacketOfType(uint8_t pid,uint32_t maxSize, uint32_t *packetSize,uint64_t *pts,uint64_t *dts,uint8_t *buffer,uint64_t *startAt);
    virtual uint64_t    getPos(void)=0;
    virtual bool        setPos(uint64_t pos)=0;

    uint64_t    getSize(void) { return _file->getSize();}


};


#endif
//EOF


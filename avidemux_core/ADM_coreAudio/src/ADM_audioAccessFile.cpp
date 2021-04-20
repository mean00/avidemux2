/**
    \file ADM_audioAccessFile
    \brief Source is a plain file

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioAccessFile.h"
/**
    \fn
    \brief
*/

ADM_audioAccessFile::ADM_audioAccessFile(const char *fileName,int offset)
{
        _offset=offset;
        fileLength=ADM_fileSize(fileName)-offset;
        _fd=ADM_fopen(fileName,"rb");
        ADM_assert(_fd);
}
/**
    \fn
    \brief
*/

ADM_audioAccessFile::~ADM_audioAccessFile()
{
        if(_fd) ADM_fclose(_fd);
        _fd=NULL;
}
/**
    \fn
    \brief
*/
bool      ADM_audioAccessFile::setPos(uint64_t pos)
{
    if(fseeko(_fd,_offset+pos,SEEK_SET))
        return false;
    return true;
}
/**
    \fn
    \brief
*/
uint64_t  ADM_audioAccessFile::getPos()
{
    uint64_t p=(uint64_t)ftello(_fd)-_offset;
    return p;
}
/**
    \fn
    \brief
*/

bool    ADM_audioAccessFile::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    if(!getPos()) *dts=0;
        else      *dts=ADM_NO_PTS;
    int n=fread(buffer,1,maxSize,_fd);
    *size=n;
 //   ADM_info("ExternalTrack : Request for %d bytes, maxSize=%d\n",*size,maxSize);
    if(n>0) return true;
        return false;
}

/* ################################################################## */

/**
    \fn ctor
*/
ADM_audioAccessFilePCM::ADM_audioAccessFilePCM(const char *fileName, int offset, WAVHeader *info)
    : ADM_audioAccessFile(fileName,offset)
{
    ADM_assert(info);
    ADM_assert(info->channels && info->channels <= MAX_CHANNELS);
    ADM_assert(info->frequency >= MIN_SAMPLING_RATE && info->frequency <= MAX_SAMPLING_RATE);
    switch(info->bitspersample)
    {
        case 8: case 16: case 24:
            break;
        default:
            ADM_error("Unsupported bit depth %u\n",info->bitspersample);
            ADM_assert(0);
            break;
    }
    uint16_t alignment = (info->bitspersample >> 3) * info->channels;
    if(info->blockalign != alignment)
    {
        ADM_warning("Block alignment mismatch: %u vs %u, using the latter.\n",info->blockalign,alignment);
        info->blockalign = alignment;
    }
    memcpy(&_hdr,info,sizeof(WAVHeader));
    double d = (double)fileLength / _hdr.blockalign;
    d *= 1000.;
    d /= _hdr.frequency;
    d *= 1000.;
    _duration = d;
}
/**
    \fn goToTime
*/
bool ADM_audioAccessFilePCM::goToTime(uint64_t usecs)
{
    double d = usecs;
    d /= 1000. * 1000.;
    d *= _hdr.frequency;
    uint64_t pos = d;
    pos *= _hdr.blockalign;
    return setPos(pos);
}
/**
    \fn getPacket
*/
bool ADM_audioAccessFilePCM::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize, uint64_t *dts)
{
    uint32_t maxSamplesPerPacket = _hdr.frequency / 100;
    uint64_t pos = getPos();
    if(pos % _hdr.blockalign)
    {
        ADM_warning("Unaligned access by %u bytes.\n",pos % _hdr.blockalign);
        pos /= _hdr.blockalign;
        pos++;
        pos *= _hdr.blockalign;
        if(!setPos(pos))
            return false;
    }
    double d = pos;
    d /= _hdr.blockalign;
    d *= 1000. * 1000.;
    d /= _hdr.frequency;
    *dts = d;
    if(maxSize / _hdr.blockalign < maxSamplesPerPacket)
        maxSamplesPerPacket = maxSize / _hdr.blockalign;
    uint32_t blocks = fread(buffer, _hdr.blockalign, maxSamplesPerPacket, _fd);
    *size = blocks * _hdr.blockalign;
    return !!blocks;
}
// EOF


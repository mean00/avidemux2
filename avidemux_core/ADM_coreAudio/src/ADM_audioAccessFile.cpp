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
    fseeko(_fd+_offset,pos,SEEK_SET);
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
    if(n>0) return true;
        return false;
}

// EOF


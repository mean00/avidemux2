/***************************************************************************
                          ADM_metaToFile.h  -  description
                             -------------------
        Demuxer indexing (meta data) to file helper

    copyright            : (C) 2024 by szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include "ADM_default.h"
#include "ADM_metaToFile.h"

#define ALLOCATION_GROWTH       (1048576)       // 1MB
#define INDEX_SIZE_LIMIT    (1073741824ULL)     // max 1GB index file

enum EntryTypes {
    ET_ZERO = 0,
    ET_ONE,
    ET_NEG_ONE,
    ET_UNUSED1,
    ET_FALSE,
    ET_TRUE,
    ET_U8,
    ET_I8,
    ET_U16,
    ET_I16,
    ET_U24,
    ET_I24,
    ET_U32,
    ET_I32,
    ET_U40,
    ET_I40,
    ET_U64,
    ET_I64,
    ET_BYTE_ARRAY8,
    ET_BYTE_ARRAY16,
    ET_BYTE_ARRAY32,
    ET_VARIABLE8,
    ET_VARIABLE16,
    ET_VARIABLE32,
    

    ET_LAST_INVALID,
};

metaToFile::metaToFile(const std::string &idxFileName, uint64_t videoFileSize, const char * magic8, uint32_t idxVersion)
{
    ADM_assert(ET_LAST_INVALID <= 256);
    
    _initialized = false;
    if (idxFileName.length() < 4) return;
    if (videoFileSize == 0) return;
    ADM_assert(magic8);
    ADM_assert(strlen(magic8) == 8);
    ADM_assert(idxVersion > 0);
    
    _idxFileName = idxFileName;
    _videoFileSize = videoFileSize;
    memcpy(_magic8, magic8, 8);
    _idxVersion = idxVersion;
    _loadBuf = NULL;            // new & delete !
    _loadBufLen = 0;
    _readPtr = 0;
    _storeBuf = NULL;           // malloc & realloc & free !
    _storeBufAlloc = 0;
    _writePtr = 0;
    
    _initialized = true;
}

metaToFile::~metaToFile()
{
    if (_loadBuf) delete [] _loadBuf;
    if (_storeBuf) free(_storeBuf);
}



void metaToFile::checkReadPtr(uint32_t ahead)
{
    if ((_readPtr + ahead) >=_loadBufLen) throw "Prevented buffer overrun";
}

uint8_t metaToFile::readNextByte(void)
{
    checkReadPtr();
    return _loadBuf[_readPtr++];
}

uint64_t metaToFile::bytesToU64(uint8_t * bytes)
{
    uint64_t tmp;
    memcpy(&tmp, bytes, 8);
    return R64(tmp);
}


void metaToFile::writeByte(uint8_t byte)
{
    if (_writePtr >= _storeBufAlloc)
    {
        _storeBufAlloc += ALLOCATION_GROWTH;
        if (_storeBufAlloc > INDEX_SIZE_LIMIT) throw "Index size too large";
        uint8_t * newBuf = (uint8_t *)realloc(_storeBuf, _storeBufAlloc);       // realloc is the most efficient on proper OSes
        if (newBuf == NULL) throw "Memory allocation failure";
        _storeBuf = newBuf;
    }
    
    _storeBuf[_writePtr] = byte;
    _writePtr += 1;
}

void metaToFile::writeHeaderU64(uint64_t x)
{
    uint8_t tmp[8];
    memcpy(tmp, &x, 8);
    for (int i=0; i<8; i++)
    {
        writeByte(tmp[i]);
    }
}

void metaToFile::loadIndexFile(void)
{
    if (!_initialized) throw "metaToFile error";
    FILE * f = ADM_fopen(_idxFileName.c_str(), "rb");
    if (!f) throw "Unable to open the index file";
    
    try
    {

        uint64_t indexSize = ADM_fileSize(_idxFileName.c_str()); 
        if (indexSize < 40) throw "Index size too small";

        uint8_t tmpBuf[8];
        uint64_t tmpU64;

        if (ADM_fread(tmpBuf, 8, 1, f)!=1) throw "Cannot read index magic";
        if (memcmp(tmpBuf, _magic8, 8)) throw "Index magic1 is wrong";
        
        if (ADM_fread(&tmpU64, sizeof(uint64_t) , 1, f)!=1) throw "Cannot read index version";
        if (tmpU64 != _idxVersion) throw "Index version mismatch";

        if (ADM_fread(&tmpU64, sizeof(uint64_t), 1, f)!=1) throw "Cannot read file size";
        if (tmpU64 != _videoFileSize) throw "File size mismatch";

        indexSize -= 8+2*sizeof(uint64_t);
        if (indexSize < 8) throw "Remaning index size too small";
        if (indexSize > INDEX_SIZE_LIMIT) throw "Index size too large";
        
        _loadBuf = new uint8_t[indexSize+65536];

        if (ADM_fread(_loadBuf, indexSize, 1, f)!=1) throw "Cannot read index file";

        _loadBufLen = (indexSize - 8);
        if (memcmp(_loadBuf + _loadBufLen, _magic8, 8)) throw "Index magic2 is wrong";
    }
    catch(char const * e)
    {
        ADM_fclose(f);
        throw e;
    }
}

void metaToFile::createIndexFile(void)
{
    if (!_initialized) throw "metaToFile error";
    if (ADM_fileExist(_idxFileName.c_str())) throw "Index file already exist";

    for (int i=0; i<8; i++)
    {
        writeByte(_magic8[i]);
    }    
    writeHeaderU64(_idxVersion);
    writeHeaderU64(_videoFileSize);
}

void metaToFile::finishIndexFile(void)
{
    for (int i=0; i<8; i++)
    {
        writeByte(_magic8[i]);
    }
    
    FILE *f = ADM_fopen(_idxFileName.c_str(), "wb");
    if (!f) throw "Unable to open the index file";
    
    try
    {
         if (ADM_fwrite(_storeBuf, _writePtr,1,f) != 1) throw "File write error";
         ADM_info("Index written, %u bytes.\n", _writePtr);
         ADM_fclose(f);
    }
    catch(char const * e)
    {
        ADM_fclose(f);
        throw e;
    }    
}


uint64_t metaToFile::readUnsignedInt(void)
{
    uint8_t decodeBuf[8] = {0};
    
    switch(readNextByte())
    {
        case ET_ZERO:
            return 0;
        case ET_ONE:
            return 1;
        case ET_U8:
            for (int i=0; i<1; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        case ET_U16:
            for (int i=0; i<2; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        case ET_U24:
            for (int i=0; i<3; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        case ET_U32:
            for (int i=0; i<4; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        case ET_U40:
            for (int i=0; i<5; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        case ET_U64:
            for (int i=0; i<8; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        default:
            throw "Invalid entry type";
            return 0;
    }
    
    return bytesToU64(decodeBuf);
}

int64_t metaToFile::readSignedInt(void)
{
    uint8_t decodeBuf[8] = {0};
    
    switch(readNextByte())
    {
        case ET_ZERO:
            return 0;
        case ET_ONE:
            return 1;
        case ET_NEG_ONE:
            return -1;
        case ET_I8:
            for (int i=0; i<1; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            if (decodeBuf[0] & 0x80)
            {
                for (int i=1; i<8; i++) decodeBuf[i] = 0xFF;
            }
            break;
        case ET_I16:
            for (int i=0; i<2; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            if (decodeBuf[1] & 0x80)
            {
                for (int i=2; i<8; i++) decodeBuf[i] = 0xFF;
            }
            break;
        case ET_I24:
            for (int i=0; i<3; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            if (decodeBuf[2] & 0x80)
            {
                for (int i=3; i<8; i++) decodeBuf[i] = 0xFF;
            }
            break;
        case ET_I32:
            for (int i=0; i<4; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            if (decodeBuf[3] & 0x80)
            {
                for (int i=4; i<8; i++) decodeBuf[i] = 0xFF;
            }
            break;
        case ET_I40:
            for (int i=0; i<5; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            if (decodeBuf[4] & 0x80)
            {
                for (int i=5; i<8; i++) decodeBuf[i] = 0xFF;
            }
            break;
        case ET_I64:
            for (int i=0; i<8; i++)
            {
                decodeBuf[i] = readNextByte();
            }
            break;
        default:
            throw "Invalid entry type";
            return 0;
    }
    
    return static_cast<int64_t>(bytesToU64(decodeBuf));    
}

bool metaToFile::readBool(void)
{
    switch(readNextByte())
    {
        case ET_FALSE:
            return false;
        case ET_TRUE:
            return true;
        default:
            throw "Invalid entry type";
            return false;
    }
}
uint32_t metaToFile::readByteArray(uint8_t * array, uint32_t maxLen)
{
    uint32_t len = 0;
    
    switch(readNextByte())
    {
        case ET_BYTE_ARRAY8:
            len = readNextByte();
            break;
        case ET_BYTE_ARRAY16:
            len = readNextByte();
            len += readNextByte() << 8;
            break;
        case ET_BYTE_ARRAY32:
            len = readNextByte();
            len += readNextByte() << 8;
            len += readNextByte() << 16;
            len += readNextByte() << 24;
            break;
        default:
            throw "Invalid entry type";
            return 0;
    }
    
    if (len > maxLen) throw "Invalid array size";
    
    for (int i=0; i<len; i++)
    {
        array[i] = readNextByte();
    }
    
    return len;
}

uint32_t metaToFile::readByteArrayWithNew(uint8_t ** array, uint32_t maxLen)
{
    uint32_t len = 0;
    
    switch(readNextByte())
    {
        case ET_BYTE_ARRAY8:
            len = readNextByte();
            break;
        case ET_BYTE_ARRAY16:
            len = readNextByte();
            len += readNextByte() << 8;
            break;
        case ET_BYTE_ARRAY32:
            len = readNextByte();
            len += readNextByte() << 8;
            len += readNextByte() << 16;
            len += readNextByte() << 24;
            break;
        default:
            throw "Invalid entry type";
            return 0;
    }
    
    if (len > maxLen) throw "Invalid array size";
    
    if (*array) delete [] *array;
    *array = new uint8_t[len];
    
    for (int i=0; i<len; i++)
    {
        (*array)[i] = readNextByte();
    }
    
    return len;
}

uint32_t metaToFile::readByteArrayWithMalloc(uint8_t ** array, uint32_t maxLen)
{
    uint32_t len = 0;
    
    switch(readNextByte())
    {
        case ET_BYTE_ARRAY8:
            len = readNextByte();
            break;
        case ET_BYTE_ARRAY16:
            len = readNextByte();
            len += readNextByte() << 8;
            break;
        case ET_BYTE_ARRAY32:
            len = readNextByte();
            len += readNextByte() << 8;
            len += readNextByte() << 16;
            len += readNextByte() << 24;
            break;
        default:
            throw "Invalid entry type";
            return 0;
    }
    
    if (len > maxLen) throw "Invalid array size";
    
    if (*array) free(*array);
    *array = (uint8_t *)malloc(len);
    
    for (int i=0; i<len; i++)
    {
        (*array)[i] = readNextByte();
    }
    
    return len;
}

void metaToFile::readVariable(void * variable, uint32_t size)
{
    uint32_t len = 0;
    
    switch(readNextByte())
    {
        case ET_VARIABLE8:
            len = readNextByte();
            break;
        case ET_VARIABLE16:
            len = readNextByte();
            len += readNextByte() << 8;
            break;
        case ET_VARIABLE32:
            len = readNextByte();
            len += readNextByte() << 8;
            len += readNextByte() << 16;
            len += readNextByte() << 24;
            break;
        default:
            throw "Invalid entry type";
            return;
    } 
    
    if (len != size) throw "Invalid variable size";
    for (int i=0; i<len; i++)
    {
        ((uint8_t*)variable)[i] = readNextByte();
    }
}


void metaToFile::writeUnsignedInt(uint64_t x)
{
    if (x == 0)
    {
        writeByte(ET_ZERO);
    } else
    if (x == 1)
    {
        writeByte(ET_ONE);
    } else
    if (x < 256ull)
    {
        writeByte(ET_U8);
        writeByte(x);
    } else
    if (x < 65536ull)
    {
        writeByte(ET_U16);
        for (int i=0; i<2; i++)
        {
            writeByte(x & 0xFFull);
            x >>= 8;
        }
    } else
    if (x < 16777216ull)
    {
        writeByte(ET_U24);
        for (int i=0; i<3; i++)
        {
            writeByte(x & 0xFFull);
            x >>= 8;
        }
    } else
    if (x < 4294967296ull)
    {
        writeByte(ET_U32);
        for (int i=0; i<4; i++)
        {
            writeByte(x & 0xFFull);
            x >>= 8;
        }
    } else
    if (x < 1099511627776ull)
    {
        writeByte(ET_U40);
        for (int i=0; i<5; i++)
        {
            writeByte(x & 0xFFull);
            x >>= 8;
        }
    }
    else
    {
        writeByte(ET_U64);
        for (int i=0; i<8; i++)
        {
            writeByte(x & 0xFFull);
            x >>= 8;
        }
    }
}

void metaToFile::writeSignedInt(int64_t x)
{
    uint64_t ux = static_cast<uint64_t>(x);
    int64_t absX = x;
    if (absX < 0) absX = 0ll - absX;
    uint64_t ax = static_cast<uint64_t>(absX);
    
    if (x == 0)
    {
        writeByte(ET_ZERO);
    } else
    if (x == 1)
    {
        writeByte(ET_ONE);
    } else
    if (x == -1ll)
    {
        writeByte(ET_NEG_ONE);
    } else
    if (ax < 128ull)
    {
        writeByte(ET_I8);
        writeByte(ux);
    } else
    if (ax < 32768ull)
    {
        writeByte(ET_I16);
        for (int i=0; i<2; i++)
        {
            writeByte(ux & 0xFFull);
            ux >>= 8;
        }
    } else
    if (ax < 8388608ull)
    {
        writeByte(ET_I24);
        for (int i=0; i<3; i++)
        {
            writeByte(ux & 0xFFull);
            ux >>= 8;
        }
    } else
    if (ax < 2147483648ull)
    {
        writeByte(ET_I32);
        for (int i=0; i<4; i++)
        {
            writeByte(ux & 0xFFull);
            ux >>= 8;
        }
    } else
    if (ax < 549755813888ull)
    {
        writeByte(ET_I40);
        for (int i=0; i<5; i++)
        {
            writeByte(ux & 0xFFull);
            ux >>= 8;
        }
    }
    else
    {
        writeByte(ET_I64);
        for (int i=0; i<8; i++)
        {
            writeByte(ux & 0xFFull);
            ux >>= 8;
        }
    }    
}

void metaToFile::writeBool(bool x)
{
    writeByte(x ? ET_TRUE : ET_FALSE);
}

void metaToFile::writeByteArray(uint8_t * array, uint32_t length)
{
    int lenBytes;
    if (length < 256)
    {
        lenBytes = 1;
        writeByte(ET_BYTE_ARRAY8);
    } else
    if (length < 65536)
    {
        lenBytes = 2;
        writeByte(ET_BYTE_ARRAY16);
    }
    else
    {
        lenBytes = 4;
        writeByte(ET_BYTE_ARRAY32);
    }
    uint32_t x = length;
    for (int i=0; i<lenBytes; i++)
    {
        writeByte(x & 0xFF);
        x >>= 8;
    }
    while (length--)
    {
        writeByte(*array++);
    }
}

void metaToFile::writeVariable(void * variable, uint32_t size)
{
    int lenBytes;
    if (size < 256)
    {
        lenBytes = 1;
        writeByte(ET_VARIABLE8);
    } else
    if (size < 65536)
    {
        lenBytes = 2;
        writeByte(ET_VARIABLE16);
    }
    else
    {
        lenBytes = 4;
        writeByte(ET_VARIABLE32);
    }
    uint32_t x = size;
    for (int i=0; i<lenBytes; i++)
    {
        writeByte(x & 0xFF);
        x >>= 8;
    }
    uint8_t * array = (uint8_t *)variable;
    while (size--)
    {
        writeByte(*array++);
    }    
}

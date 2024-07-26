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

#ifndef __META2FILE__
#define __META2FILE__

#ifdef _WIN32
#    include <windows.h>
#endif

#include "ADM_coreDemuxer6_export.h"
#include "ADM_inttype.h"


/**
    \class metaToFile
    \brief Helper class for most demuxers
*/
class ADM_COREDEMUXER6_EXPORT metaToFile
{
private:
        bool                    _initialized;
        std::string             _idxFileName;
        uint64_t                _videoFileSize;
        uint8_t                 _magic8[8];
        uint64_t                _idxVersion;
        uint8_t *               _loadBuf;
        uint32_t                _loadBufLen;
        uint32_t                _readPtr;
        uint8_t *               _storeBuf;
        uint32_t                _storeBufAlloc;
        uint32_t                _writePtr;
        
        
        void        checkReadPtr(uint32_t ahead = 0);
        uint8_t     readNextByte(void);
        uint64_t    bytesToU64(uint8_t * bytes);
        
        void        writeByte(uint8_t byte);
        void        writeHeaderU64(uint64_t x);

public:
                    metaToFile(const std::string &idxFileName, uint64_t videoFileSize, const char * magic8, uint32_t idxVersion);
                    ~metaToFile();
                    
        void        loadIndexFile(void);
        uint64_t    readUnsignedInt(void);
        int64_t     readSignedInt(void);
        bool        readBool(void);
        uint32_t    readByteArray(uint8_t * array, uint32_t maxLen = 4294967295ul);
        uint32_t    readByteArrayWithNew(uint8_t ** array, uint32_t maxLen = 4294967295ul);
        uint32_t    readByteArrayWithMalloc(uint8_t ** array, uint32_t maxLen = 4294967295ul);
        void        readVariable(void * variable, uint32_t size);
        
        void        createIndexFile(void);
        void        finishIndexFile(void);
        void        writeUnsignedInt(uint64_t x);
        void        writeSignedInt(int64_t x);
        void        writeBool(bool x);
        void        writeByteArray(uint8_t * array, uint32_t length);
        void        writeVariable(void * variable, uint32_t size);
};
#endif  // __META2FILE__

/**
    \file ADM_indexFile
    \brief Handle index file reading
    copyright            : (C) 2009 by mean
    email                : fixounet@free.fr
        
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_INDEXFILE_H
#define ADM_INDEXFILE_H
#include "ADM_coreDemuxerMpeg6_export.h"
#include <BVector.h>
#include "ADM_byteBuffer.h"
/**
    \class indexFile
    \brief Read a index file easily
*/
#define ADM_INDEX_FILE_VERSION 5
#define ADM_INDEX_BUFFER (20*1024)
/**
    \class dmxToken
*/
class ADM_COREDEMUXER6_EXPORT dmxToken
{
protected:
    char *name;
    char *value;
    FILE *file;
    void  purgeTokens(void);
public:
    
    dmxToken(const char *name,const char *value);
    ~dmxToken();
    char *getName(void);
    char *getValue(void);
    bool  isNumeric(void);
    uint32_t getAsNumber(void);
    uint64_t getAsNumber64(void);
};

/**
    \class indexFile
*/

class ADM_COREDEMUXER6_EXPORT indexFile
{
protected:
    ADM_byteBuffer buffer;
    dmxToken        *searchToken(const char *name);
    void            purgeTokens(void);

    FILE            *file;
    BVector         <dmxToken *> ListOfTokens;
public:

        indexFile();
        ~indexFile();
    bool open(const char *name);
    bool close(void);
    bool goToSection(const char *section);
    bool readSection(const char *section);
    uint64_t getAsUint64(const char *token);
    uint32_t getAsUint32(const char *token);
    uint32_t getAsHex(const char *token);
    char *getAsString(const char *token);
    bool  readString(uint32_t maxLen,uint8_t *buffer);

};

#endif

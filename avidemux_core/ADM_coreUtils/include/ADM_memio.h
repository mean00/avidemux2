/** *************************************************************************
    \fn ADM_memio.h
    \brief Handle buffer access through a class
                      
    copyright            : (C) 2012 by mean
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ADM_MEM_IO
#define ADM_MEM_IO

#include "ADM_coreUtils6_export.h"

/**
    \class ADMMemio
*/

class ADM_COREUTILS6_EXPORT ADMMemio
{
protected:
        uint8_t         *buffer;
        uint8_t         *cur;
        uint8_t         *tail;
public:
                         ADMMemio(int size);
        virtual         ~ADMMemio();
        int             size() const {return (int)(cur-buffer);};
        const uint8_t  *getBuffer() const {return buffer;}

        void            write32(uint32_t w);
        void            write16(uint16_t w);
        void            write8(uint8_t w);
        void            write(int len, const uint8_t *data);
        void            reset() {cur=buffer;}
};

#endif

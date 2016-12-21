/***************************************************************************
                          ADM_mpegparser.h  -  description
                             -------------------
    begin                : Tue Oct 15 2002
    copyright            : (C) 2002 by mean
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
#ifndef FPARSER_
#define FPARSER_
#include "ADM_coreDemuxerMpeg6_export.h"
#define DMX_BUFFER 1024*100
#include <BVector.h>

/*
        _off is the logical offset in the file
        _head is the logical offset of the 1st byte in _buffer
        _tail is the logical offset of the last byte in the buffer


*/
typedef enum 
{
        FP_PROBE=1,
        FP_DONT_APPEND=2,
        FP_APPEND=3
}FP_TYPE;
/**
    \class fdIo
    \brief Describe one file
*/
class ADM_COREDEMUXER6_EXPORT fdIo
{
public:
        FILE        *file;
        uint64_t    fileSize;
        uint64_t    fileSizeCumul;// Cumulative side from beginning =offset for the 1st byte in the file
        fdIo() {file=NULL;fileSize=0;fileSizeCumul=0;}
};
/**
    \class fileParser
    \brief helper class to read one logical file over several physical ones
*/
 class ADM_COREDEMUXER6_EXPORT fileParser
{
        private:
         
            uint8_t  *_buffer;
            uint64_t _off;              // Absolute offset
            
            uint32_t _curFd;        
            BVector <fdIo> listOfFd;
            uint64_t _head,_tail,_size;       
           
        public:
                                fileParser(void);
                                ~fileParser();                                         
                        uint8_t  open(const char *name,FP_TYPE *multi);
                        uint8_t  forward(uint64_t u);
                        uint8_t  sync(uint8_t *t );
                        uint8_t  syncH264(uint8_t *t );
                        uint8_t  getpos(uint64_t *o);
                        uint8_t  setpos(uint64_t o);                
                        uint64_t getSize( void ) ;
                        uint32_t read32(uint32_t l, uint8_t *buffer);
                        uint8_t  end(void) { return _off>=_size-1;};
						void hexDump(uint8_t *buf, int size);
                        uint8_t  peek8i(void); // Only call it once!!
#ifdef NO_INLINE_FP
                        uint32_t read32i(void );
                        uint16_t read16i(void );
                        uint8_t  read8i(void );
                        
                        

#else
uint32_t read32i(void )
{
       uint32_t v;
       uint8_t c[4];
       uint8_t *p;
        // case one, it fits in the buffer
        //
        if(_off+3<_tail)
        {
                p=&(_buffer[_off-_head]);
                _off+=4;
        }
        else
        {
               read32(4,c);
               p=c;
        }
       v= (p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3];
       return v;
}
uint16_t read16i(void )
{
  uint32_t v;
       uint8_t c[4];
       uint8_t *p;
        // case one, it fits in the buffer
        //
        if(_off+1<_tail)
        {
                p=&(_buffer[_off-_head]);
                _off+=2;
        }
        else
        {
               read32(2,c);
               p=c;
        }
       v= (p[0]<<8)+p[1];
       return v;
}
uint8_t read8i(void )
{
uint8_t r;
        if(_off<_tail)
        {
                r= _buffer[_off-_head];
                _off++;     
        }
        else
        {
                read32(1,&r);     
        }
        return r;
}
#endif


} ;


#endif

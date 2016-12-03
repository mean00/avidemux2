/***************************************************************************
                          Base class for Mpeg Demuxer
                             -------------------
                
    copyright            : (C) 2005 by mean
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

 #ifndef DMX_DMX_ES
 #define DMX_DMX_ES
 
#include "ADM_coreDemuxerMpeg6_export.h"
#include "dmx_demuxer.h"
 

#define ENDCHECK if(parser->end()) _lastErr=1;
class ADM_COREDEMUXER6_EXPORT dmx_demuxerES: public dmx_demuxer
 {
          protected : 
                  uint64_t stampAbs;
                  uint32_t consumed;
                  fileParser *parser;
                
          public:
                           dmx_demuxerES() ;
                virtual    ~dmx_demuxerES();             
                
                     uint8_t      open(const char *name);
                 
                
                  uint8_t         forward(uint32_t f);
                  uint8_t         stamp(void); 

                  uint64_t        elapsed(void);
                
                  uint8_t         getPos( uint64_t *abs,uint64_t *rel);
                  uint8_t         setPos( uint64_t abs,uint64_t  rel);
                
                  uint64_t        getSize( void) { return _size;}          
                
                
                  uint32_t        read(uint8_t *w,uint32_t len) {ENDCHECK;consumed+=len;return parser->read32(len,w);}
                  uint8_t         read8i(void)                  {ENDCHECK;consumed++;return parser->read8i();}
                  uint16_t        read16i(void)                 {ENDCHECK;consumed+=2;return parser->read16i();}
                  uint32_t        read32i(void)                 {ENDCHECK;consumed+=4;return parser->read32i();}
                
                  uint8_t         sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts);
                  uint8_t         syncH264( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts);
                  uint8_t         getStats(uint64_t *stat) {ADM_assert(0);}
};      
        

#endif

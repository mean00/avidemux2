/***************************************************************************
                          Base class for Mpeg Demuxer
                             -------------------

    copyright            : (C) 2005--2008 by mean
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

 #ifndef DMX_DMX
 #define DMX_DMX
 
#include "dmx_io.h"

#include "dmx_indexer.h"

typedef enum 
{
  DMX_PAYLOAD_ERROR=0,
  DMX_PAYLOAD_MPEG2=1,
  DMX_PAYLOAD_MPEG4,
  DMX_PAYLOAD_H264
}dmx_payloadType;
/**
    \class dmx_demuxer
    \brief low level mpeg demuxer, also used for the hack called MS-DVR

*/
class dmx_demuxer
 {
  protected : 
              
    uint64_t  _size;
    uint8_t   _lastErr;   // Very important : The derivated signals an error using that!
    
  public:
                            dmx_demuxer();
    virtual                 ~dmx_demuxer();	       
    
    virtual uint8_t         open(const char *name)=0;
    virtual uint8_t         hasAudio(void) { return 0;}                
        
    virtual uint8_t         forward(uint32_t f)=0;
    virtual uint8_t         stamp(void)=0; 
    virtual uint64_t        elapsed(void)=0;
    
    virtual uint8_t         getPos( uint64_t *abs,uint64_t *rel)=0;
    virtual uint8_t         setPos( uint64_t abs,uint64_t  rel)=0;
    
    virtual uint64_t        getSize( void) { return _size;}          
    virtual uint8_t         getStats(uint64_t *stat)=0;
    virtual uint8_t         getAllPTS(uint64_t *stat) {return 0;};
    
    virtual uint32_t        read(uint8_t *w,uint32_t len)=0;
    virtual uint8_t	        read8i(void)=0;
    virtual uint16_t        read16i(void)=0;
    virtual uint32_t        read32i(void)=0;

    virtual uint8_t         syncH264( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts, uint64_t *dts) {return 0;};

    virtual uint8_t         sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts, uint64_t *dts)=0;
    virtual uint8_t         changePid(uint32_t pid,uint32_t pes) {return 0;}
    // Read a PES packet
    virtual uint8_t         readPes(uint8_t *data, uint32_t *pesBlockLen, uint32_t *dts,uint32_t *pts) {return 0;}
};	
	

#endif

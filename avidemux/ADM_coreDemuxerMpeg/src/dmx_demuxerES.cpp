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

#include <string.h>
#include <math.h>

#include "ADM_default.h"

#include "dmx_demuxerEs.h"
 
dmx_demuxerES::dmx_demuxerES(void)
{
        consumed=0;
        parser=new fileParser();
        stampAbs=0;
}
dmx_demuxerES::~dmx_demuxerES()
{
        if(parser) delete parser;
        parser=NULL;
}
uint8_t dmx_demuxerES::open(const char *name)
{
FP_TYPE fp=FP_DONT_APPEND;
        if(! parser->open(name,&fp)) return 0;
        _size=parser->getSize();
        return 1;
}
uint8_t dmx_demuxerES::forward(uint32_t f)
{
        consumed+=f;
        return parser->forward(f);
}
uint8_t  dmx_demuxerES::stamp(void)
{
        consumed=0;
        parser->getpos(&stampAbs);
        stampAbs-=4;
}
uint64_t dmx_demuxerES::elapsed(void)
{
        return consumed;        
}
uint8_t  dmx_demuxerES::getPos( uint64_t *abs,uint64_t *rel)
{
        *rel=0;
        parser->getpos(abs);       
        return 1;
}
uint8_t dmx_demuxerES::setPos( uint64_t abs,uint64_t  rel)
{
               return parser->setpos(abs);
}
/*
        Sync on mpeg sync word, returns the sync point in abs/r
*/
uint8_t         dmx_demuxerES::sync( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
uint32_t val,hnt;
         *r=0;
                *pts=ADM_NO_PTS;
                *dts=ADM_NO_PTS;

                val=0;
                hnt=0;                  
                        
                // preload
                hnt=(read8i()<<16) + (read8i()<<8) +read8i();
                if(_lastErr)
                {
                        _lastErr=0;
                        printf("\n io error , aborting sync\n");
                        return 0;       
                }
                
                while((hnt!=0x00001))
                {
                                        
                        hnt<<=8;
                        val=read8i();                                   
                        hnt+=val;
                        hnt&=0xffffff;  
                                        
                        if(_lastErr)
                        {
                             _lastErr=0;
                            printf("\n io error , aborting sync\n");
                            return 0;
                         }
                                                                        
                }
                                
                *stream=read8i();
                parser->getpos(abs);
                *abs-=4;
                return 1;
}

/**
          \fn syncH264
          \brief search h264 startcode 00 00 00 01
*/
uint8_t         dmx_demuxerES::syncH264( uint8_t *stream,uint64_t *abs,uint64_t *r,uint64_t *pts,uint64_t *dts)
{
uint32_t val,hnt;
         *r=0;
                *pts=ADM_NO_PTS;
                *dts=ADM_NO_PTS;

                val=0;
                hnt=0;                  
                        
                // preload
                hnt=(read8i()<<24)+(read8i()<<16) + (read8i()<<8) +read8i();
                if(_lastErr)
                {
                        _lastErr=0;
                        printf("\n io error , aborting sync\n");
                        return 0;       
                }
                
                while((hnt!=1))
                {
                                        
                        hnt<<=8;
                        val=read8i();                                   
                        hnt+=val;
                        
                                        
                        if(_lastErr)
                        {
                             _lastErr=0;
                            printf("\n io error , aborting sync\n");
                            return 0;
                         }
                                                                        
                }
                                
                *stream=read8i();
                parser->getpos(abs);
                *abs-=5;
                return 1;
}

          
          

/***************************************************************************
                         DVB-T subtitle filter
    
    copyright            : (C) 2007 by mean
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

#ifndef ADM_VIDDVBSUB_H
#define ADM_VIDDVBSUB_H
#include "ADM_inputs/ADM_mpegdemuxer/dmx_mpegstartcode.h"
#include "ADM_inputs/ADM_mpegdemuxer/dmx_demuxerTS.h"

#include "ADM_codecs/ADM_ffmp43.h"
#include "ADM_videoFilter/ADM_vidVobSubBitmap.h"


#define READ_BUFFER_SIZE (64*1024)

class ADMVideoSubDVB : public AVDMGenericVideoStream 
{
protected:
        virtual char* printConf(void);
       
       
        
        decoderFFSubs *decoder;
        ADMCompressedImage *binary;
        dmx_demuxerTS *demuxer;
        AVSubtitle  sub;
        uint8_t     readBuffer[READ_BUFFER_SIZE];
        uint32_t    _inited;
        
public:
        ADMVideoSubDVB(AVDMGenericVideoStream *in, CONFcouple *conf);
        ADMVideoSubDVB(const char *fileName, uint32_t pid,uint32_t w,uint32_t h);
        virtual ~ADMVideoSubDVB();
        virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len, ADMImage *data,uint32_t *flags);
        uint8_t configure(AVDMGenericVideoStream *instream);
        uint8_t	getCoupledConf(CONFcouple **conf);
        uint8_t getNextBitmap(vobSubBitmap *data,uint32_t *pts);
        uint8_t init(const char  *tsFileName);
};
#endif
/************************************************/

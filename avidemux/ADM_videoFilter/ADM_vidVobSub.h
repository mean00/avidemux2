
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VOBSUB_V_
#define _VOBSUB_V_
#include "ADM_coreDemuxerMpeg/include/dmx_demuxerPS.h"
#define VS_MAXPACKET 128*1024

#define NOSUB 0xffffffff

#define MAX_DISPLAY_TIME 3000 // 3s

//************************************
typedef struct vobSubParam
{
        char            *subname;
        uint32_t        index;
        int32_t         subShift;
}vobSubParam;
//************************************
#include "ADM_videoFilter/ADM_vidVobSubBitmap.h"
//************************************
class  ADMVideoVobSub:public AVDMGenericVideoStream
 {

 protected:
        virtual char                    *printConf(void);
        uint8_t                         guessPalette(void);
        uint8_t                         readbyte(void);         /// Read a byte from buffer
        uint16_t                        readword(void);         /// Read a 16 bits word from buffer
        uint8_t                         forward(uint32_t v);    /// Read a 16 bits word from buffer
        uint8_t                         decodeRLE(uint32_t off,uint32_t start,uint32_t end);
        uint8_t                         setup(void);            /// Rebuild internal info
        uint8_t                         cleanup(void);          /// Destroy all internal info
        uint8_t                         paletteYUV( void );     /// Convert RGB Pallette to yuv
        uint8_t                         Palettte2Display( void ); /// Convert the RLE to YUV bitmap
        uint8_t                         handleSub( uint32_t idx );/// Decode a sub packet
        uint32_t                        lookupSub(uint64_t time);/// Return sub index corresponding to time
        
        dmx_demuxerPS                   *_parser;        
        uint8_t                         *_data;                 /// Data for packet
        VobSubInfo                      *_vobSubInfo;           /// Info of the index file
        vobSubParam                     *_param;
        vobSubBitmap                    *_original;              /// True size (..) depacked vobsub
        vobSubBitmap                    *_resampled;            /// Final one; to be blended in picture
        vobSubBitmap                    *_chromaResampled;      /// Same as above but shinked by 2
        uint32_t                        _x1,_y1,_x2,_y2;        /// sub boxing
        uint32_t                        _subW,_subH;
        uint8_t                         _displaying;            ///  Is display active
        uint32_t                        _curOffset;
        uint32_t                        _subSize;
        uint32_t                        _dataSize;              /// Size of the data chunk
        
        uint8_t                         _colors[4];             /// Colors palette
        uint8_t                         _alpha[4];              /// Colors alpha
        int16_t                         _YUVPalette[16];        /// Luma only
        uint32_t                        _currentSub;            ///
        uint32_t                        _initialPts;
 public:
    /* This 3 functions are used by OCR */
                        ADMVideoVobSub(  char *fileidx,uint32_t idx);
              vobSubBitmap *getBitmap(uint32_t nb,uint32_t *start, uint32_t *end,uint32_t *first,uint32_t *last);
              uint32_t     getNbImage( void);
    /* /ocr */ 
    
                        ADMVideoVobSub(  AVDMGenericVideoStream *in,CONFcouple *setup);
                        ~ADMVideoVobSub();
        virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                ADMImage *data,uint32_t *flags);
        virtual uint8_t getCoupledConf( CONFcouple **couples)           ;
        virtual uint8_t configure( AVDMGenericVideoStream *instream);
                                                        
};
#endif

/* 
   RTjpeg (C) Justin Schoeman 1998 (justin@suntiger.ee.up.ac.za)
   
   With modifications by:
   (c) 1998, 1999 by Joerg Walter <trouble@moes.pmnet.uni-oldenburg.de>
   and
   (c) 1999 by Wim Taymans <wim.taymans@tvd.be>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public  
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of   
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public   
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __RTJPEG_H__
#define __RTJPEG_H__

/*
 * Macros and definitions used internally to RTjpeg
 */

#define RTJPEG_FILE_VERSION 0
#define RTJPEG_HEADER_SIZE 12

#ifdef WORDS_BIGENDIAN
#define RTJPEG_SWAP_WORD(a) ( ((a) << 24) | \
			(((a) << 8) & 0x00ff0000) | \
			(((a) >> 8) & 0x0000ff00) | \
			((unsigned long)(a) >>24) )
#define RTJPEG_SWAP_HALFWORD(a) ( (((a) << 8) & 0xff00) | \
			(((a) >> 8) & 0x00ff) )
#else
#define RTJPEG_SWAP_WORD(a) (a)
#define RTJPEG_SWAP_HALFWORD(a) (a)
#endif

/*#ifndef _STDINT_H
#include <stdint.h>
#endif
*/
#ifdef MMX
#include "mmx.h"
#endif

/* Format definitions */

#define RTJ_YUV420 0
#define RTJ_YUV422 1
#define RTJ_RGB8   2

#include "ADM_inputs/ADM_nuv/ADM_rtGlob.h"

class RTjpeg : public baseRT
{
  public:
    RTjpeg();
  virtual  ~RTjpeg();
   
    int SetQuality(int *quality);
virtual  int SetFormat(int *format);
virtual    int SetSize(int *w, int *h);
    int SetIntra(int *key, int *lm, int *cm);

    int Compress(int8_t *sp, uint8_t **planes);
virtual     void Decompress(int8_t *sp, uint8_t **planes);
  

private:
    int b2s(int16_t *data, int8_t *strm, uint8_t bt8);
    int s2b(int16_t *data, int8_t *strm, uint8_t bt8, int32_t *qtbla);

    void QuantInit(void);
    void Quant(int16_t *block, int32_t *qtbl);
   
    void DctInit(void);
    void DctY(uint8_t *idata, int rskip);

    void IdctInit(void);
    void Idct(uint8_t *odata, int16_t *data, int rskip);

    void CalcTbls(void);

    inline int compressYUV420(int8_t *sp, uint8_t **planes);
    inline int compressYUV422(int8_t *sp, uint8_t **planes);
    inline int compress8(int8_t *sp, uint8_t **planes);

    int mcompressYUV420(int8_t *sp, uint8_t **planes);
    int mcompressYUV422(int8_t *sp, uint8_t **planes);
    int mcompress8(int8_t *sp, uint8_t **planes);
    
    void decompressYUV422(int8_t *sp, uint8_t **planes);
    void decompressYUV420(int8_t *sp, uint8_t **planes);
    
    void decompress8(int8_t *sp, uint8_t **planes);

#ifdef MMX
    int bcomp(int16_t *rblock, int16_t *old, mmx_t *mask);
#else
    int bcomp(int16_t *rblock, int16_t *old, uint16_t *mask);
#endif
    
    int16_t block[64] __attribute__ ((aligned (32)));
    int32_t ws[64*4] __attribute__ ((aligned (32)));
    int32_t lqt[64] __attribute__ ((aligned (32)));
    int32_t cqt[64] __attribute__ ((aligned (32)));
    int32_t liqt[64] __attribute__ ((aligned (32)));
    int32_t ciqt[64] __attribute__ ((aligned (32)));
    int32_t lb8;
    int32_t cb8;
    int32_t Ywidth;
    int32_t Cwidth;
    int32_t Ysize;
    int32_t Csize;
    int16_t *old;
    int16_t *old_start;
    int key_count;

    int width;
    int height;
    int Q;
    int f;
#ifdef MMX
    mmx_t lmask;
    mmx_t cmask;
#else
    uint16_t lmask;
    uint16_t cmask;
#endif
    int key_rate;
};

typedef struct {
	uint32_t framesize;
	uint8_t headersize;
	uint8_t version;
	uint16_t width;
	uint16_t height;
	uint8_t quality;
	uint8_t key;
	
} RTjpeg_frameheader;

#endif

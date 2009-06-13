/***************************************************************************
                         ADM_Rgb : wrapper for yv12->RGB display
                            using mplayer postproc


    begin                : Thu Apr 16 2003
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
    
 16 08 2007 : Removed the manual RGB<->BGR, newer swscale can output directly in the correct mode   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_default.h"
#include "ADM_colorspace.h"

extern "C" {
#include "ADM_ffmpeg/libavcodec/avcodec.h"
#include "ADM_ffmpeg/libavutil/avutil.h"
#include "ADM_ffmpeg/libswscale/swscale.h"
}

	
#include "ADM_rgb.h" 

#define CLEANUP() \
    if(_context) \
    {\
        if(ww==w && hh==h)\
           return 1; \
        clean();  \
    }

#ifdef ADM_CPU_X86
		#define ADD(x,y) if( CpuCaps::has##x()) flags|=SWS_CPU_CAPS_##y;
#define FLAGS()		ADD(MMX,MMX);				ADD(3DNOW,3DNOW);		ADD(MMXEXT,MMX2);
#else
#ifdef ADM_CPU_ALTIVEC
#define FLAGS() flags|=SWS_CPU_CAPS_ALTIVEC;
#else
#define FLAGS()
#endif
#endif

#define TARGET_COLORSPACE       PIX_FMT_RGB32 
#define ALTERNATE_COLORSPACE    PIX_FMT_BGR32

 //***********************************************
 ColBase::ColBase(uint32_t ww, uint32_t hh)
 {
      _context=NULL;
      w=0xfffff;
      h=0xfffff;
      //reset(ww,hh);   
 }
 ColBase::~ColBase()
 {
    clean();   
 }
 //***********************************************
 uint8_t ColBase::clean(void)
 {
        if(_context)
		{
			sws_freeContext((SwsContext *)_context);
		}
		_context=NULL; 
		return 1;  
 }
 //***********************************************
 uint8_t ColYuvRgb::reset(uint32_t ww, uint32_t hh)
 {
 int flags=0;
	
    CLEANUP();
    FLAGS();
    flags|=SWS_BILINEAR;
    PixelFormat fmt=TARGET_COLORSPACE;
    if(_inverted) fmt=ALTERNATE_COLORSPACE;
    if(!ww || !hh) return 0;

	if (_context)
		sws_freeContext((SwsContext *)_context);

	 _context=(void *)sws_getContext(
                      ww,hh,
                      PIX_FMT_YUV420P ,
                      ww,hh,
                      fmt,
                      flags, NULL, NULL,NULL);

    if(!_context) ADM_assert(0);
    w=ww;
    h=hh;
    return 1;
}
/**
      \fn invertRGB
      \brief Do RGBA->BGRA swap
      howmuch is in pixel, i.e. you have to multiply it by 4 to get the nb of bytes!
*/
static void ADM_RGBA2BGRA(uint8_t *ptr, uint32_t howmuch)
{
#ifdef ADM_CPU_X86
    __asm__(
                        "1: mov            (%0),%%eax \n"
                        "bswap          %%eax \n"
                        "rorl           $8,%%eax \n"
                        "mov            %%eax,(%0) \n"
                        "add            $4,%0 \n"
                        "dec            %1 \n"
                        "jne            1b\n"
                :  :"r" (ptr),"r" (howmuch)
                : "%eax"
                
      
                );
#else
uint8_t r,g,b,a;
                  for(int xx=0;xx<howmuch;xx++)
                  {
                      r=ptr[0];
                      g=ptr[1];
                      b=ptr[2];
                      a=ptr[3];
                      ptr[0]=b;
                      ptr[1]=g;
                      ptr[2]=r;
                      ptr[3]=a;
                      ptr+=4;
                  }
#endif
}
/**
      \fn scale
      \brief Do the actual colorconversion

*/
 uint8_t ColYuvRgb::scale(uint8_t *src, uint8_t *target)
 {
    uint8_t *srd[3];
	uint8_t *dst[3];
	int ssrc[3];
	int ddst[3];

	ADM_assert(_context);
	
			uint32_t page;

			page=w*h;
			srd[0]=src;
			srd[1]=src+page;
			srd[2]=src+((page*5)>>2);

			ssrc[0]=w;
			ssrc[1]=ssrc[2]=w>>1;

			
			dst[0]=target;
			dst[1]=NULL;
			dst[2]=NULL;
			ddst[0]=w*4;
			ddst[1]=ddst[2]=0;

			sws_scale((SwsContext *)_context,srd,ssrc,0,h,dst,ddst);
#if 0
        if(_inverted)
        {
                uint8_t r,g,b,a;
                uint8_t *ptr=target;
                int pel=h*w;
                ADM_RGBA2BGRA(ptr,pel);
        }
#endif
#if  defined( ADM_BIG_ENDIAN)
	if (!_inverted)
	{
        uint8_t r,g,b,a;
        uint8_t *ptr=target;
        int pel=h*w;
        for(int yy=0;yy<pel;yy++)
        {
              r=ptr[0];
              g=ptr[1];
              b=ptr[2];
              a=ptr[3];
              ptr[0]=a;
              ptr[1]=b;
              ptr[2]=g;
              ptr[3]=r;
              ptr+=4;
        }
	}
#endif
     
        return 1;
 }

/**
      \fn scale
      \brief Change colorspace but and put result in the target image , which is bigger
        Normally not used !
*/
 uint8_t ColYuvRgb::scale(uint8_t *src, uint8_t *target,uint32_t startx,uint32_t starty, uint32_t tw,uint32_t th,uint32_t totalW,uint32_t totalH)
 {
    uint8_t *srd[3];
    uint8_t *dst[3];
    int ssrc[3];
    int ddst[3];

    ADM_assert(_context);
    
    uint32_t page;

    page=tw*th;
    srd[0]=src;
    srd[1]=src+page;
    srd[2]=src+((page*5)>>2);

    ssrc[0]=tw;
    ssrc[1]=ssrc[2]=tw>>1;

    
    dst[0]=target+(startx*4)+starty*totalW*4;
    dst[1]=NULL;
    dst[2]=NULL;
    ddst[0]=totalW*4;
    ddst[1]=ddst[2]=0;

    sws_scale((SwsContext *)_context,srd,ssrc,0,th,dst,ddst);
#if 0
    if(_inverted)
    {
            uint8_t r,g,b,a;
            uint8_t *ptr=NULL;
            int pel=th;
            for(int yy=0;yy<th;yy++)
            {
              ptr=target+(startx*4)+(starty+yy)*totalW*4;;
              ADM_RGBA2BGRA(ptr,tw);
            }
     }
#endif

#if defined(ADM_BIG_ENDIAN)
     uint8_t r, g, b, a;
     uint8_t *ptr = target;
     int pel = h * w;

     for (int yy = 0; yy < th; yy++)
     {
          ptr = target + (startx * 4) + (starty + yy) * totalW * 4;
          ADM_RGBA2BGRA(ptr, tw);
     }
#endif

     return 1;
 }

 
 
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
uint8_t ColYv12Rgb24::reset(uint32_t ww, uint32_t hh)
 {
 int flags=0;
	CLEANUP();
    FLAGS();
    flags|=SWS_BILINEAR;
   if(!ww || !hh) return 0;

	if (_context)
		sws_freeContext((SwsContext *)_context);

	 _context=(void *)sws_getContext(
				    		ww,hh,
						PIX_FMT_YUV420P ,
		 				ww,hh,
	   					PIX_FMT_RGB24,
	    					flags, NULL, NULL,NULL);

    if(!_context) ADM_assert(0);
    w=ww;
    h=hh;
    return 1;
}
  uint8_t ColYv12Rgb24::scale(uint8_t *src, uint8_t *target)
 {
    uint8_t *srd[3];
	uint8_t *dst[3];
	int ssrc[3];
	int ddst[3];

	ADM_assert(_context);
	
			uint32_t page;

			page=w*h;
			srd[0]=src;
			srd[1]=src+page;
			srd[2]=src+((page*5)>>2);

			ssrc[0]=w;
			ssrc[1]=ssrc[2]=w>>1;

			
			dst[0]=target;
			dst[1]=NULL;
			dst[2]=NULL;
			ddst[0]=w*3;
			ddst[1]=ddst[2]=0;
			
			sws_scale((SwsContext *)_context,srd,ssrc,0,h,dst,ddst);
     
        return 1;
 }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 uint8_t ColRgbToYV12::reset(uint32_t ww, uint32_t hh)
 {
 int flags=0;
 int c;	
    clean();
    FLAGS();
    flags|=SWS_BILINEAR;
    switch(_colorspace)
    {
                case ADM_COLOR_RGB24:c=PIX_FMT_RGB24;break;
                case ADM_COLOR_RGB32A:c=TARGET_COLORSPACE;break;
                case ADM_COLOR_RGB16:c=PIX_FMT_RGB565;break;
                default: ADM_assert(0);
    }

	if (_context)
		sws_freeContext((SwsContext *)_context);

         _context=(void *)sws_getContext(
				    		ww,hh,
						c ,
		 				ww,hh,
	   					PIX_FMT_YUV420P,
	    					flags, NULL, NULL,NULL);

    if(!_context) ADM_assert(0);
    w=ww;
    h=hh;
    return 1;
}
uint8_t ColRgbToYV12::setBmpMode(void)
{
        _bmpMode=1;
        return 1;
}

//***********************************************
 uint8_t ColRgbToYV12::scale(uint8_t *src, uint8_t *target)
 {
    uint8_t *srd[3];
        uint8_t *dst[3];
        int ssrc[3];
        int ddst[3];
        int mul=0;

        ADM_assert(_context);

        uint32_t page;

        page=w*h;
        srd[0]=src;
        srd[1]=0;
        srd[2]=0;

        switch(_colorspace)
        {
                case ADM_COLOR_RGB16:  mul=2;
                        break;
                case ADM_COLOR_RGB24:  mul=3;
                        break;
                case ADM_COLOR_RGB32A:  mul=4;
                        break;
        }
        ssrc[0]=mul*w;
        ssrc[1]=0;
        ssrc[2]=0;

        dst[0]=target;
        dst[1]=target+page;
        dst[2]=target+((page*5)>>2);
        ddst[0]=w;
        ddst[1]=ddst[2]=w>>1;
        if(_bmpMode)
        {
                ssrc[0]=-mul*w;
                srd[0]=src+mul*w*(h-1);
                dst[2]=target+page;
                dst[1]=target+((page*5)>>2);
        }


        sws_scale((SwsContext *)_context,srd,ssrc,0,h,dst,ddst);
     
        return 1;
 }
 uint8_t ColRgbToYV12::changeColorSpace(ADM_colorspace col)
 {
int z;
    
    z=(int)col;
    _backward=!!(z & ADM_COLOR_BACKWARD);
    z&=~ADM_COLOR_BACKWARD;
    _colorspace= (ADM_colorspace) z;
       
 }
 //*************************
static inline void SwapMe(uint8_t *tgt,uint8_t *src,int nb);
void SwapMe(uint8_t *tgt,uint8_t *src,int nb)
{
    uint8_t r,g,b;
   nb=nb;
   while(nb--)
   {
       r=*src++;
       g=*src++;
       b=*src++;
       *tgt++=r;
       *tgt++=g;
       *tgt++=b;
       
   }
   return;
    
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 uint8_t  COL_yv12rgbBMP(uint32_t ww, uint32_t hh, uint8_t* in, uint8_t *out)
{
     ColYv12Rgb24 rgb(ww,hh);
     
        rgb.reset(ww,hh);
        rgb.scale(in,out);
        // Now time to swap it
        uint8_t swap[ww*3];
        
        uint8_t *up=out;
        uint8_t *down=out+(hh-1)*ww*3;
        
        for(int y=0;y<hh>>1;y++)
        {
            SwapMe(swap,up,ww); 
            SwapMe(up,down,ww);
            memcpy( down,swap,ww*3);
            down-=3*ww;
            up+=3*ww;
        }
        
        return 1;
     
}
//***************************************************
//***************************************************
//***************************************************
//***************************************************
//***************************************************
//***************************************************

COL_Generic2YV12::COL_Generic2YV12(uint32_t ww, uint32_t hh,ADM_colorspace col)
{
int flags=0;
int c=0; 

        FLAGS();
        flags|=SWS_BILINEAR;
        _context=NULL;
        w=ww;
        h=hh;
        _colorspace=(ADM_colorspace)(col & ADM_COLOR_MASK);
        _backward= !!(col & ADM_COLOR_BACKWARD);
      
 
    switch(_colorspace)
    {
     
                case ADM_COLOR_BGR24:c=PIX_FMT_BGR24;break;
                case ADM_COLOR_RGB24:c=PIX_FMT_RGB24;break;
                case ADM_COLOR_RGB555:c=PIX_FMT_RGB555;break;
                case ADM_COLOR_BGR555:c=PIX_FMT_BGR555;break;
                case ADM_COLOR_BGR32A:c=PIX_FMT_BGR32;break;
                case ADM_COLOR_RGB32A:c=TARGET_COLORSPACE;break;
                case ADM_COLOR_RGB16:c=PIX_FMT_RGB565;break;
                case ADM_COLOR_YUV411:c=PIX_FMT_YUV411P;break;
				case ADM_COLOR_YUV422:c=PIX_FMT_YUV422P;break;
				case ADM_COLOR_YUV444:c=PIX_FMT_YUV444P;break;
                default: ADM_assert(0);
    }
         _context=(void *)sws_getContext(
                                                ww,hh,
                                                c ,
                                                ww,hh,
                                                PIX_FMT_YUV420P,
                                                flags, NULL, NULL,NULL);

    if(!_context) ADM_assert(0);
}

uint8_t COL_Generic2YV12::clean(void)
{
        if(_context)
                {
                        sws_freeContext((SwsContext *)_context);
                }
                _context=NULL; 
                return 1;  
}

uint8_t COL_Generic2YV12::transform(uint8_t **planes, uint32_t *strides,uint8_t *target)
{
	uint8_t *srd[3];
	uint8_t *dst[3];
	int ssrc[3];
	int ddst[3];
	int mul = 0;
	uint32_t page = w * h;

	ADM_assert(_context);

	if(_colorspace & ADM_COLOR_IS_YUV)
	{
		srd[0] = planes[0];
		srd[1] = planes[2];
		srd[2] = planes[1];
		ssrc[0] = strides[0];
		ssrc[1] = strides[2];
		ssrc[2] = strides[1];

		dst[0] = target;
		dst[2] = target + page;
		dst[1] = target + ((page * 5) >> 2);
		ddst[0] = w;
		ddst[1] = ddst[2] = w >> 1;

		sws_scale((SwsContext *)_context, srd, ssrc, 0, h, dst, ddst);

		return 1;
	}

	// Else RGB colorspace
	switch (_colorspace & 0x7FFF)
	{
		case ADM_COLOR_RGB16:
		case ADM_COLOR_RGB555:
		case ADM_COLOR_BGR555:
			mul = 2;
			break;
		case ADM_COLOR_BGR24:
		case ADM_COLOR_RGB24:
			mul = 3;
			break;
		case ADM_COLOR_BGR32A:
		case ADM_COLOR_RGB32A:
			mul = 4;
			break;
		default:
			ADM_assert(0);
	}

	srd[0] = planes[0];
	srd[1] = srd[2] = 0;
	ssrc[0] = mul * w;
	ssrc[1] = ssrc[2] = 0;

	if(strides && strides[0] > ssrc[0])
		ssrc[0] = strides[0];

	dst[0] = target;
	dst[2] = target + page;
	dst[1] = target + ((page * 5) >> 2);
	ddst[0] = w;
	ddst[1] = ddst[2] = w >> 1;

	if(_backward)
	{
		ssrc[0] = -mul * w;
		srd[0] = planes[0] + mul * w * (h - 1);
	}

	sws_scale((SwsContext *)_context, srd, ssrc, 0, h, dst, ddst);

	return 1;
}
//EOF

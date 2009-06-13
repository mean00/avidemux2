/***************************************************************************
                       
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
 #ifndef ADM_COLORSPACE_H
 #define ADM_COLORSPACE_H
#include "ADM_rgb.h" // To have colors

// Some handy simple colorspace code

uint8_t  COL_yv12rgbBMP(uint32_t ww, uint32_t hh, uint8_t* in, uint8_t *out);
uint8_t  COL_yv12rgb(uint32_t w, uint32_t h,uint8_t * ptr, uint8_t * out );

uint8_t COL_422_YV12( uint8_t *in[3], uint32_t stride[3],  uint8_t *out,uint32_t w, uint32_t h);
uint8_t COL_411_YV12( uint8_t *in[3], uint32_t stride[3],  uint8_t *out,uint32_t w, uint32_t h);

uint8_t COL_RgbToYuv(uint8_t R,uint8_t G,uint8_t B, uint8_t *y,int8_t *u,int8_t  *v);
uint8_t COL_YuvToRgb( uint8_t y,int8_t u,int8_t v,uint8_t *r,uint8_t *g,uint8_t *b);

uint8_t COL_RawRGB32toYV12(uint8_t *data1,uint8_t *data2, uint8_t *oy,uint8_t *oy2, 
				uint8_t *u, uint8_t *v,uint32_t lineSize,uint32_t height,uint32_t stride);

uint8_t COL_RGB24_to_YV12(uint32_t w,uint32_t h,uint8_t *rgb, uint8_t *yuv);
uint8_t COL_RGB24_to_YV12_revert(uint32_t w,uint32_t h,uint8_t *rgb, uint8_t *yuv);

// Generic colorspace conversion class

class ADMColorspace
{
  protected:
    void  *context;
    uint32_t width,height;
    ADM_colorspace fromColor,toColor;
    uint8_t getStrideAndPointers(uint8_t  *from,ADM_colorspace fromColor,uint8_t **srcData,int *srcStride);
  public :
    
    ADMColorspace(uint32_t w, uint32_t h, ADM_colorspace from,ADM_colorspace to);
    uint8_t convert(uint8_t  *from, uint8_t *to);
    ~ADMColorspace();
};
/* Convert YV12 to RGB32, the reset must be called at least once before using scale */
 class ColBase
 {
  protected:
    void        *_context;
    uint32_t    w,h;  
    uint8_t     clean(void);
     
  public:
                ColBase(uint32_t w, uint32_t h);
                ~ColBase();
     virtual   uint8_t reset(uint32_t neww, uint32_t newh) {return 0;};
     virtual   uint8_t scale(uint8_t *src, uint8_t *target){return 0;};  
 };
 //************ YV12 to RB32********************************
 class ColYuvRgb : public ColBase
 {
  protected:
              uint32_t _inverted;
  public:
                ColYuvRgb(uint32_t w, uint32_t h,uint32_t inv=0): ColBase(w,h) {_inverted=inv;};
                ~ColYuvRgb(){clean();};
      virtual  uint8_t reset(uint32_t neww, uint32_t newh);
      virtual  uint8_t scale(uint8_t *src, uint8_t *target);
      virtual  uint8_t scale(uint8_t *src, uint8_t *target,uint32_t startx,uint32_t starty, uint32_t w,uint32_t h,uint32_t totalW,uint32_t totalH);    
 };
 //********************************************

  /* Convert RGB24/32 to YV12, the reset must be called at least once before using scale */
  class ColRgbToYV12  : public ColBase
  {
  protected:
                int             _bmpMode;
                ADM_colorspace  _colorspace;
                uint32_t        _backward;
  public:
                ColRgbToYV12(uint32_t w, uint32_t h,ADM_colorspace col) : ColBase(w,h) 
                    {
                            _colorspace=col;
                            _bmpMode=0;
                    };
                ~ColRgbToYV12(){clean();};
      virtual  uint8_t reset(uint32_t neww, uint32_t newh);
      virtual  uint8_t scale(uint8_t *src, uint8_t *target);  
               uint8_t changeColorSpace(ADM_colorspace col);   
               uint8_t setBmpMode(void);
      
  };
//********************************************
/* Convert RGB24 to YV12, the reset must be called at least once before using scale */
  class ColYv12Rgb24  : public ColBase
  {
  protected:
    
  public:
                ColYv12Rgb24(uint32_t w, uint32_t h) : ColBase(w,h) {};
                ~ColYv12Rgb24(){clean();};
      virtual  uint8_t reset(uint32_t neww, uint32_t newh);
      virtual  uint8_t scale(uint8_t *src, uint8_t *target);     
      
  };
//*************************************
class COL_Generic2YV12 
{
protected:
                void        *_context;
                uint32_t    w,h;  
                uint8_t     clean(void);
                ADM_colorspace _colorspace;
                uint32_t       _backward;
    
public:
        
                COL_Generic2YV12(uint32_t w, uint32_t h,ADM_colorspace) ;
                ~COL_Generic2YV12(){clean();};
                uint8_t transform(uint8_t **planes, uint32_t *strides,uint8_t *target);
                
}; 
#endif
//EOF


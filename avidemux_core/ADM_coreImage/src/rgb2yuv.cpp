
/***************************************************************************
                          yv.cpp  -  description
                             -------------------

	Convert a RGB triplet to its YUV conterpart

    begin                : Wed Aug 12 2003
    copyright            : (C) 2003 by mean
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

#include <math.h>


#include "ADM_default.h"


extern "C" {
#include "ADM_ffmpeg/libavcodec/avcodec.h"
#include "ADM_ffmpeg/libswscale/swscale.h"
}

#include "ADM_colorspace.h"

/**
	This is unoptimized because seldom called
*/
uint8_t COL_RgbToYuv(uint8_t R,uint8_t G,uint8_t B, uint8_t *y,int8_t *u,int8_t *v)
{

	float rr=R,bb=B,gg=G;
	float yy,uu,vv;

	yy=0.299*rr+ 		0.587*gg+ 	0.114*bb;
	uu=-0.169*rr+ 	-0.331*gg+  	0.5*bb;
	vv=0.5*rr+ 		-0.419*gg+ 	-0.081*bb;


	if(uu>127) uu=127;
	if(uu<-127) uu=-127;
	*u=(int8_t)floor(uu);

	if(vv>127) vv=127;
	if(vv<-127) vv=-127;
	*v=(int8_t)floor(vv);

	if(yy>255) yy=255;
	if(yy<0) yy=0;
	*y=(uint8_t)floor(yy);

	return 1;

}
static inline uint8_t PixelClip(int16_t in)
{
    if (in > 255)
	in = 255;
   if(in<0)
   	in=0;
    return (uint8_t) in;
};

			   	   
static inline unsigned char ScaledPixelClip(long  i)
{
    return PixelClip(((i +32768) >> 16)); //  + 32768
};

//0.299	0.587	0.114
const long int cyb = 6416; //int(0.114*219*65536/255+0.5);
const long int cyg = 33038; //int(0.587*219*65536/255+0.5);
const long int cyr = 16828; //int(0.299*219*65536/255+0.5);

uint8_t COL_RawRGB32toYV12(uint8_t *data1,uint8_t *data2, uint8_t *oy,uint8_t *oy2, 
				uint8_t *u, uint8_t *v,uint32_t lineSize,uint32_t height,uint32_t stride)
{


long int y1,y2;
long int y3,y4;
long int scaled_y,b_y;
long int scaled_y2,b_y2;
long int r_y,r_y2;  

uint8_t *org1,*org2;
/*
0 2 1
2 6 0
1 5 3
3 7 1
*/
#define B0 2
#define R0 0
#define M0 1

#define M1 (M0+4)
#define B1 (B0+4)
#define R1 (R0+4)
#define LEFT 0x108000

//	printf("w %d h %d stride %d\n",lineSize,height,stride);
	org1=data1;
	org2=data2;
	
	long int alpha,beta,gamma;
	
    for(uint32_t yy=0;yy<(height>>1);yy++)
    {
    	data1=org1;
	data2=org2;
      for (uint32_t x=0;x<(lineSize>>1);x++)
      {
      
#define MK1(source,out,row ) \
       alpha=source[B##row]; \
       alpha*=cyb; \
       beta=source[M##row]; \
       beta*=cyg; \
       gamma=source[R##row]; \
       gamma*=cyr;        \
       out=(gamma+alpha+beta+LEFT)>>16; 
      
        
       //y1 = (cyb*(long int)data1[B0] + cyg*(long int)data1[M0] + cyr*(long int)data1[R0] +LEFT) >> 16;
        MK1(data1,y1,0);
        oy[0] = y1;
        //y2 = (cyb*(long int)data1[B1] + cyg*(long int)data1[M1] + cyr*(long int)data1[R1] +LEFT) >> 16;
	MK1(data1,y2,1);
        oy[1] = y2;
	
	//y3 = (cyb*(long int)data2[B0] + cyg*(long int)data2[M0] + cyr*(long int)data2[R0] +LEFT) >> 16;
	MK1(data2,y3,0);
        oy2[0] = y3;
	
        //y4 = (cyb*(long int)data2[B1] + cyg*(long int)data2[M1] + cyr*(long int)data2[R1] +LEFT) >> 16;
	MK1(data2,y4,1);
        oy2[1] = y4;


        scaled_y = (y1+y2 - 32) * 38155;
	scaled_y2 = (y3+y4 - 32) * 38155;
	
        b_y = ((data1[B0]+data1[B1]) << 15) - scaled_y;
	b_y2 = ((data2[B0]+data2[B1]) << 15) - scaled_y2;
	
	b_y=(b_y+b_y2)/2;
	
        u[0] = ScaledPixelClip((b_y >> 10)*int(1/2.018*1024+0.5) + 0x800000);  // u * int(1/2.018*1024+0.5)
        
	r_y = ((data1[R0]+data1[R1]) << 15) - scaled_y;
	r_y2 = ((data2[R0]+data2[R1]) << 15) - scaled_y2;
	r_y=(r_y+r_y2)/2;
	
        v[0] = ScaledPixelClip((r_y >> 10)* int(1/1.596*1024+0.5) + 0x800000);  // v* int(1/1.596*1024+0.5)
	
	//u[0]=128;
	//v[0]=128;
        
	oy+=2;
	oy2+=2;
	u++;
	v++;
	data1+=8;
	data2+=8;
      }
     	org2+=2*stride;
      	org1+=2*stride;
	oy+=lineSize;
	oy2+=lineSize;	
     }
      
  return 1;
}
uint8_t COL_YuvToRgb( uint8_t y,int8_t u,int8_t v,uint8_t *r,uint8_t *g,uint8_t *b)
{

	float rr,bb,gg;
	float yy=y,uu=u,vv=v;

	rr=	yy+			 	1.402*vv;
	gg= yy+ 	-0.344*uu+  	-0.714*vv;
	bb=	yy+ 	1.772*uu 	 		;

	#define CLIP(x) if(x>255) x=255; else if (x<0) x=0;x=x+0.49;
	#define CVT(x,y) CLIP(x);*y=(uint8_t)floor(x);

	CVT(rr,r);
	CVT(gg,g);
	CVT(bb,b);

	return 1;

}
/**
 * 		\fn COL_RGB24_to_YV12
 *		\brief Convert RGB to YV12 using swscale
 */
uint8_t COL_RGB24_to_YV12_revert(uint32_t w,uint32_t h,uint8_t *rgb, uint8_t *yuv)
{
	int flags = SWS_SPLINE;
	struct SwsContext *context=NULL;
	
	#ifdef ADM_CPU_X86
		#define ADD(x, y) if (CpuCaps::has##x()) flags |= SWS_CPU_CAPS_##y;

		ADD(MMX,MMX);
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
	#endif

	#ifdef ADM_CPU_ALTIVEC
	    flags |= SWS_CPU_CAPS_ALTIVEC;
	#endif
	    context = sws_getContext(w, h,
	    									  PIX_FMT_RGB24,
	    									  w, h,
	    									  PIX_FMT_YUV420P,
	    									  flags, NULL, NULL,NULL);
	    ADM_assert(context);
	
	    uint8_t *src[3]={rgb+w*h*3-w*3,NULL,NULL};
	    int srcStride[3]={-w*3,0,0};
	    int dstStride[3]={w,w>>1,w>>1};
	    uint8_t *dst[3]={yuv,yuv+w*h,yuv+((w*h*5)>>2)};
//	    int sws_scale(struct SwsContext *context, uint8_t* src[], int srcStride[], int srcSliceY,
//	                  int srcSliceH, uint8_t* dst[], int dstStride[]);	    
	    sws_scale(context, src, srcStride, 0, h, dst, dstStride);
	    
		sws_freeContext(context);
}
/**
 * 		\fn COL_RGB24_to_YV12
 *		\brief Convert RGB to YV12 using swscale
 */
uint8_t COL_RGB24_to_YV12(uint32_t w,uint32_t h,uint8_t *rgb, uint8_t *yuv)
{
	int flags = SWS_SPLINE;
	struct SwsContext *context=NULL;
	
	#ifdef ADM_CPU_X86
		#define ADD(x, y) if (CpuCaps::has##x()) flags |= SWS_CPU_CAPS_##y;

		ADD(MMX,MMX);
		ADD(3DNOW,3DNOW);
		ADD(MMXEXT,MMX2);
	#endif

	#ifdef ADM_CPU_ALTIVEC
	    flags |= SWS_CPU_CAPS_ALTIVEC;
	#endif
	    context = sws_getContext(w, h,
	    									  PIX_FMT_RGB24,
	    									  w, h,
	    									  PIX_FMT_YUV420P,
	    									  flags, NULL, NULL,NULL);
	    ADM_assert(context);
	
	    uint8_t *src[3]={rgb,NULL,NULL};
	    int srcStride[3]={w*3,0,0};
	    int dstStride[3]={w,w>>1,w>>1};
	    uint8_t *dst[3]={yuv,yuv+((w*h*5)>>2),yuv+w*h};
//	    int sws_scale(struct SwsContext *context, uint8_t* src[], int srcStride[], int srcSliceY,
//	                  int srcSliceH, uint8_t* dst[], int dstStride[]);	    
	    sws_scale(context, src, srcStride, 0, h, dst, dstStride);
	    
		sws_freeContext(context);
}
//EOF

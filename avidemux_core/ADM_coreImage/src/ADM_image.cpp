/***************************************************************************
    \author  MEan (C) 2003-20010 by mean fixounet@free.fr
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
#include "ADM_image.h"

//#include "ADM_assert.h"
void ADMImage_stat( void );

static uint32_t imgMaxMem=0;
static uint32_t imgCurMem=0;
static uint32_t imgMaxNb=0;
static uint32_t imgCurNb=0;

void ADMImage_stat( void )
{
	printf("\nImages stat:\n");
	printf("___________\n");
	printf("Max memory consumed (MB)     : %"LU"\n",imgMaxMem>>20);
	printf("Current memory consumed (MB) : %"LU"\n",imgCurMem>>20);
	printf("Max image used               : %"LU"\n",imgMaxNb);
	printf("Cur image used               : %"LU"\n",imgCurNb);

}
/**
    \fn ADMImage
    \brief ctor
*/
ADMImage::ADMImage(uint32_t width, uint32_t height,ADM_IMAGE_TYPE type)
{
        _width=width;
        _height=height;
        _Qp=2;
        flags=0;
        _aspect=ADM_ASPECT_1_1;
        imgCurNb++;
        _noPicture=0;
        _colorspace=ADM_COLOR_YV12;
        Pts=0;
        _imageType=type;
        quant=NULL;
        _qStride=0;
        _qSize=0;
};
/**
    \fn ADMImage
    \brief dtor

*/
ADMImage::~ADMImage()
{
	imgCurNb--;

}


/**
 * 		\fn BitBlitAlpha
 * 		\brief Alpha blit from dst to src
 */
bool BitBlitAlpha(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,
		uint32_t width, uint32_t height,uint32_t alpha)
{

    for(int y=0;y<height;y++)
    {
    	for(int x=0;x<width;x++)
    	{
    		uint32_t s=src[x],d=dst[x];

    		d=s*alpha+(255-alpha)*d;
    		d>>=8;
    		dst[x]=d;
    	}
        src+=pitchSrc;
        dst+=pitchDst;
    }
    return 1;
}

bool BitBlit(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height)
{

    for(int y=0;y<height;y++)
    {
        memcpy(dst,src,width);
        src+=pitchSrc;
        dst+=pitchDst;
    }
    return 1;
}
//****************************************
/**
    \fn ADMImageDefault
    \brief ctor

*/
ADMImageDefault::ADMImageDefault(uint32_t w, uint32_t h) : ADMImage(w,h,ADM_IMAGE_DEFAULT)
{
    data=new uint8_t [(w*h*3)/2];
}
/**
    \fn ADMImageDefault
    \brief dtor
*/
ADMImageDefault::~ADMImageDefault()
{
    if(data) delete [] data;
    data=NULL;
}
bool           ADMImageDefault::isWrittable(void) {return true;}
uint32_t       ADMImageDefault::GetPitch(ADM_PLANE plane)
                    {
                            if(plane==PLANAR_Y) return _width;
                            return _width/2;
                        }
uint8_t        *ADMImageDefault::GetWritePtr(ADM_PLANE plane) {return GetReadPtr(plane);}
uint8_t        *ADMImageDefault::GetReadPtr(ADM_PLANE plane)
{
    int xplane=_width*_height;
    switch(plane)
    {
        case PLANAR_Y: return data;
        case PLANAR_U: return data+xplane;
        case PLANAR_V: return data+(5*xplane)/4;
    }
}
//****************************************
/**
    \fn ADMImageRef
    \brief ctor

*/
ADMImageRef::ADMImageRef(uint32_t w, uint32_t h) : ADMImage(w,h,ADM_IMAGE_REF)
{
    _planes[0]=_planes[1]=_planes[2]=NULL;
    _planeStride[0]=_planeStride[1]=_planeStride[2]=0;
}
/**
    \fn ADMImageRef
    \brief dtor
*/
ADMImageRef::~ADMImageRef()
{
}
bool           ADMImageRef::isWrittable(void) {return false;}
uint32_t       ADMImageRef::GetPitch(ADM_PLANE plane)
                    {
                          return _planeStride[plane];
                        }
uint8_t        *ADMImageRef::GetWritePtr(ADM_PLANE plane) {return NULL;}
uint8_t        *ADMImageRef::GetReadPtr(ADM_PLANE plane)
{
    return _planes[plane];
}
//EOF

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

#include <cmath>	// for NAN
#include "ADM_default.h"
#include "ADM_image.h"
extern "C"
{
#include "libavutil/imgutils.h"
}

static uint32_t imgMaxMem=0;
static uint32_t imgCurMem=0;
static uint32_t imgMaxNb=0;
static uint32_t imgCurNb=0;

void ADMImage_stat( void )
{
	printf("\nImages stat:\n");
	printf("___________\n");
	printf("Max memory consumed (MB)     : %" PRIu32"\n",imgMaxMem>>20);
	printf("Current memory consumed (MB) : %" PRIu32"\n",imgCurMem>>20);
	printf("Max image used               : %" PRIu32"\n",imgMaxNb);
	printf("Cur image used               : %" PRIu32"\n",imgCurNb);

}

/**
    \fn ADM_HDR_Info
    \brief Intialize all fields to NaNs
*/
ADM_HDR_Info::ADM_HDR_Info(void)
{
    int j,k;
    for (j=0; j<3; j++)
    {
        for (k=0; k<2; k++)
            primaries[j][k] = NAN;
        maxSCL[j] = NAN;
    }
    for (j=0; j<2; j++)
    {
        whitePoint[j] = NAN;
        kneePoint[j] = NAN;
    }
    for (j=0; j<15; j++)
        bezierCurveAnchors[j] = NAN;
    maxLuminance = minLuminance = maxCLL = maxFALL = avgMaxRGB = colorSaturationWeight = targetMaxLuminance = NAN;
}

/**
    \fn ADMImage
    \brief ctor
*/
ADMImage::ADMImage(uint32_t width, uint32_t height,ADM_IMAGE_TYPE type)
{
        refType=ADM_HW_NONE;
        memset(&refDescriptor,0,sizeof(refDescriptor));

        _width=width;
        _height=height;
        _Qp = ADM_IMAGE_UNKNOWN_QP;
        flags=0;
        _aspect=ADM_ASPECT_1_1;
        imgCurNb++;
        if(imgCurNb>imgMaxNb)
            imgMaxNb=imgCurNb;
        _noPicture=0;
        _pixfrmt=ADM_PIXFRMT_YV12;
        _range=ADM_COL_RANGE_MPEG;
        _colorPrim=ADM_COL_PRI_BT709;
        _colorTrc=ADM_COL_TRC_BT709;
        _colorSpace=ADM_COL_SPC_BT709;
        Pts=0;
        refPts=0;
        refVideo=0;
        _imageType=type;
        quant=NULL;
        _qStride=0;
        _qSize=0;
        _alpha=NULL;
        _alphaStride=0;
};
/**
    \fn ADMImage
    \brief dtor

*/
ADMImage::~ADMImage()
{
    imgCurNb--;
    hwDecRefCount();
}
/**
    \fn hwIncRefCount
    \brief hwIncRefCount

*/

 bool            ADMImage::hwIncRefCount(void)
{
        if(refType==ADM_HW_NONE) return true;
        ADM_assert(refDescriptor.refMarkUsed);
        return refDescriptor.refMarkUsed(refDescriptor.refCodec,refDescriptor.refHwImage); 
}
/**
    \fn hwDecRefCount
    \brief hwDecRefCount

*/

 bool            ADMImage::hwDecRefCount(void)
{
        if(refType==ADM_HW_NONE) return true;
        ADM_assert(refDescriptor.refMarkUnused);
        if(refDescriptor.refMarkUnused(refDescriptor.refCodec, refDescriptor.refHwImage))
        {
            refType = ADM_HW_NONE;
            return true;
        }
        return false;
}
/**
    \fn    hwDownloadFromRef
    \brief Convert an HW ref image to a regular image

*/
 bool            ADMImage::hwDownloadFromRef(void)
{
bool r=false;
        if(refType==ADM_HW_NONE) return true;
        ADM_assert(refDescriptor.refDownload);
        r=refDescriptor.refDownload(this,refDescriptor.refCodec,refDescriptor.refHwImage);
        hwDecRefCount();
        refType=ADM_HW_NONE;
        return r;

}

/**
 *  \fn BitBlitAlpha
 *  \brief Alpha blit from dst to src
 */
bool BitBlitAlpha(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,
                  uint32_t width, uint32_t height,uint32_t alpha)
{
    if(alpha>255) alpha=255;
    if(alpha==255) // take a shortcut
        return BitBlit(dst,pitchDst,src,pitchSrc,width,height);
    float af = alpha * (256.0/255.0);
    alpha = af+0.49;
    uint32_t negalpha = 256-alpha;
    uint32_t pix;
    for(int y=0;y<height;y++)
    {
        for(int x=0;x<width;x++)
        {
            pix = alpha*src[x] + negalpha*dst[x];
            dst[x]=(uint8_t)(pix>>8);
        }
        src+=pitchSrc;
        dst+=pitchDst;
    }
    return 1;
}
/**
 * 
 * @param dst
 * @param pitchDst
 * @param src
 * @param pitchSrc
 * @param width
 * @param height
 * @return 
 */
bool BitBlit(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height)
{
#if 0
    // av_image_copy_plane is just a wrapper around a wrapper of line-by-line memcpy
    // ffmpeg makes it better
     av_image_copy_plane(dst, (int) pitchDst,
                         src, (int) pitchSrc,
                         width, height);
#else
    for (;height > 0; height--)
    {
        memcpy(dst,src,width);
        src+=pitchSrc;
        dst+=pitchDst;
    }
#endif
  return 1;
}
//****************************************
/**
    \fn ADMImageDefault
    \brief ctor

*/
ADMImageDefault::ADMImageDefault(uint32_t w, uint32_t h) : ADMImage(w,h,ADM_IMAGE_DEFAULT)
{
    uint32_t pitch=ADM_IMAGE_ALIGN(w);
    uint32_t allocatedHeight=ADM_IMAGE_ALIGN(h);
    data.setSize(64+(pitch*allocatedHeight*3)/2);
    _planes[0]=data.at(0);
    _planes[1]=data.at(pitch*allocatedHeight);
    _planes[2]=data.at((pitch*allocatedHeight*5)>>2);
    _planeStride[0]=pitch;
    _planeStride[1]=pitch/2;
    _planeStride[2]=pitch/2;
}
/**
    \fn ADMImageDefault
    \brief dtor
*/
ADMImageDefault::~ADMImageDefault()
{
    data.clean();
}
/**
 * 
 * @return 
 */
bool           ADMImageDefault::isWrittable(void)
{
        return true;
}
/**
 */
bool           ADMImageDefault::addAlphaChannel(void)
{
    int paddedWidth=(_width+31)& ~31;
    alphaChannel.setSize(paddedWidth*_height);
    _alpha=alphaChannel.at(0);
    _alphaStride=paddedWidth;
    return true;
}
 
/**
 * 
 * @param plane
 * @return 
 */
uint32_t       ADMImageDefault::GetPitch(ADM_PLANE plane)
{
    if(plane==PLANAR_ALPHA)
        return _alphaStride;
    return _planeStride[plane];
}
/**
 * 
 * @param plane
 * @return 
 */
uint8_t        *ADMImageDefault::GetWritePtr(ADM_PLANE plane) 
{
    return GetReadPtr(plane);
}
/**
 * 
 * @param plane
 * @return 
 */
uint8_t        *ADMImageDefault::GetReadPtr(ADM_PLANE plane)
{
    if(plane==PLANAR_ALPHA)
        return _alpha;
    return _planes[plane];
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
bool           ADMImageRef::isWrittable(void) 
{
        return false;
}
uint32_t       ADMImageRef::GetPitch(ADM_PLANE plane)
{
    if(plane==PLANAR_ALPHA)
        return _alphaStride;
    return _planeStride[plane];
 }
/**
 * 
 * @param plane
 * @return 
 */
// Cannot write to a ref, the buffer does not belong to us...
uint8_t        *ADMImageRef::GetWritePtr(ADM_PLANE plane) 
{
        return NULL;
}
/**
 * 
 * @param plane
 * @return 
 */
uint8_t        *ADMImageRef::GetReadPtr(ADM_PLANE plane)
{
    return _planes[plane];
}
/**
 * 
 * @param plane
 * @return 
 */
int             ADMImage::GetHeight(ADM_PLANE plane)
{
    if(plane==PLANAR_Y  || plane==PLANAR_ALPHA) 
        return _height; 
    return _height/2;
}
/**
 * 
 * @param plane
 * @return 
 */
int             ADMImage::GetWidth(ADM_PLANE plane) 
{
    if(plane==PLANAR_Y || plane==PLANAR_ALPHA) 
        return _width; 
    return _width/2;
}
/**
 * 
 * @param pitches
 * @return 
 */
bool            ADMImage::GetPitches(int *pitches) 
{
    pitches[0]=GetPitch(PLANAR_Y);
    pitches[1]=GetPitch(PLANAR_V);
    pitches[2]=GetPitch(PLANAR_U);
    return true;
}
/**
 * 
 * @param planes
 * @return 
 */
bool            ADMImage::GetWritePlanes(uint8_t **planes) 
{
    planes[0]=GetWritePtr(PLANAR_Y);
    planes[1]=GetWritePtr(PLANAR_V);
    planes[2]=GetWritePtr(PLANAR_U);
    return true;
}
/**
 * 
 * @param planes
 * @return 
 */
bool            ADMImage::GetReadPlanes(uint8_t **planes) 
{
    planes[0]=GetReadPtr(PLANAR_Y);
    planes[1]=GetReadPtr(PLANAR_V);
    planes[2]=GetReadPtr(PLANAR_U);
    return true;
}


//EOF

/***************************************************************************
    copyright            : (C) 2003-2005 by mean
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


#include "ADM_default.h"
#include "ADM_image.h"
/**
    \fn duplicateMacro
    \brief copy src to this, swapping u&v possibly
*/
bool ADMImage::duplicateMacro(ADMImage *src,bool swap)
{
//#warning handle swap
        // Sanity check
        ADM_assert(src->_width==_width);
        ADM_assert(src->_height==_height);
        ADM_assert(isWrittable()==true); // could not duplicate to a linked data image
        copyInfo(src);
        uint32_t sourceStride,destStride;
        uint8_t  *source,*dest;

        hwDecRefCount(); // free hw ref image if any..
        if(src->refType==ADM_HW_NONE)
        {
            for(int plane=PLANAR_Y;plane<PLANAR_LAST;plane++)
            {
                source=src->GetReadPtr((ADM_PLANE)plane);
                dest=this->GetWritePtr((ADM_PLANE)plane);
                sourceStride=src->GetPitch((ADM_PLANE)plane);
                destStride=this->GetPitch((ADM_PLANE)plane);
                int opHeight=_height;
                int opWidth=_width;
                if(plane!=PLANAR_Y)
                {
                    opHeight>>=1;
                    opWidth>>=1;
                }
                BitBlit(dest, destStride,source,sourceStride,opWidth, opHeight);
            }
        }
         else // it is a hw surface
        {
            refType                    =src->refType;
            refDescriptor.refHwImage   =src->refDescriptor.refHwImage;
            refDescriptor.refCodec     =src->refDescriptor.refCodec;
            refDescriptor.refMarkUsed  =src->refDescriptor.refMarkUsed;
            refDescriptor.refMarkUnused=src->refDescriptor.refMarkUnused;
            refDescriptor.refDownload  =src->refDescriptor.refDownload;
            hwIncRefCount();
        }
        return true;
}
/**
    \fn duplicate
*/
bool ADMImage::duplicate(ADMImage *src)
{
	return duplicateMacro(src,false);
}
/**
    \fn duplicateFull
    \brief copy data + info (pts...)
*/
bool ADMImage::duplicateFull(ADMImage *src)
{
    // Sanity check
    ADM_assert(src->_width==_width);
    ADM_assert(src->_height==_height);


    copyInfo(src);
    duplicate(src);
    return 1;
}
/**
    \fn copyInfo
    \brief Copy the additionnal infos attached to an image (flags/aspect ration/PTS)
*/
bool ADMImage::copyInfo(ADMImage *src)
{
    _Qp=src->_Qp;
    flags=src->flags;
    _aspect=src->_aspect;
    Pts=src->Pts;
    return 1;
}
/**
    \fn blacken
*/
bool ADMImage::blacken(void)
{
        ADM_assert(isWrittable()==true); // could not duplicate to a linked data image
        uint32_t sourceStride,destStride;
        uint8_t  *dest;

        for(int plane=PLANAR_Y;plane<PLANAR_LAST;plane++)
        {
            dest=this->GetWritePtr((ADM_PLANE)plane);
            destStride=this->GetPitch((ADM_PLANE)plane);
            int opHeight=_height;
            int opWidth=_width;
            uint8_t color=0;
            if(plane!=PLANAR_Y)
            {
                opHeight>>=1;
                opWidth>>=1;
                color=128;
            }
            for(int y=0;y<opHeight;y++)
            {
                memset(dest,color,opWidth);
                dest+=destStride;
            }
        }
        return true;
}
/**
    \fn copyTo
    \brief  Copy "this" image into dest image at x,y position

*/
bool ADMImage::copyTo(ADMImage *dest, uint32_t x,uint32_t y)
{

    uint32_t box_w=_width, box_h=_height;
    // Clip if needed
    if(y>dest->_height)
    {
        printf("Y out : %u %u\n",y,dest->_height);
         return 1;
    }
    if(x>dest->_width)
    {
        printf("X out : %u %u\n",x,dest->_width);
         return 1;
    }

    if(x+box_w>dest->_width) box_w=dest->_width-x;
    if(y+box_h>dest->_height) box_h=dest->_height-y;
    // Get Source plane
    uint8_t *srcPlanes[3];
    uint8_t *dstPlanes[3];
    dest->GetWritePlanes(dstPlanes);
    GetReadPlanes(srcPlanes);

    int srcPitches[3],dstPitches[3];
    dest->GetPitches(dstPitches);
    GetPitches(srcPitches);
    // do y
    for(int i=0;i<3;i++)
    {
        int xx=x;
        int yy=y;
        int ww=box_w;
        int hh=box_h;
        if(i) {xx/=2;yy/=2;ww/=2;hh/=2;} /// u or v
        BitBlit(dstPlanes[i]+xx+dstPitches[i]*yy, dstPitches[i],
                     srcPlanes[i],srcPitches[i],
                     ww,hh);
    }
    return 1;

}
/**
    \fn    copyToAlpha
    \brief Copy "this" image into dest image at x,y position using alpha alpha
    @param alpha alpha value (0--255)

*/
bool ADMImage::copyToAlpha(ADMImage *dest, uint32_t x,uint32_t y,uint32_t alpha)
{

    uint32_t box_w=_width, box_h=_height;
    // Clip if needed
    if(y>dest->_height)
    {
        printf("Y out : %u %u\n",y,dest->_height);
         return 1;
    }
    if(x>dest->_width)
    {
        printf("X out : %u %u\n",x,dest->_width);
         return 1;
    }

    if(x+box_w>dest->_width) box_w=dest->_width-x;
    if(y+box_h>dest->_height) box_h=dest->_height-y;
    // Get Source plane
    uint8_t *srcPlanes[3];
    uint8_t *dstPlanes[3];
    dest->GetWritePlanes(dstPlanes);
    GetReadPlanes(srcPlanes);

    int srcPitches[3],dstPitches[3];
    dest->GetPitches(dstPitches);
    GetPitches(srcPitches);
    // do y
    for(int i=0;i<3;i++)
    {
        int xx=x;
        int yy=y;
        int ww=box_w;
        int hh=box_h;
        if(i) {xx/=2;yy/=2;ww/=2;hh/=2;} /// u or v
        BitBlitAlpha(dstPlanes[i]+xx+dstPitches[i]*yy, dstPitches[i],
                     srcPlanes[i],srcPitches[i],
                     ww,hh,alpha);
    }
    return 1;
}
/**
 * 
 * @param dest
 * @param x
 * @param y
 * @param alpha
 * @return 
 */

static bool blitWithAlpha(uint8_t *dst, uint8_t *src, uint8_t *alpha,int dstStride, int srcStride,int alphaStride,int w,int h, int mul)
{
    for(int k=0;k<h;k++)
    {
        for(int j=0;j<w;j++)
        {
            int a=alpha[mul*j];
            uint32_t x=(255-a)*dst[j]+a*src[j];
            dst[j]=x>>8;
        }
        dst+=dstStride;
        src+=srcStride;
        alpha+=alphaStride*mul;
    }
    return true;
}
bool ADMImage::copyWithAlphaChannel(ADMImage *dest, uint32_t x,uint32_t y)
{

    uint32_t box_w=_width, box_h=_height;
    // Clip if needed
    if(y>dest->_height)
    {
        printf("Y out : %u %u\n",y,dest->_height);
         return 1;
    }
    if(x>dest->_width)
    {
        printf("X out : %u %u\n",x,dest->_width);
         return 1;
    }

    if(x+box_w>dest->_width) box_w=dest->_width-x;
    if(y+box_h>dest->_height) box_h=dest->_height-y;
    // Get Source plane
    uint8_t *srcPlanes[3];
    uint8_t *dstPlanes[3];
    dest->GetWritePlanes(dstPlanes);
    GetReadPlanes(srcPlanes);

    int srcPitches[3],dstPitches[3];
    dest->GetPitches(dstPitches);
    GetPitches(srcPitches);

    uint8_t *alpha=GetReadPtr(PLANAR_ALPHA);
    int      alphaStride=GetPitch(PLANAR_ALPHA);
    
    // do U & V
    for(int i=0;i<3;i++)
    {
        int mul=0;
        if(i) mul++;
        int xx=x>>mul;
        int yy=y>>mul;
        int ww=box_w>>mul;
        int hh=box_h>>mul;
        mul++;
        blitWithAlpha(  dstPlanes[i]+xx+dstPitches[i]*yy, 
                        srcPlanes[i],
                        alpha,
                
                        dstPitches[i], 
                        srcPitches[i], 
                        alphaStride,
                        ww,hh,mul);
    }
    return 1;
}
//EOF

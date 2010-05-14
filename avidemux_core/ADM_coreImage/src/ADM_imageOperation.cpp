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
#warning handle swap
        // Sanity check
        ADM_assert(src->_width==_width);
        ADM_assert(src->_height==_height);
        ADM_assert(isWrittable()==true); // could not duplicate to a linked data image
        copyInfo(src);
        uint32_t sourceStride,destStride;
        uint8_t  *source,*dest;

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
        uint8_t  *source,*dest;

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
    ADM_assert(0);
#warning not implemented
#if 0    
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

    // do y
    BitBlit(YPLANE(dest)+x+dest->_width*y,dest->_width,
            data,_width,
            box_w,box_h);
    // Do u
    BitBlit(UPLANE(dest)+x/2+(dest->_width*y)/4,dest->_width/2,
            UPLANE(this),_width>>1,
            box_w>>1,box_h>>1);

    BitBlit(VPLANE(dest)+x/2+(dest->_width*y)/4,dest->_width/2,
            VPLANE(this),_width>>1,
            box_w>>1,box_h>>1);

#endif
    return 1;

}
/**
    \fn    copyToAlpha
    \brief Copy "this" image into dest image at x,y position using alpha alpha
    @param alpha alpha value (0--255)

*/
bool ADMImage::copyToAlpha(ADMImage *dest, uint32_t x,uint32_t y,uint32_t alpha)
{
#warning not implemented
    ADM_assert(0);
#if 0
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

    // do y
    BitBlitAlpha(YPLANE(dest)+x+dest->_width*y,dest->_width,         data,_width,            box_w,box_h,alpha);
    // Do u
    BitBlitAlpha(UPLANE(dest)+x/2+(dest->_width*y)/4,dest->_width/2,   UPLANE(this),_width>>1,  box_w>>1,box_h>>1,alpha);
    // and V
    BitBlitAlpha(VPLANE(dest)+x/2+(dest->_width*y)/4,dest->_width/2, VPLANE(this),_width>>1, box_w>>1,box_h>>1,alpha);

#endif
    return 1;
}

//EOF

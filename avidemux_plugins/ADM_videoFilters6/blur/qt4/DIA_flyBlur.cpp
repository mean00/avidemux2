/**/
/***************************************************************************
                          DIA_flyBlur
                             -------------------

			   Ui for Blur

    begin                : 08 Apr 2005
    copyright            : (C) 2004/7 by mean
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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyBlur.h"
#include "ADM_vidBlur.h"

/************* COMMON PART *********************/

/**
 */
flyBlur::flyBlur (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
  {
    rubber=new ADM_rubberControl(this,canvas);
    _ox=0;
    _oy=0;
    _ow=width;
    _oh=height;
  }
/**
 * 
 */
flyBlur::~flyBlur()
{
    delete rubber;
    rubber=NULL;
}

static int bound(int val, int other, int maxx)
{
   int r=(int)maxx-(int)(val+other);
   if(r<0) 
        r=0;
   return r;
}

/**
 * \fn bandResized
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */
bool    flyBlur::bandResized(int x,int y,int w, int h)
{

    //aprintf("Rubber resized: x=%d, y=%d, w=%d, h=%d\n",x,y,w,h);
    //aprintf("Debug: old values: x=%d, y=%d, w=%d, h=%d\n",_ox,_oy,_ow,_oh);

    double halfzoom=_zoom/2-0.01;
    // try to recalculate values only if these values were actually modified by moving the handles
    bool leftHandleMoved=false;
    bool rightHandleMoved=false;
    if((x+w)==(_ox+_ow) && (y+h)==(_oy+_oh))
        leftHandleMoved=true;
    if(x==_ox && y==_oy)
        rightHandleMoved=true;

    _ox=x;
    _oy=y;
    _ow=w;
    _oh=h;

    bool ignore=false;
    if(leftHandleMoved && rightHandleMoved) // bogus event
        ignore=true;

    int normX, normY, normW, normH;
    normX=(int)(((double)x+halfzoom)/_zoom);
    normY=(int)(((double)y+halfzoom)/_zoom);
    normW=(int)(((double)w+halfzoom)/_zoom);
    normH=(int)(((double)h+halfzoom)/_zoom);

    // resize the rubberband back into bounds once the user tries to drag handles out of sight
    bool resizeRubber=false;
    if(normX<0 || normY<0 || normX+normW>_w || normY+normH>_h)
    {
        resizeRubber=true;
        //aprintf("rubberband out of bounds, will be resized back\n");
    }

    if(ignore)
    {
        upload(false,resizeRubber);
        return false;
    }

    if(rightHandleMoved)
    {
        right=bound(normX,normW,_w);
        bottom=bound(normY,normH,_h);
    }

    if(normX<0) normX=0;
    if(normY<0) normY=0;

    if(leftHandleMoved)
    {
        top=normY;
        left=normX;
    }

    upload(false,resizeRubber);
    sameImage();
  
    return true; 
}
/**
 * \fn bandMoved
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */
bool    flyBlur::bandMoved(int x,int y,int w, int h)
{
    double halfzoom=_zoom/2-0.01;

    int normX, normY, normW, normH;
    normX=(int)(((double)x+halfzoom)/_zoom);
    normY=(int)(((double)y+halfzoom)/_zoom);
    normW=(int)(((double)w+halfzoom)/_zoom);
    normH=(int)(((double)h+halfzoom)/_zoom);

    // bound checks are done in rubber control

    right=bound(normX,normW,_w);
    bottom=bound(normY,normH,_h);

    if(normX<0) normX=0;
    if(normY<0) normY=0;

    top=normY;
    left=normX;

    upload(false,false);
    sameImage();
  
    return true; 
}


uint8_t  flyBlur::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyBlur::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    param.left = left;
    param.top = top;
    param.right = right;
    param.bottom = bottom;
    // Do it!
    ADMVideoBlur::BlurProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param, rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
    return 1;
}


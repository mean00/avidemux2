/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
    copyright            : (C) 2002/2007 by mean
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
#include "DIA_flyBlackenBorders.h"
#include "ui_blackenBorders.h"

#if 0 
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

/**
 */
flyBlacken::flyBlacken (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider)
                : ADM_flyDialogRgb(parent,width, height,in,canvas, slider,RESIZE_LAST) 
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
flyBlacken::~flyBlacken()
{
    delete rubber;
    rubber=NULL;
}

/**
    \fn process
	\brief 
*/
uint8_t    flyBlacken::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
        uint32_t x,y;
        uint8_t  *in;
        uint32_t w=_w,h=_h;
  
        
        memcpy(imageOut,imageIn,_w*_h*4);
        in=imageOut;
        for(y=0;y<top;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        
                        *in++=0;
                        *in++=0xff;
                }
        }
        // bottom
        in=imageOut+(w*4)*(h-bottom);
        for(y=0;y<bottom;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        *in++=0;
                        *in++=0xff;
                }
        }
        // left
        in=imageOut;
        uint32_t stride=4*w-4;
        for(y=0;y<h;y++)
        {
                for(x=0;x<left;x++)
                {
                        *(in+4*x)=0;
                        
                        
                        *(in+4*x+1)=0xff;
                        *(in+4*x+2)=0;
                        *(in+4*x+3)=0xff;
                }
                for(x=0;x<right;x++)
                {
                        *(in-4*x+stride-4)=0;
                        
                        
                        *(in-4*x+stride-3)=0xff;
                        *(in-4*x+stride-2)=0;
                        *(in-4*x+stride-1)=0xff;
                        
                }
                in+=4*w;
  
        }
        return true;

}

/**
 * 
 * @param imageIn
 * @param imageOut
 * @return 
 */
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
bool    flyBlacken::bandResized(int x,int y,int w, int h)
{

    aprintf("Rubber resized: x=%d, y=%d, w=%d, h=%d\n",x,y,w,h);
    aprintf("Debug: old values: x=%d, y=%d, w=%d, h=%d\n",_ox,_oy,_ow,_oh);

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
        aprintf("rubberband out of bounds, will be resized back\n");
    }

    if(ignore)
    {
        upload(false,resizeRubber);
        return false;
    }

    if(rightHandleMoved)
    {
        right=bound(normX,normW,_w)&0xfffe;
        bottom=bound(normY,normH,_h)&0xfffe;
    }

    if(normX<0) normX=0;
    if(normY<0) normY=0;

    if(leftHandleMoved)
    {
        top=normY&0xfffe;
        left=normX&0xfffe;
    }

    upload(false,resizeRubber);
    sameImage();
  
    return true; 
}
/**
 * \fn blockChanges
 * @param block
 * @return 
 */
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x;rubber->x;}
bool flyBlacken::blockChanges(bool block)
{
    Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;
    APPLY_TO_ALL(blockSignals(block));
    return true;
}
//EOF


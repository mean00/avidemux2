/***************************************************************************
  \fn     DIA_flyBlur
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for blur
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_BLUR_H
#define FLY_BLUR_H
#include "blur.h"
#include "DIA_flyDialogQt4.h"
/**
    \class flyBlur
*/
class flyBlur : public ADM_flyDialogYuv
{
  private:
    int                    rgbBufStride;
    ADM_byteBuffer *       rgbBufRaw;
    ADMImageRef *          rgbBufImage;
    ADMColorScalerFull *   convertYuvToRgb;
    ADMColorScalerFull *   convertRgbToYuv;

    ADM_rubberControl *rubber;
    bool        rubber_is_hidden;
    int         _ox,_oy,_ow,_oh;

  public:
    blur        param;
    uint32_t    left,right,top,bottom;

    bool        blockChanges(bool block);
    bool        bandResized(int x,int y,int w, int h);
    bool        bandMoved(int x,int y,int w, int h);
    bool        rubberIsHidden(void) { return rubber_is_hidden; }
    void        hideRubber(bool hide);
    void        adjustRubber(void);

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void) {return upload(true,true);}
    uint8_t    upload(bool redraw, bool rubber);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyBlur (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider);
    virtual    ~flyBlur();

};
#endif

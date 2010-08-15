/****************************************************************************
 copyright            : (C) 2007 by mean
 email                : fixounet@free.fr
 
 Simple class to do filter that have a configuration that have real time effect
 
 Case 1: The filter process YUV
 
 YUV-> Process->YUVOUT->YUV2RGB->RGBUFFEROUT
 
 Case 2: The filter process RGV
 
 YUV-> YUV2RGB->RGB->Process->RGBUFFEROUT
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_FLY_DIALOG_GTKH
#define ADM_FLY_DIALOG_GTKH
#include "DIA_flyDialog.h"
class ADM_flyDialogGtk : public ADM_flyDialog
{
public:
  ADM_flyDialogGtk(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod);
  virtual           ~ADM_flyDialogGtk(void);
  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(uint8_t *rgbdata);
  virtual float    calcZoomFactor(void);
  virtual uint32_t sliderGet(void);
  virtual uint8_t  sliderSet(uint32_t value);
  virtual void     postInit(uint8_t reInit);
  virtual bool     setCurrentPts(uint64_t  pts){return true;};
};

#endif

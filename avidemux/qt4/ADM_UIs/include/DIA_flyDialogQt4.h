/** **************************************************************************
        \fn DIA_flyDialogQt4.h
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
#ifndef ADM_FLY_DIALOG_QT4H
#define ADM_FLY_DIALOG_QT4H

#include "ADM_UIQT46_export.h"
#include "ADM_image.h"
#include "DIA_flyDialog.h"

class FlyDialogEventFilter : public QObject
{
	ADM_flyDialog *flyDialog;
	bool recomputed;

public:
	FlyDialogEventFilter(ADM_flyDialog *flyDialog);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};

class ADM_UIQT46_EXPORT ADM_flyDialogQt4 : public ADM_flyDialog
{
public:
  ADM_flyDialogQt4(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod);

  
  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(uint8_t *rgbData);
  virtual float    calcZoomFactor(void);
  virtual uint32_t sliderGet(void);
  virtual uint8_t  sliderSet(uint32_t value);
  virtual void     postInit(uint8_t reInit);
    bool           setCurrentPts(uint64_t pts) {return 1;}
};


class ADM_UIQT46_EXPORT ADM_QCanvas : public QWidget
{
protected:
	uint32_t _w,_h;
public:
	uint8_t *dataBuffer;

	ADM_QCanvas(QWidget *z, uint32_t w, uint32_t h);
	~ADM_QCanvas();
	void paintEvent(QPaintEvent *ev);
	void changeSize(uint32_t w, uint32_t h);
};

#endif

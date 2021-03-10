/***************************************************************************
  \fn     DIA_flyArtGrid
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for artGrid
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_ARTGrid_H
#define FLY_ARTGrid_H
#include "artGrid.h"
/**
    \class flyArtGrid
*/
class flyArtGrid : public ADM_flyDialogYuv
{
  private:
    ADMImage   *work;
  public:
    artGrid    param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       blacken(void) { work->blacken(); }
    void       setTabOrder(void);
               flyArtGrid (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
    virtual    ~flyArtGrid() ;
    virtual bool goToTime(uint64_t time);
};
#endif

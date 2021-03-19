
/***************************************************************************
                        flyDialog for Eq2-Gtk
                        (C) Mean 2007
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include <QGraphicsScene>
#include "ADM_vidEq2.h"

class flyEq2 : public ADM_flyDialogYuv
{
  private:
    Eq2Settings mySettings;
    bool        tablesDone;
  public:
    eq2         param;
    QGraphicsScene *scene;
  public:
    uint8_t     processYuv(ADMImage* in, ADMImage *out);
    uint8_t     download(void);
    uint8_t     upload(void);
    uint8_t     update(void);
    void        setTabOrder(void);

                flyEq2 (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                        ADM_QCanvas *canvas, ADM_QSlider *slider, QGraphicsScene *sc);
};


/***************************************************************************
                          DIA_flyZoom.cpp  -  description
                             -------------------

        Common part of the zoom dialog
    
    copyright            : (C) 2002/2017 by mean
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
#include "DIA_flyZoom.h"
#include "Q_zoom.h"
#include "ADM_toolkitQt.h"
/**
      \fn     DIA_getZoomParams
      \brief  Handle zoom dialog
*/
int DIA_getZoomParams(	const char *name,zoom *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;

    Ui_zoomWindow dialog(qtLastRegisteredDialog(), param,in);
    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }
    qtUnregisterDialog(&dialog);
    return ret;
}


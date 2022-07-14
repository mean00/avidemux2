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
#include "DIA_flyZoom.h"
#include "Q_zoom.h"
#include "ADM_QSettings.h"

/**
      \fn     DIA_getZoomParams
      \brief  Handle zoom dialog
*/
bool DIA_getZoomParams(const char *name, zoom *param, bool firstRun, ADM_coreVideoFilter *in)
{
    bool accepted = false;
    Ui_zoomWindow dialog(qtLastRegisteredDialog(), param, firstRun, in);
    qtRegisterDialog(&dialog);

    if(dialog.exec() == QDialog::Accepted)
    {
        dialog.gather(param);
        accepted = true;
    }

    QSettings *qset = qtSettingsCreate();
    if(qset)
    {
        qset->beginGroup("zoom");
        qset->setValue("rubberIsHidden", dialog.rubberIsHidden());
        if(accepted)
        {
            if (qset->value("saveAlgo", 0).toInt() == 1)
                qset->setValue("defaultAlgo", param->algo);
            if (qset->value("savePad", 0).toInt() == 1)
                qset->setValue("defaultPadding", param->pad);
        }
        qset->endGroup();
        delete qset;
        qset = NULL;
    }
    qtUnregisterDialog(&dialog);
    return accepted;
}


/**/
/***************************************************************************
                          DIA_flyMSharpen
                             -------------------

                           Ui for MPlayer DeLogo filter

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
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
#include "QSizeGrip"
#include "QHBoxLayout"
#include "ADM_default.h"
#include "ADM_image.h"
#include "msharpen.h"
#include "DIA_flymsharpen.h"
#include "ADM_vidMSharpen.h"
#include "Q_msharpen.h"
#include "ADM_toolkitQt.h"


#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif


/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
 flyMSharpen::flyMSharpen (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider) : 
                ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) 
 {
 }
 /**
  * 
  */
flyMSharpen::~flyMSharpen()
{
}


/**
    \fn process
*/
uint8_t    flyMSharpen::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
#if 0
    out->duplicate(in);
    if(preview)
        MPDelogo::doDelogo(out, param.xoff, param.yoff,
                             param.lw,  param.lh,param.band,param.show);        
    else
    {
        rubber->move(_zoom*(float)param.xoff,_zoom*(float)param.yoff);
        rubber->resize(_zoom*(float)param.lw,_zoom*(float)param.lh);
    }
#endif
    return 1;
}

/**
 * \fn upload
 * @return 
 */
uint8_t flyMSharpen::upload()
{
#define MYSPIN(x) w->x
#define MYTOGGLE(x) w->x
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;
    
    
    MYSPIN(spinBoxThreshold)->setValue(param.threshold);
    MYSPIN(spinBoxStrength)->setValue(param.strength);
    MYTOGGLE(CheckBoxHQ)->setChecked(param.highq);
    MYTOGGLE(checkBoxMask)->setChecked(param.mask);

    printf("Upload\n");
    return 1;
}
/**
        \fn download
*/
uint8_t flyMSharpen::download(void)
{
    
#define MYSPIN(x) w->x
#define MYTOGGLE(x) w->x
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;
    
    
    param.threshold=MYSPIN(spinBoxThreshold)->value();
    param.strength=MYSPIN(spinBoxStrength)->value();
    param.highq= MYTOGGLE(CheckBoxHQ)->isChecked();
    param.mask= MYTOGGLE(checkBoxMask)->isChecked();
    
    printf("Download\n");
    return true;
}

/**
      \fn     DIA_getMpDelogo
      \brief  Handle delogo dialog
*/
bool DIA_msharpen(msharpen &param, ADM_coreVideoFilter *in)
{
        bool ret=false;
        
        Ui_msharpenWindow dialog(qtLastRegisteredDialog(), &param,in);
		qtRegisterDialog(&dialog);

        if(dialog.exec()==QDialog::Accepted)
        {
            dialog.gather(&param); 
            ret=true;
        }

        qtUnregisterDialog(&dialog);
        return ret;
}

//EOF

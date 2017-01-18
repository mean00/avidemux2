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

/************* COMMON PART *********************/
/**
    \fn process
*/
uint8_t    flyMSharpen::processYuv(ADMImage* in, ADMImage *out)
{
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

//------------------------

uint8_t flyMSharpen::upload()
{
#if 0
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    if(!redraw)
    {
        blockChanges(true);
    }
    printf("Upload event : %d x %d , %d x %d\n",param.xoff,param.yoff,param.lw,param.lh);

    MYSPIN(spinX)->setValue(param.xoff);
    MYSPIN(spinY)->setValue(param.yoff);
    MYSPIN(spinW)->setValue(param.lw);
    MYSPIN(spinH)->setValue(param.lh);   
    MYSPIN(spinBand)->setValue(param.band);   

    if(!redraw)
    {
         blockChanges(false);
    }

#endif        
        printf("Upload\n");
        return 1;
}
/**
        \fn download
*/
uint8_t flyMSharpen::download(void)
{
#if 0
        Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
        param.xoff= MYSPIN(spinX)->value();
        param.yoff= MYSPIN(spinY)->value();
        param.lw= MYSPIN(spinW)->value();
        param.lh= MYSPIN(spinH)->value();
        param.band= MYSPIN(spinBand)->value();
#if 0        
        blockChanges(true);
        rubber->resize(_zoom*(float)param.lw,_zoom*(float)param.lh);
         blockChanges(false);
#endif         
        printf("Download\n");
#endif        
        return true;
}

/**
      \fn     DIA_getMpDelogo
      \brief  Handle delogo dialog
*/
bool DIA_getMSharpen(msharpen *param, ADM_coreVideoFilter *in)
{
        bool ret=false;
        
        Ui_msharpenWindow dialog(qtLastRegisteredDialog(), param,in);
		qtRegisterDialog(&dialog);

        if(dialog.exec()==QDialog::Accepted)
        {
            dialog.gather(param); 
            ret=true;
        }

        qtUnregisterDialog(&dialog);
        return ret;
}

//EOF

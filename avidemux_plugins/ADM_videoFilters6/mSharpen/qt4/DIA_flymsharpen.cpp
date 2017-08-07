/***************************************************************************
                          DIA_flyMSharpen
                             -------------------

                           Ui for msharpen
    copyright            : (C) 2004/2017 by mean
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
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) :
                ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) 
 {
     blur=  new ADMImageDefault(_w/2,_h);
     work=  new ADMImageDefault(_w,_h);;     //<-dafuq ?
 }
 /**
  * 
  */
flyMSharpen::~flyMSharpen()
{
    delete blur;
    delete work;
    blur=NULL;
    work=NULL;
}


/**
    \fn process
*/
uint8_t    flyMSharpen::processYuv(ADMImage* in, ADMImage *out)
{        
    
    ADMImageRef          refIn(_w/2,_h);
    ADMImageRefWrittable refOut(_w/2,_h);
    
    in->copyLeftSideTo(out);
    
    for(int i=0;i<3;i++)
    {
        int halfWidth=in->GetWidth((ADM_PLANE)i)/2; // in and out have the same width
        refIn._planeStride[i] =in->_planeStride[i];
        refOut._planeStride[i]=out->_planeStride[i];
        refIn._planes[i]      =in->_planes[i];//+halfWidth;
        refOut._planes[i]     =out->_planes[i]+halfWidth;
    }
    
    for (int i=0;i<3;i++)
    { 
            Msharpen::blur_plane(&refIn, blur, i,work);
            Msharpen::detect_edges(blur, &refOut,  i,param);
            if (param.highq == true)
                Msharpen::detect_edges_HiQ(blur, &refOut,  i,param);
            if (!param.mask) 
                Msharpen::apply_filter(&refIn, blur, &refOut,  i,param,invstrength);
    }
    out->copyInfo(in);
    out->printString(1,1,"Original"); // printString can't handle non-ascii input, do not translate this!
    out->printString(in->GetWidth(PLANAR_Y)/24+1,1,"Processed"); // as above, don't try to translate

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
    invstrength=255-param.strength;	
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
    invstrength=255-param.strength;	
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

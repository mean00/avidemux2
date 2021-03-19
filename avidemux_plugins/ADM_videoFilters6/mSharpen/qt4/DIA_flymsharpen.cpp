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
    uint32_t outw = _w;
    if(blur->_width != outw)
    {
        delete blur;
        blur = new ADMImageDefault(outw,_h);
    }

    ADMImageRef          refIn(outw,_h);
    ADMImageRefWrittable refOut(outw,_h);

    for(int i=0;i<3;i++)
    {
        int halfWidth=in->GetWidth((ADM_PLANE)i)/2; // in and out have the same width
        refIn._planeStride[i] =in->_planeStride[i];
        refOut._planeStride[i]=out->_planeStride[i];
        refIn._planes[i]      =in->_planes[i];//+halfWidth;
        refOut._planes[i] = out->_planes[i];
    }
    
    for (int i=0;i<(param.chroma ? 3:1);i++)
    { 
            Msharpen::blur_plane(&refIn, blur, i,work);
            Msharpen::detect_edges(blur, &refOut,  i,param);
            if (param.highq == true)
                Msharpen::detect_edges_HiQ(blur, &refOut,  i,param);
            if (!param.mask) 
                Msharpen::apply_filter(&refIn, blur, &refOut,  i,param,invstrength);
    }
    if (!param.chroma)
    {
        (&refOut)->copyPlane(&refIn,&refOut,PLANAR_U);
        (&refOut)->copyPlane(&refIn,&refOut,PLANAR_V);
    }
    out->copyInfo(in);

    return 1;
}

/**
 * \fn upload
 * @return 
 */
uint8_t flyMSharpen::upload()
{
#define MYSPIN(x,y) w->spinBox##x->setValue(param.y); w->horizontalSlider##x->setValue(param.y);
#define MYTOGGLE(x,y) w->checkBox##x->setChecked(param.y);
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;
    blockChanges(true);
    if(param.strength > 255) param.strength=255;

    MYSPIN(Strength,strength)
    MYSPIN(Threshold,threshold)
    MYTOGGLE(HQ,highq)
    MYTOGGLE(Mask,mask)
    MYTOGGLE(Chroma,chroma)
#undef MYSPIN
#undef MYTOGGLE
    blockChanges(false);
    invstrength = 255-param.strength;
    return 1;
}
/**
        \fn download
*/
uint8_t flyMSharpen::download(void)
{
#define MYSPIN(x,y) param.y=w->spinBox##x->value(); w->horizontalSlider##x->setValue(param.y);
#define MYTOGGLE(x,y) param.y=w->checkBox##x->isChecked();
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;
    blockChanges(true);

    MYSPIN(Strength,strength)
    MYSPIN(Threshold,threshold)
    MYTOGGLE(HQ,highq)
    MYTOGGLE(Mask,mask)
    MYTOGGLE(Chroma,chroma)
#undef MYSPIN
#undef MYTOGGLE
    blockChanges(false);
    if(param.strength > 255) param.strength=255;
    invstrength = 255-param.strength;
    return true;
}
/**
    \fn blockChanges
*/
#define APPLY_TO_ALL(x) w->horizontalSliderThreshold->x; w->horizontalSliderStrength->x; \
                        w->spinBoxThreshold->x; w->spinBoxStrength->x; \
                        w->checkBoxHQ->x; w->checkBoxChroma->x; w->checkBoxMask->x;
void flyMSharpen::blockChanges(bool block)
{
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;

    APPLY_TO_ALL(blockSignals(block))
}
/**
    \fn setTabOrder
*/
void flyMSharpen::setTabOrder(void)
{
    Ui_msharpenDialog *w=(Ui_msharpenDialog *)_cookie;
    std::vector<QWidget *> controls;

#define MYSPIN(x) controls.push_back(w->horizontalSlider##x); \
                  controls.push_back(w->spinBox##x);
#define MYTOGGLE(x) controls.push_back(w->checkBox##x);
    MYSPIN(Strength)
    MYSPIN(Threshold)
    MYTOGGLE(HQ)
    MYTOGGLE(Chroma)
    MYTOGGLE(Mask)

    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);

    QWidget *first, *second;

    for(std::vector<QWidget *>::iterator tor = controls.begin(); tor != controls.end(); ++tor)
    {
        if(tor+1 == controls.end()) break;
        first = *tor;
        second = *(tor+1);
        _parent->setTabOrder(first,second);
        //ADM_info("Tab order: %p (%s) --> %p (%s)\n",first,first->objectName().toUtf8().constData(),second,second->objectName().toUtf8().constData());
    }
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

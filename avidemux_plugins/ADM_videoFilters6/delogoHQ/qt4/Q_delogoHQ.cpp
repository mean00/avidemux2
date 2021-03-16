/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

			    GUI for cropping including autocrop
			    +Revisted the Gtk2 way
			     +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
    copyright            : (C) 2002/2007 by mean
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

#include "Q_delogoHQ.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidDelogoHQ.h"
#include <cmath>
#include "ADM_imageLoader.h"
#include "DIA_fileSel.h"
#include "ADM_last.h"
#include "DIA_coreToolkit.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_delogoHQWindow::Ui_delogoHQWindow(QWidget *parent, delogoHQ *param,ADM_coreVideoFilter *in) : QDialog(parent)
{

        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        admCoreUtils::getLastReadFolder(lastFolder);
        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyDelogoHQ( this,width, height,in,canvas,ui.horizontalSlider);
        ADMVideoDelogoHQ::DelogoHQCreateBuffers(width,height, &(myFly->rgbBufStride), &(myFly->rgbBufRaw), &(myFly->rgbBufImage), &(myFly->convertYuvToRgb), &(myFly->convertRgbToYuv));
        //memcpy(&(myFly->param),param,sizeof(delogoHQ));
        myFly->param.blur = param->blur;
        myFly->param.gradient = param->gradient;
        myFly->showOriginal = false;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->upload();
        myFly->sliderChanged();

        if(param->maskfile.size())
        {
            if(tryToLoadimage(param->maskfile.c_str()))
            {
                maskFName=param->maskfile;
            }
        }

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Blur,1,0)
        SPINNER(Gradient,1,0)

        connect( ui.pushButtonSave,SIGNAL(pressed()),this,SLOT(imageSave()));
        connect( ui.pushButtonLoad,SIGNAL(pressed()),this,SLOT(imageLoad()));

        setModal(true);
}
void Ui_delogoHQWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_delogoHQWindow::gather(delogoHQ *param)
{
    myFly->download();
    //memcpy(param,&(myFly->param),sizeof(delogoHQ));
    param->blur = myFly->param.blur;
    param->gradient = myFly->param.gradient;

    param->maskfile=maskFName;
}
Ui_delogoHQWindow::~Ui_delogoHQWindow()
{
    admCoreUtils::setLastReadFolder(lastFolder);
    if(myFly) {
        ADMVideoDelogoHQ::DelogoHQDestroyBuffers(myFly->rgbBufRaw, myFly->rgbBufImage, myFly->convertYuvToRgb, myFly->convertRgbToYuv);
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_delogoHQWindow::imageSave()
{
    char * buffer = (char *)malloc(2048);
    if (!buffer) return;
    std::string source;
    if(maskFName.size())
        source=maskFName;
    else
        source=lastFolder;
    if(FileSel_SelectWrite(QT_TRANSLATE_NOOP("delogoHQ","Save selected frame..."),buffer,2047,source.c_str(),"png"))
    {
        admCoreUtils::setLastReadFolder(std::string(buffer));
        myFly->saveCurrentFrame(buffer);
        buffer=NULL;
        myFly->sameImage();
    }
    if (buffer) free(buffer);
}
void Ui_delogoHQWindow::imageLoad()
{
    char buffer[2048];
    std::string source;
    if(maskFName.size())
        source=maskFName;
    else
        source=lastFolder;
    if(FileSel_SelectRead(QT_TRANSLATE_NOOP("delogoHQ","Load mask"),buffer,2048,source.c_str(),"png"))
    {
        admCoreUtils::setLastReadFolder(std::string(buffer));
        if(tryToLoadimage(buffer))
        {
            myFly->sameImage();
        }
    }
}
bool Ui_delogoHQWindow::tryToLoadimage(const char *filename)
{
    bool status=false;
    if(strlen(filename))
    {
        ADMImage * mask=createImageFromFile(filename);
        if(mask)
        {
            if (myFly->setMask(mask))
            {
                maskFName=std::string(filename);
                ui.lineEditImage->clear();
                ui.lineEditImage->insert(QString::fromStdString(maskFName));
                status=true;
            }
            delete mask;
        } else {
            GUI_Error_HIG(QT_TRANSLATE_NOOP("delogoHQ","Load failed!"), NULL);
        }
    }
    return status;
}
void Ui_delogoHQWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_delogoHQWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_delogoHQWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyDelogoHQ::upload(void)
{
    Ui_delogoHQDialog *w=(Ui_delogoHQDialog *)_cookie;
    MYSPIN(Blur)->setValue((int)param.blur);
    MYSPIN(Gradient)->setValue((int)param.gradient);
    return 1;
}

uint8_t flyDelogoHQ::download(void)
{
    Ui_delogoHQDialog *w=(Ui_delogoHQDialog *)_cookie;
    param.blur=(int)MYSPIN(Blur)->value();
    param.gradient=(int)MYSPIN(Gradient)->value();
    return 1;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getDelogoHQ(delogoHQ *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_delogoHQWindow dialog(qtLastRegisteredDialog(), param,in);

    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }

    qtUnregisterDialog(&dialog);

    return ret;
}
//____________________________________
// EOF



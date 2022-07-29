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
#include <QPushButton>
#include <QMessageBox>
#include "Q_cubicLUT.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidCubicLUT.h"
#include "ADM_imageLoader.h"
#include "DIA_fileSel.h"
#include "ADM_last.h"
#include "DIA_coreToolkit.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_cubicLUTWindow::Ui_cubicLUTWindow(QWidget *parent, cubicLUT *param,ADM_coreVideoFilter *in) : QDialog(parent)
{

        ui.setupUi(this);
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        admCoreUtils::getLastReadFolder(lastFolder);
        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyCubicLUT( this,width, height,in,canvas,ui.horizontalSlider);
        //memcpy(&(myFly->param),param,sizeof(cubicLUT));
        myFly->param.hald = param->hald;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, ControlOption::PeekOriginalBtn);
        myFly->setTabOrder();

        if(param->lutfile.size())
        {
            if (param->hald)
            {
                if(tryToLoadImage(param->lutfile.c_str()))
                {
                    lutFName=param->lutfile;
                }
            }
            else
            {
                if(tryToLoadCube(param->lutfile.c_str()))
                {
                    lutFName=param->lutfile;
                }
            }
        }
        myFly->refreshImage();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));

        connect( ui.pushButtonLoadHaldCLUT,SIGNAL(pressed()),this,SLOT(imageLoad()));
        connect( ui.pushButtonLoadCube,SIGNAL(pressed()),this,SLOT(cubeLoad()));
        disconnect( ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        connect( ui.buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));

        setModal(true);
}
void Ui_cubicLUTWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_cubicLUTWindow::gather(cubicLUT *param)
{
    myFly->download();
    //memcpy(param,&(myFly->param),sizeof(cubicLUT));
    param->hald = myFly->param.hald;

    param->lutfile=lutFName;
}
Ui_cubicLUTWindow::~Ui_cubicLUTWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
    admCoreUtils::setLastReadFolder(lastFolder);    // restore
}

void Ui_cubicLUTWindow::imageLoad()
{
    char buffer[2048];
    std::string source;
    if(lutFName.size())
        source=lutFName;
    else
        source=lastFolder;
    if(FileSel_SelectRead(QT_TRANSLATE_NOOP("cubicLUT","Load HaldCLUT"),buffer,2048,source.c_str(),"png"))
    {
        if(tryToLoadImage(buffer))
        {
            myFly->sameImage();
        }
    }
}

void Ui_cubicLUTWindow::cubeLoad()
{
    char buffer[2048];
    std::string source;
    if(lutFName.size())
        source=lutFName;
    else
        source=lastFolder;
    if(FileSel_SelectRead(QT_TRANSLATE_NOOP("cubicLUT","Load Cube"),buffer,2048,source.c_str(),"cube"))
    {
        if(tryToLoadCube(buffer))
        {
            myFly->sameImage();
        }
    }    
}

bool Ui_cubicLUTWindow::tryToLoadImage(const char *filename)
{
    bool status=false;
    if(strlen(filename))
    {
        if (!QApplication::overrideCursor())
            QApplication::setOverrideCursor(Qt::WaitCursor);
        const char * errorMsg = myFly->loadHald(filename);
        if (errorMsg == NULL)
        {
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();            
            lutFName=std::string(filename);
            ui.lineEditFile->clear();
            ui.lineEditFile->insert(QString::fromStdString(lutFName));
            status=true;
            myFly->lutValid=true;
        } else {
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();            
            GUI_Error_HIG(QT_TRANSLATE_NOOP("cubicLUT","Load failed"), errorMsg);
        }
    }
    return status;
}

bool Ui_cubicLUTWindow::tryToLoadCube(const char *filename)
{
    bool status=false;
    if(strlen(filename))
    {
        if (!QApplication::overrideCursor())
            QApplication::setOverrideCursor(Qt::WaitCursor);
        const char * errorMsg = myFly->loadCube(filename);
        if (errorMsg == NULL)
        {
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();            
            lutFName=std::string(filename);
            ui.lineEditFile->clear();
            ui.lineEditFile->insert(QString::fromStdString(lutFName));
            status=true;
            myFly->lutValid=true;
        } else {
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();            
            GUI_Error_HIG(QT_TRANSLATE_NOOP("cubicLUT","Load failed"), errorMsg);
        }
    }
    return status;
}


void Ui_cubicLUTWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_cubicLUTWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

//************************
void flyCubicLUT::setTabOrder(void)
{
    Ui_cubicLUTDialog *w=(Ui_cubicLUTDialog *)_cookie;
    std::vector<QWidget *> controls;

    controls.push_back(w->pushButtonLoadHaldCLUT);
    controls.push_back(w->pushButtonLoadCube);

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

void Ui_cubicLUTWindow::okButtonClicked()
{
    if (ui.lineEditFile->text().length() > 0)
        accept();
    else
        GUI_Error_HIG(QT_TRANSLATE_NOOP("cubicLUT","LUT file is not specified!"), NULL);
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getCubicLUT(cubicLUT *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_cubicLUTWindow dialog(qtLastRegisteredDialog(), param,in);

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



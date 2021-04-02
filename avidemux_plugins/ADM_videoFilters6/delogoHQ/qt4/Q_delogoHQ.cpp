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
#include "Q_delogoHQ.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidDelogoHQ.h"
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
        //memcpy(&(myFly->param),param,sizeof(delogoHQ));
        myFly->param.blur = param->blur;
        myFly->param.gradient = param->gradient;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        if(param->maskfile.size())
        {
            if(tryToLoadimage(param->maskfile.c_str()))
            {
                maskFName=param->maskfile;
            }
        }
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSpinBox(int)));
        SPINNER(Blur)
        SPINNER(Gradient)

        QPushButton *helpButton = ui.buttonBox->button(QDialogButtonBox::Help);
        connect(helpButton,SIGNAL(clicked()),this,SLOT(showHelp()));

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
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_delogoHQWindow::showHelp()
{
    QString usage = "<ol><li>";

    const char *text = QT_TRANSLATE_NOOP("delogoHQ","Find a frame in the preview with the logo clearly visible and not blending into the background.");
    usage += QString::fromUtf8(text);

    usage += "<br></li><li>";

    text = QT_TRANSLATE_NOOP("delogoHQ","Save this frame as a PNG image.");
    usage += QString::fromUtf8(text);

    usage += "<br></li><li>";

    text = QT_TRANSLATE_NOOP("delogoHQ","Use an appropriate image editor to paint the area of the logo of any shape white and everything else solid black.");
    usage += QString::fromUtf8(text);

    usage += "<br><br>";

    text = QT_TRANSLATE_NOOP("delogoHQ","This black and white image will serve as a mask where white pixels correspond to the logo to be removed.");
    usage += QString::fromUtf8(text);

    usage += "<br><br>";

    text = QT_TRANSLATE_NOOP("delogoHQ","If the logo has fully transparent areas, it is recommended to exclude them from the mask by making them black.");
    usage += QString::fromUtf8(text);

    usage += "<br></li><li>";

    text = QT_TRANSLATE_NOOP("delogoHQ","Load the mask image.");
    usage += QString::fromUtf8(text);

    usage += "<br></li></ol><p>";

    text = QT_TRANSLATE_NOOP("delogoHQ","Note: To remove multiple distant logos (e.g. opposite corners), using separate filter instances for each logo will be much faster.");
    usage += QString::fromUtf8(text);

    usage += "<br></p>";

    QString title = QString::fromUtf8(QT_TRANSLATE_NOOP("delogoHQ","How to use DelogoHQ"));
    QMessageBox msgBoxUsage(QMessageBox::Information, title, usage, QMessageBox::Ok, qtLastRegisteredDialog());
#ifdef __APPLE__
    QWidget *forceTitle = static_cast<QWidget *>(&msgBoxUsage);
    forceTitle->setWindowTitle(title);
#endif
    msgBoxUsage.exec();
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
#define COPY_VALUE_TO_SPINBOX(x) \
        ui.spinBox##x->blockSignals(true); \
        ui.spinBox##x->setValue(ui.horizontalSlider##x->value()); \
        ui.spinBox##x->blockSignals(false);
void Ui_delogoHQWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Blur);
    COPY_VALUE_TO_SPINBOX(Gradient);
    myFly->download();
    myFly->sameImage();
    lock--;
}
#define COPY_VALUE_TO_SLIDER(x) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue(ui.spinBox##x->value()); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_delogoHQWindow::valueChangedSpinBox(int foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Blur);
    COPY_VALUE_TO_SLIDER(Gradient);
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
#define MYSLIDER(x) w->horizontalSlider##x
#define MYSPIN(x) w->spinBox##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADSPIN(x, value) \
        w->spinBox##x->blockSignals(true); \
        w->spinBox##x->setValue(value); \
        w->spinBox##x->blockSignals(false);
//************************
uint8_t flyDelogoHQ::upload(void)
{
    Ui_delogoHQDialog *w=(Ui_delogoHQDialog *)_cookie;
    MYSLIDER(Blur)->setValue((int)param.blur);
    UPLOADSPIN(Blur, param.blur);
    MYSLIDER(Gradient)->setValue((int)param.gradient);
    UPLOADSPIN(Gradient, param.gradient);
    return 1;
}

uint8_t flyDelogoHQ::download(void)
{
    Ui_delogoHQDialog *w=(Ui_delogoHQDialog *)_cookie;
    param.blur=(int)MYSLIDER(Blur)->value();
    param.gradient=(int)MYSLIDER(Gradient)->value();
    return 1;
}

void flyDelogoHQ::setTabOrder(void)
{
    Ui_delogoHQDialog *w=(Ui_delogoHQDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_DIAL(x) controls.push_back(MYDIAL(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    controls.push_back(w->pushButtonSave);
    controls.push_back(w->pushButtonLoad);
    PUSH_SLIDER(Blur)
    PUSH_SPIN(Blur)
    PUSH_SLIDER(Gradient)
    PUSH_SPIN(Gradient)

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



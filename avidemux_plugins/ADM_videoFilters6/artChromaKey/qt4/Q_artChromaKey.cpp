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

#include "Q_artChromaKey.h"
#include "ADM_toolkitQt.h"
#include <cmath>
#include <QColorDialog>
#include <QPalette>
#include <QString>
#include <QImage>
#include <QPixmap>
#include "ADM_imageLoader.h"
#include "DIA_fileSel.h"
#include "ADM_last.h"

static void rgb2yuv(int * yuv, int * rgb);
static void yuv2rgb(int * rgb, int * yuv);

//
//	Video is in YV12 Colorspace
//
//
Ui_artChromaKeyWindow::Ui_artChromaKeyWindow(QWidget *parent, artChromaKey *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        admCoreUtils::getLastReadFolder(lastFolder);
        image = new ADMImageDefault(width, height);
        image->blacken();
        if(param->imagefile.size())
        {
            if(tryToLoadimage(param->imagefile.c_str()))
            {
                imageName=param->imagefile;
            }
        }
        testimage = new ADMImageDefault(width, height);
        testimage->blacken();

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        scene=new QGraphicsScene(this);
        ui.graphicsViewUV->setScene(scene);
        ui.graphicsViewUV->scale(1.0,1.0);

        myFly=new flyArtChromaKey( this,width, height,in,canvas,ui.horizontalSlider, scene);
        //memcpy(&(myFly->param),param,sizeof(artChromaKey));
#define COPYPARAMTOFLY(x) \
            myFly->param.c##x##en = param->c##x##en; \
            myFly->param.c##x##u = param->c##x##u; \
            myFly->param.c##x##v = param->c##x##v; \
            myFly->param.c##x##dist = param->c##x##dist; \
            myFly->param.c##x##slope = param->c##x##slope;
        COPYPARAMTOFLY(1);
        COPYPARAMTOFLY(2);
        COPYPARAMTOFLY(3);
        myFly->param.spill = param->spill;

        myFly->showTestImage = false;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(C1en);
        CHKBOX(C2en);
        CHKBOX(C3en);
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(C1dist,100,2);
        SPINNER(C1slope,100,2);
        SPINNER(C2dist,100,2);
        SPINNER(C2slope,100,2);
        SPINNER(C3dist,100,2);
        SPINNER(C3slope,100,2);
#define PUSHB(x) connect(ui.pushButton##x,SIGNAL(released()),this,SLOT(pushed##x()));
        PUSHB(C1);
        PUSHB(C2);
        PUSHB(C3);
        connect( ui.pushButtonImage,SIGNAL(pressed()),this,SLOT(imageSelect()));
        ui.checkBoxTest->setChecked(false);
        connect(ui.checkBoxTest,SIGNAL(stateChanged(int)),this,SLOT(testImageChanged(int)));
        connect(ui.comboBoxSpill, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));



        setModal(true);
}
void Ui_artChromaKeyWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artChromaKeyWindow::gather(artChromaKey *param)
{
#define COPYPARAMFROMFLY(x) \
        param->c##x##en = myFly->param.c##x##en; \
        param->c##x##u = myFly->param.c##x##u; \
        param->c##x##v = myFly->param.c##x##v; \
        param->c##x##dist = myFly->param.c##x##dist; \
        param->c##x##slope = myFly->param.c##x##slope;
    myFly->download();
    //memcpy(param,&(myFly->param),sizeof(artChromaKey));
    COPYPARAMFROMFLY(1);
    COPYPARAMFROMFLY(2);
    COPYPARAMFROMFLY(3);
    param->spill = myFly->param.spill;
    param->imagefile=imageName;
}
Ui_artChromaKeyWindow::~Ui_artChromaKeyWindow()
{
    admCoreUtils::setLastReadFolder(lastFolder);
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
    //if (scene) delete scene;
    scene=NULL;
    if (image) delete image;
    image = NULL;
    if (testimage) delete testimage;
    testimage = NULL;
}
void Ui_artChromaKeyWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_artChromaKeyWindow::testImageChanged(int f)
{
    if (ui.checkBoxTest->isChecked())
    {
        myFly->download();
        if (myFly->param.c1en || myFly->param.c2en || myFly->param.c3en)
        {
            float su = 0.0;
            float sv = 0.0;
            float sc = 0.0;
            int iu,iv;
            if (myFly->param.c1en)
            {
                su += myFly->param.c1u;
                sv += myFly->param.c1v;
                sc += 1.0;
            }
            if (myFly->param.c2en)
            {
                su += myFly->param.c2u;
                sv += myFly->param.c2v;
                sc += 1.0;
            }
            if (myFly->param.c2en)
            {
                su += myFly->param.c2u;
                sv += myFly->param.c2v;
                sc += 1.0;
            }
            su /= sc;
            sv /= sc;
            su = std::round(-1.0*su*128+128);
            sv = std::round(-1.0*sv*128+128);
            if (su < 0.0) su = 0.0;
            if (su > 255.0) su = 255.0;
            if (sv < 0.0) sv = 0.0;
            if (sv > 255.0) sv = 255.0;
            iu = su;
            iv = sv;

            int width=testimage->GetWidth(PLANAR_Y); 
            int height=testimage->GetHeight(PLANAR_Y);
            int stride;
            uint8_t * ptr;

            stride=testimage->GetPitch(PLANAR_Y);
            ptr=testimage->GetWritePtr(PLANAR_Y);
            for (int y=0; y<height; y++)
            {
                memset(ptr,128,width);
                ptr += stride;
            }

            stride=testimage->GetPitch(PLANAR_U);
            ptr=testimage->GetWritePtr(PLANAR_U);
            for (int y=0; y<height/2; y++)
            {
                memset(ptr,iu,width/2);
                ptr += stride;
            }

            stride=testimage->GetPitch(PLANAR_V);
            ptr=testimage->GetWritePtr(PLANAR_V);
            for (int y=0; y<height/2; y++)
            {
                memset(ptr,iv,width/2);
                ptr += stride;
            }

        } else {
            testimage->blacken();
        }
        myFly->showTestImage = true;
    } else {
        myFly->showTestImage = false;
    }
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}

#define PUSHHANDLER(x) \
    QPalette indctrPalette(ui.lineEditC##x->palette()); \
    QColor startColor = indctrPalette.color(QPalette::Window); \
    QColor color = QColorDialog::getColor(startColor, this ); \
    if( color.isValid() ) \
    { \
        int rgb[3],yuv[3]; \
        color.getRgb(rgb+0,rgb+1,rgb+2, NULL); \
        rgb2yuv(yuv, rgb); \
        yuv[0] = 128; \
        myFly->param.c##x##u = (float)yuv[1]/128.0; \
        myFly->param.c##x##v = (float)yuv[2]/128.0; \
        yuv2rgb(rgb, yuv); \
        color.setRgb(rgb[0],rgb[1],rgb[2],255); \
        indctrPalette.setColor(QPalette::Window,color); \
        indctrPalette.setColor(QPalette::Base,color); \
        indctrPalette.setColor(QPalette::AlternateBase,color); \
        ui.lineEditC##x->setPalette(indctrPalette); \
        if(!lock) \
        { \
            lock++; \
            myFly->download(); \
            myFly->sameImage(); \
            lock--; \
        } \
    }

void Ui_artChromaKeyWindow::pushedC1()
{
    PUSHHANDLER(1);
}
void Ui_artChromaKeyWindow::pushedC2()
{
    PUSHHANDLER(2);
}
void Ui_artChromaKeyWindow::pushedC3()
{
    PUSHHANDLER(3);
}
void Ui_artChromaKeyWindow::imageSelect()
{
    char buffer[2048];
    std::string source;
    if(imageName.size())
        source=imageName;
    else
        source=lastFolder;
    if(FileSel_SelectRead(QT_TRANSLATE_NOOP("artChromaKey","Select Background Image"),buffer,2048,source.c_str()))
    {
        admCoreUtils::setLastReadFolder(std::string(buffer));
        if(tryToLoadimage(buffer))
        {
            myFly->sameImage();
        }
    }
}
bool Ui_artChromaKeyWindow::tryToLoadimage(const char *filename)
{
    bool status=false;
    if(strlen(filename))
    {
        ADMImage *im2=createImageFromFile(filename);
        if(im2)
        {
            ADMColorScalerFull * scaler = new ADMColorScalerFull(ADM_CS_BICUBIC,im2->GetWidth(PLANAR_Y),im2->GetHeight(PLANAR_Y),image->GetWidth(PLANAR_Y),image->GetHeight(PLANAR_Y),im2->_colorspace,ADM_COLOR_YV12);
            if (scaler)
            {
                imageName=std::string(filename);
                ui.lineEditImage->clear();
                ui.lineEditImage->insert(QString::fromStdString(imageName));

                scaler->convertImage(im2, image);
                if(image->_range == ADM_COL_RANGE_MPEG)
                    image->expandColorRange();
                delete scaler;
            }
            delete im2;
            status=true;
        }
    }
    return status;
}

void rgb2yuv(int * yuv, int * rgb)
{
    yuv[0] = std::round( 0.299*rgb[0] + 0.587*rgb[1] + 0.114*rgb[2]);
    yuv[1] = std::round(-0.169*rgb[0] - 0.331*rgb[1] + 0.500*rgb[2]);
    yuv[2] = std::round( 0.500*rgb[0] - 0.419*rgb[1] - 0.081*rgb[2]);
    if (yuv[0] <   0) yuv[0] = 0;
    if (yuv[0] > 255) yuv[0] = 255;
    if (yuv[1] < -128) yuv[1] = -128;
    if (yuv[1] >  127) yuv[1] = 127;
    if (yuv[2] < -128) yuv[2] = -128;
    if (yuv[2] >  127) yuv[2] = 127;
}
void yuv2rgb(int * rgb, int * yuv)
{
    rgb[0] = std::round(yuv[0]                +   1.4*yuv[2]);
    rgb[1] = std::round(yuv[0] - 0.343*yuv[1] - 0.711*yuv[2]);
    rgb[2] = std::round(yuv[0] + 1.765*yuv[1]               );
    for (int i=0; i<3; i++)
    {
        if (rgb[i] < 0) rgb[i] = 0;
        if (rgb[i] > 255) rgb[i] = 255;
    }
}

void Ui_artChromaKeyWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artChromaKeyWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADYUV(x) { \
    QLineEdit * indctr = w->lineEditC##x; \
    QPalette indctrPalette(indctr->palette()); \
    QColor color; \
    yuv[0] = 128; \
    yuv[1] = (int)round(param.c##x##u*128.0); \
    yuv[2] = (int)round(param.c##x##v*128.0); \
    yuv2rgb(rgb, yuv); \
    color.setRgb(rgb[0],rgb[1],rgb[2],255); \
    indctrPalette.setColor(QPalette::Window,color); \
    indctrPalette.setColor(QPalette::Base,color); \
    indctrPalette.setColor(QPalette::AlternateBase,color); \
    indctr->setPalette(indctrPalette); \
    }

//************************
uint8_t flyArtChromaKey::upload(void)
{
    Ui_artChromaKeyDialog *w=(Ui_artChromaKeyDialog *)_cookie;

    MYCHECK(C1en)->setChecked(param.c1en);
    MYCHECK(C2en)->setChecked(param.c2en);
    MYCHECK(C3en)->setChecked(param.c3en);
    MYSPIN(C1dist)->setValue((int)round(param.c1dist*100.0));
    MYSPIN(C1slope)->setValue((int)round(param.c1slope*100.0));
    MYSPIN(C2dist)->setValue((int)round(param.c2dist*100.0));
    MYSPIN(C2slope)->setValue((int)round(param.c2slope*100.0));
    MYSPIN(C3dist)->setValue((int)round(param.c3dist*100.0));
    MYSPIN(C3slope)->setValue((int)round(param.c3slope*100.0));
    MYCOMBOX(Spill)->setCurrentIndex(param.spill);


    int rgb[3],yuv[3];
    UPLOADYUV(1);
    UPLOADYUV(2);
    UPLOADYUV(3);

    drawScene();
    return 1;
}

uint8_t flyArtChromaKey::download(void)
{
    Ui_artChromaKeyDialog *w=(Ui_artChromaKeyDialog *)_cookie;
    param.c1en=MYCHECK(C1en)->isChecked();
    param.c2en=MYCHECK(C2en)->isChecked();
    param.c3en=MYCHECK(C3en)->isChecked();
    param.c1dist=((float)MYSPIN(C1dist)->value()) / 100.0;
    param.c1slope=((float)MYSPIN(C1slope)->value()) / 100.0;
    param.c2dist=((float)MYSPIN(C2dist)->value()) / 100.0;
    param.c2slope=((float)MYSPIN(C2slope)->value()) / 100.0;
    param.c3dist=((float)MYSPIN(C3dist)->value()) / 100.0;
    param.c3slope=((float)MYSPIN(C3slope)->value()) / 100.0;
    param.spill=MYCOMBOX(Spill)->currentIndex();

    drawScene();
    return 1;
}
void flyArtChromaKey::drawScene()
{
    #define YUVmapSize (124)
    uint8_t * uvplane = (uint8_t*)malloc(YUVmapSize*YUVmapSize);
    QImage * img = new QImage(YUVmapSize,YUVmapSize,QImage::Format_RGB32);
    if(scene && uvplane && img)
    {
        bool cen[3];
        float cu[3];
        float cv[3];
        float cdist[3];
        float cslope[3];
        cen[0] = param.c1en;
        cu[0] = param.c1u;
        cv[0] = param.c1v;
        cdist[0] = param.c1dist;
        cslope[0] = param.c1slope;
        cen[1] = param.c2en;
        cu[1] = param.c2u;
        cv[1] = param.c2v;
        cdist[1] = param.c2dist;
        cslope[1] = param.c2slope;
        cen[2] = param.c3en;
        cu[2] = param.c3u;
        cv[2] = param.c3v;
        cdist[2] = param.c3dist;
        cslope[2] = param.c3slope;
        float fi,fj;
        float diff,slp;
        uint8_t uvcurr,uvnew;
        if (cen[0] || cen[1] || cen[2])
            memset(uvplane, 0, YUVmapSize*YUVmapSize);
        else
            memset(uvplane, 255, YUVmapSize*YUVmapSize);    // if no one enabled --> show full colors

        for (int c=0; c<3; c++)
        {
            if (!cen[c]) continue;
            for (int i=0; i<YUVmapSize; i++) for(int j=0; j<YUVmapSize; j++)
            {
                fi = ((float)i-YUVmapSize/2.0)/(YUVmapSize/2.0);
                fj = ((float)j-YUVmapSize/2.0)/(YUVmapSize/2.0);
                diff = std::sqrt(((cu[c]-fi)*(cu[c]-fi) + (cv[c]-fj)*(cv[c]-fj))) - cdist[c];
                if (diff <= 0) uvplane[i*YUVmapSize+j] = 255;
                else {
                    uvcurr = uvplane[i*YUVmapSize+j];
                    if ((diff > cslope[c]) || (cslope[c] == 0.0)) uvnew = 0;
                    else {
                        slp = (diff / cslope[c]);
                        if (slp < 0) slp = 0;
                        if (slp > 1) slp = 1;
                        uvnew = std::floor(255.0 - 255.0*slp);
                    }
                    if (uvnew > uvcurr) uvplane[i*YUVmapSize+j] = uvnew;
                }
            }
        }

        int rgb[3],yuv[3];

        for (int i=0; i<YUVmapSize; i++) for(int j=0; j<YUVmapSize; j++)
        {
            fi = ((float)uvplane[i*YUVmapSize+j]/255.0)*((float)i-YUVmapSize/2.0)/(YUVmapSize/2.0);
            fj = ((float)uvplane[i*YUVmapSize+j]/255.0)*((float)j-YUVmapSize/2.0)/(YUVmapSize/2.0);
            yuv[0] = uvplane[i*YUVmapSize+j]>>1;
            yuv[1] = std::floor(fi*128.0);
            yuv[2] = std::floor(fj*128.0);
            yuv2rgb(rgb, yuv);
            img->setPixel(i,(YUVmapSize-1-j),qRgb(rgb[0],rgb[1],rgb[2]));
        }
        scene->clear();
        scene->addPixmap( QPixmap::fromImage(*img)); 
    }
    if (img) delete img;
    img = NULL;
    if (uvplane) free(uvplane);
    uvplane = NULL;
}
void flyArtChromaKey::setTabOrder(void)
{
    Ui_artChromaKeyDialog *w=(Ui_artChromaKeyDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHME(x) controls.push_back(w->x);
    PUSHME(pushButtonImage)
    PUSHME(checkBoxTest)
    PUSHME(comboBoxSpill)
#undef PUSHME
#define PUSHME(x) controls.push_back(MYCHECK(x##en));  controls.push_back(w->pushButton##x); \
                  controls.push_back(MYSPIN(x##dist)); controls.push_back(MYSPIN(x##slope));
    PUSHME(C1)
    PUSHME(C2)
    PUSHME(C3)

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
uint8_t DIA_getArtChromaKey(artChromaKey *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artChromaKeyWindow dialog(qtLastRegisteredDialog(), param,in);

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



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

#include "Q_artChromaHold.h"
#include "ADM_toolkitQt.h"
#include <cmath>
#include <QColorDialog>
#include <QPalette>
#include <QString>
#include <QImage>
#include <QPixmap>
//
//	Video is in YV12 Colorspace
//
//
Ui_artChromaHoldWindow::Ui_artChromaHoldWindow(QWidget *parent, artChromaHold *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        scene=new QGraphicsScene(this);
        ui.graphicsViewUV->setScene(scene);
        ui.graphicsViewUV->scale(1.0,1.0);

        myFly=new flyArtChromaHold( this,width, height,in,canvas,ui.horizontalSlider, scene);
        memcpy(&(myFly->param),param,sizeof(artChromaHold));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
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

        setModal(true);
}
void Ui_artChromaHoldWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artChromaHoldWindow::gather(artChromaHold *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artChromaHold));
}
Ui_artChromaHoldWindow::~Ui_artChromaHoldWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
    scene=NULL;
}
void Ui_artChromaHoldWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_artChromaHoldWindow::pushedC1()
{
    pushed(ui.lineEditC1);
}
void Ui_artChromaHoldWindow::pushedC2()
{
    pushed(ui.lineEditC2);
}
void Ui_artChromaHoldWindow::pushedC3()
{
    pushed(ui.lineEditC3);
}
void Ui_artChromaHoldWindow::rgb2yuv(int * yuv, int * rgb)
{
    yuv[0] = ( 77*rgb[0] +  150*rgb[1] +  29*rgb[2] + 128) >> 8;
    yuv[1] = (-43*rgb[0] +  -84*rgb[1] + 127*rgb[2] + 128) >> 8;
    yuv[2] = (127*rgb[0] + -106*rgb[1] + -21*rgb[2] + 128) >> 8;
}
void Ui_artChromaHoldWindow::yuv2rgb(int * rgb, int * yuv)
{
    rgb[0] = (256*yuv[0] +   0*yuv[1] +  358*yuv[2] + 128) >> 8;
    rgb[1] = (256*yuv[0] + -88*yuv[1] + -182*yuv[2] + 128) >> 8;
    rgb[2] = (256*yuv[0] + 452*yuv[1] +    0*yuv[2] + 128) >> 8;
    for (int i=0; i<3; i++)
    {
        if (rgb[i] < 0) rgb[i] = 0;
        if (rgb[i] > 255) rgb[i] = 255;
    }
}
void Ui_artChromaHoldWindow::pushed(QLineEdit * indctr)
{
    if (!indctr) return;
    QPalette indctrPalette(indctr->palette());
    QColor startColor = indctrPalette.color(QPalette::Background);
    QColor color = QColorDialog::getColor(startColor, this );
    if( color.isValid() )
    {
        int rgb[3],yuv[3];
        color.getRgb(rgb+0,rgb+1,rgb+2, NULL);
        rgb2yuv(yuv, rgb);
        yuv[0] = 128;
        yuv2rgb(rgb, yuv);
        color.setRgb(rgb[0],rgb[1],rgb[2],255);
        indctrPalette.setColor(QPalette::Background,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        indctr->setPalette(indctrPalette);
        if(!lock)
        {
            lock++;
            myFly->download();
            myFly->sameImage();
            lock--;
        }
    }
}

void Ui_artChromaHoldWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artChromaHoldWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADYUV(x) { \
    QLineEdit * indctr = w->lineEditC##x; \
    QPalette indctrPalette(indctr->palette()); \
    QColor color; \
    yuv[0] = 128; \
    yuv[1] = (int)round(param.c##x##u*128.0); \
    yuv[2] = (int)round(param.c##x##v*128.0); \
    Ui_artChromaHoldWindow::yuv2rgb(rgb, yuv); \
    color.setRgb(rgb[0],rgb[1],rgb[2],255); \
    indctrPalette.setColor(QPalette::Background,color); \
    indctrPalette.setColor(QPalette::Base,color); \
    indctrPalette.setColor(QPalette::AlternateBase,color); \
    indctr->setPalette(indctrPalette); \
    }

//************************
uint8_t flyArtChromaHold::upload(void)
{
    Ui_artChromaHoldDialog *w=(Ui_artChromaHoldDialog *)_cookie;

    MYCHECK(C1en)->setChecked(param.c1en);
    MYCHECK(C2en)->setChecked(param.c2en);
    MYCHECK(C3en)->setChecked(param.c3en);
    MYSPIN(C1dist)->setValue((int)round(param.c1dist*100.0));
    MYSPIN(C1slope)->setValue((int)round(param.c1slope*100.0));
    MYSPIN(C2dist)->setValue((int)round(param.c2dist*100.0));
    MYSPIN(C2slope)->setValue((int)round(param.c2slope*100.0));
    MYSPIN(C3dist)->setValue((int)round(param.c3dist*100.0));
    MYSPIN(C3slope)->setValue((int)round(param.c3slope*100.0));

    int rgb[3],yuv[3];
    UPLOADYUV(1);
    UPLOADYUV(2);
    UPLOADYUV(3);

    drawScene();
    return 1;
}

#define DOWNLOADYUV(x) { \
    QLineEdit * indctr = w->lineEditC##x; \
    QPalette indctrPalette(indctr->palette()); \
    QColor color = indctrPalette.color(QPalette::Background); \
    color.getRgb(rgb+0,rgb+1,rgb+2, NULL); \
    Ui_artChromaHoldWindow::rgb2yuv(yuv, rgb); \
    param.c##x##u = (float)yuv[1]/128.0; \
    param.c##x##v = (float)yuv[2]/128.0; \
    }
uint8_t flyArtChromaHold::download(void)
{
    Ui_artChromaHoldDialog *w=(Ui_artChromaHoldDialog *)_cookie;
    param.c1en=MYCHECK(C1en)->isChecked();
    param.c2en=MYCHECK(C2en)->isChecked();
    param.c3en=MYCHECK(C3en)->isChecked();
    param.c1dist=((float)MYSPIN(C1dist)->value()) / 100.0;
    param.c1slope=((float)MYSPIN(C1slope)->value()) / 100.0;
    param.c2dist=((float)MYSPIN(C2dist)->value()) / 100.0;
    param.c2slope=((float)MYSPIN(C2slope)->value()) / 100.0;
    param.c3dist=((float)MYSPIN(C3dist)->value()) / 100.0;
    param.c3slope=((float)MYSPIN(C3slope)->value()) / 100.0;

    int rgb[3],yuv[3];
    DOWNLOADYUV(1);
    DOWNLOADYUV(2);
    DOWNLOADYUV(3);

    drawScene();
    return 1;
}
void flyArtChromaHold::drawScene()
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
            Ui_artChromaHoldWindow::yuv2rgb(rgb, yuv);
            img->setPixel(i,j,qRgb(rgb[0],rgb[1],rgb[2]));
        }
        scene->clear();
        scene->addPixmap( QPixmap::fromImage(*img)); 
    }
    if (img) delete img;
    if (uvplane) free(uvplane);
}


/**
    \fn     DIA_getCropParams
    \brief  Handle crop dialog
*/
uint8_t DIA_getArtChromaHold(artChromaHold *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artChromaHoldWindow dialog(qtLastRegisteredDialog(), param,in);

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



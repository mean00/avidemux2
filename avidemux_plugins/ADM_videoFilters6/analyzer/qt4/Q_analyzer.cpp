/***************************************************************************
                          Analyzer filter 
        Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Q_analyzer.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidAnalyzer.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_analyzerWindow::Ui_analyzerWindow(QWidget *parent, ADM_coreVideoFilter *in) : QDialog(parent)
{
    ui.setupUi(this);
    firstRun = true;
    // Allocate space for green-ised video
    uint32_t _width=in->getInfo()->width;
    uint32_t _height=in->getInfo()->height;
    _in = in;

    canvas=new ADM_QCanvas(ui.graphicsView,_width,_height);

    sceneVectorScope=new QGraphicsScene(this);
    sceneVectorScope->setSceneRect(0,0,620,600);
    ui.graphicsViewVectorScope->setScene(sceneVectorScope);
    ui.graphicsViewVectorScope->scale(0.5,0.5);

    sceneYUVparade=new QGraphicsScene(this);
    sceneYUVparade->setSceneRect(0,0,772,258);
    ui.graphicsViewYUVparade->setScene(sceneYUVparade);
    ui.graphicsViewYUVparade->scale(0.5,0.5);

    sceneRGBparade=new QGraphicsScene(this);
    sceneRGBparade->setSceneRect(0,0,772,258);
    ui.graphicsViewRGBparade->setScene(sceneRGBparade);
    ui.graphicsViewRGBparade->scale(0.5,0.5);

    sceneHistograms=new QGraphicsScene(this);
    sceneHistograms->setSceneRect(0,0,772,259);
    ui.graphicsViewHistograms->setScene(sceneHistograms);
    ui.graphicsViewHistograms->scale(0.5,0.5);

    myFly = new flyAnalyzer(this,_width, _height,_in,canvas,ui.horizontalSlider,sceneVectorScope, sceneYUVparade, sceneRGBparade, sceneHistograms);
    myFly->_cookie=&ui;
    myFly->addControl(ui.toolboxLayout);
    myFly->setTabOrder();
    myFly->upload();
    myFly->sliderChanged();

    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));

    setModal(true);

}
void Ui_analyzerWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
Ui_analyzerWindow::~Ui_analyzerWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_analyzerWindow::adjustGraphs(void)
{
    QRectF bounds;

    bounds = sceneVectorScope->itemsBoundingRect();
    ui.graphicsViewVectorScope->fitInView(bounds, Qt::KeepAspectRatio);

    bounds = sceneYUVparade->itemsBoundingRect();
    ui.graphicsViewYUVparade->fitInView(bounds, Qt::KeepAspectRatio);

    bounds = sceneRGBparade->itemsBoundingRect();
    ui.graphicsViewRGBparade->fitInView(bounds, Qt::KeepAspectRatio);

    bounds = sceneHistograms->itemsBoundingRect();
    ui.graphicsViewHistograms->fitInView(bounds, Qt::KeepAspectRatio);
}
void Ui_analyzerWindow::resizeEvent(QResizeEvent *event)
{
    adjustGraphs();

    if(!canvas->height())
        return;
    myFly->fitCanvasIntoView(ui.graphicsView->width(), ui.graphicsView->height());
    myFly->adjustCanvasPosition();
}

void Ui_analyzerWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    adjustGraphs();

    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
    if (firstRun)
    {
        adjustSize();
        firstRun = false;
    }
}

void flyAnalyzer::setTabOrder(void)
{
    Ui_analyzerDialog *w=(Ui_analyzerDialog *)_cookie;
    std::vector<QWidget *> controls;

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
uint8_t DIA_getAnalyzer(ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_analyzerWindow dialog(qtLastRegisteredDialog(),in);

    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        ret=1;
    }

    qtUnregisterDialog(&dialog);

    return ret;
}


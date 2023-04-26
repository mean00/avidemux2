/***************************************************************************
                           Q_seekablePreview.cpp
                           ---------------------

    begin                : Fri Sep 5 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Q_seekablePreview.h"
#include "ADM_vidMisc.h"
/**
 * 
 * @param parent
 * @param videoStream
 * @param defaultFrame
 */
Ui_seekablePreviewWindow::Ui_seekablePreviewWindow(QWidget *parent, ADM_coreVideoFilter *videoStream) : QDialog(parent)
{
    ui.setupUi(this);

    seekablePreview = NULL;
    canvas = NULL;

    resetVideoStream(videoStream);

    seekablePreview->addControl(ui.toolLayout);

    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

    QT6_CRASH_WORKAROUND(seekablePreviewWindow)
}

Ui_seekablePreviewWindow::~Ui_seekablePreviewWindow()
{
    delete seekablePreview;
    delete canvas;
}

/**
 * 
 * @param videoStream
 */
void Ui_seekablePreviewWindow::resetVideoStream(ADM_coreVideoFilter *videoStream)
{
    if (seekablePreview)
        delete seekablePreview;
    seekablePreview=NULL;
    if (canvas)
        delete canvas;
    canvas=NULL;

    uint32_t canvasWidth = videoStream->getInfo()->width;
    uint32_t canvasHeight = videoStream->getInfo()->height;

    canvas = new ADM_QCanvas(ui.graphicsView, canvasWidth, canvasHeight);
    seekablePreview = new flySeekablePreview(this,canvasWidth, canvasHeight, videoStream, canvas, ui.horizontalSlider);
}
/**
 * 
 * @return 
 */
uint32_t Ui_seekablePreviewWindow::frameIndex()
{
    return seekablePreview->sliderGet();
}

/**
  * 
 * @param value
 */
void Ui_seekablePreviewWindow::sliderChanged(int value)
{
    seekablePreview->sliderChanged();
}

// EOF

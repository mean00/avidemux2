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

Ui_seekablePreviewWindow::Ui_seekablePreviewWindow(AVDMGenericVideoStream *videoStream, uint32_t defaultFrame) : QDialog()
{
	ui.setupUi(this);

	uint32_t canvasWidth = videoStream->getInfo()->width;
	uint32_t canvasHeight = videoStream->getInfo()->height;

	canvas = new ADM_QCanvas(ui.frame, canvasWidth, canvasHeight);
	seekablePreview = new flySeekablePreview(canvasWidth, canvasHeight, videoStream, canvas, ui.horizontalSlider);

	seekablePreview->process();
	seekablePreview->sliderSet(defaultFrame);
	seekablePreview->sliderChanged();

	connect(ui.okButton, SIGNAL(clicked(bool)), this, SLOT(closeWindow()));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
}

Ui_seekablePreviewWindow::~Ui_seekablePreviewWindow()
{
	delete seekablePreview;
	delete canvas;
}

void Ui_seekablePreviewWindow::closeWindow()
{
	close();
}

void Ui_seekablePreviewWindow::sliderChanged(int value)
{
	seekablePreview->sliderChanged();
}
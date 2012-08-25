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
Ui_seekablePreviewWindow::Ui_seekablePreviewWindow(QWidget *parent, ADM_coreVideoFilter *videoStream, uint32_t defaultFrame) : QDialog(parent)
{
	ui.setupUi(this);

	seekablePreview = NULL;
	canvas = NULL;

	resetVideoStream(videoStream);
	seekablePreview->sliderSet(defaultFrame);
	seekablePreview->sliderChanged();

	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(ui.next,SIGNAL(clicked()),this,SLOT(nextImage()));
}

Ui_seekablePreviewWindow::~Ui_seekablePreviewWindow()
{
	delete seekablePreview;
	delete canvas;
}
void Ui_seekablePreviewWindow::nextImage(void)
{
    seekablePreview->nextImage();
}
void Ui_seekablePreviewWindow::resetVideoStream(ADM_coreVideoFilter *videoStream)
{
	if (seekablePreview)
		delete seekablePreview;

	if (canvas)
		delete canvas;

	uint32_t canvasWidth = videoStream->getInfo()->width;
	uint32_t canvasHeight = videoStream->getInfo()->height;

	canvas = new ADM_QCanvas(ui.frame, canvasWidth, canvasHeight);
	canvas->show();
	seekablePreview = new flySeekablePreview(canvasWidth, canvasHeight, videoStream, canvas, ui.horizontalSlider);	
    seekablePreview->setCookieFunc(setCurrentPtsCallback,this);
//#warning FIXME NO UPDATE
//	seekablePreview->process();
	seekablePreview->sliderChanged();
}

void Ui_seekablePreviewWindow::sliderChanged(int value)
{
	seekablePreview->sliderChanged();
}

uint32_t Ui_seekablePreviewWindow::frameIndex()
{
	return seekablePreview->sliderGet();
}
/**
    \fn setCurrentPtsCallback
    \brief callback so that the flyDialog can update its father widget
*/
bool Ui_seekablePreviewWindow::setCurrentPtsCallback(void *cookie,uint64_t pts)
{
    if(cookie)
    {
        return ((Ui_seekablePreviewWindow *)cookie)->setTime(pts);
    }
    printf("No cookie, New PTS :%"PRId64" us\n",pts);
    return true;
}
/**
    \fn setTime
    \brief Set timecode
*/
bool      Ui_seekablePreviewWindow::setTime(uint64_t timestamp)
{
    const char *s=ADM_us2plain(timestamp);
    ui.label->setText(s);
    return true;
}
// EOF
/***************************************************************************
                            Q_seekablePreview.h
                            -------------------

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

#include "ADM_cpp.h"
#include "ui_seekablePreview.h"

#include "ADM_assert.h"
#include "DIA_flyDialogQt4.h"
#include "../ADM_dialog/DIA_flyPreview.h"
#include <QTimer>

class Ui_seekablePreviewWindow : public QDialog
{
	Q_OBJECT
protected:
    static bool setCurrentPtsCallback(void *cookie,uint64_t pts);
public:
	ADM_QCanvas         *canvas;
        uint64_t            lastPts;
        QTimer             timer;
	flySeekablePreview *seekablePreview;
	Ui_seekablePreviewDialog ui;
        
public:        
                Ui_seekablePreviewWindow(QWidget *parent, ADM_coreVideoFilter *videoStream, uint32_t defaultFrame = 0);
                ~Ui_seekablePreviewWindow();
	void    resetVideoStream(ADM_coreVideoFilter *videoStream);
	uint32_t frameIndex();
        bool     setTime(uint64_t timestamp);

public slots:
	void sliderChanged(int value);
        bool nextImage(void);
        void backOneMinute(void);
        void fwdOneMinute(void);
        void play(bool status);
        void timeout();
};

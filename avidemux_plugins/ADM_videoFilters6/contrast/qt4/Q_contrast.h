/***************************************************************************
                          ADM_vidContrast.cpp  -  description
                             -------------------
    begin                : Sun Sep 22 2002
    copyright            : (C) 2002 by mean
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
#pragma once
#include "ui_contrast.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyContrast.h"
#include "QGraphicsScene"
/**
    \class Ui_contrastWindow
*/
class Ui_contrastWindow : public QDialog
{
	Q_OBJECT

private:
        void setDialTitles(void);
        void resizeEvent(QResizeEvent *event);
        void showEvent(QShowEvent *event);

protected : 
	int lock;
        QGraphicsScene *scene;
        bool            previewState;

public:
	flyContrast *myCrop;
	ADM_QCanvas *canvas;
	Ui_contrastWindow(QWidget* parent, contrast *param,ADM_coreVideoFilter *in);
	~Ui_contrastWindow();
	Ui_contrastDialog ui;

public slots:
	void gather(contrast *param);

private slots:
	void sliderUpdate(int foo);
	void valueChanged(int foo);
        void previewActivated(int a);
        void dvd2PC();
};

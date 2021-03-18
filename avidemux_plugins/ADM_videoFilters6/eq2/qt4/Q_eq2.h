/***************************************************************************
   \file Q_eq2
   \brief Qt4 ui for eq2 filter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef Q_eq2_h
#define Q_eq2_h
#include "DIA_flyDialogQt4.h"
#include "ui_eq2.h"
#include "ADM_image.h"

#include "ADM_vidEq2.h"
#include "DIA_flyEq2.h"
#include "QGraphicsScene"
/**
    \class Ui_eq2Window
*/
class Ui_eq2Window : public QDialog
{
    Q_OBJECT

protected:
    int lock;
    QGraphicsScene *scene;

public:
    flyEq2 *myCrop;
    ADM_QCanvas *canvas;
    Ui_eq2Window(QWidget *parent, eq2 *param,ADM_coreVideoFilter *in);
    ~Ui_eq2Window();
    Ui_eq2Dialog ui;
    static const int initialValues[];

public slots:
    void gather(eq2 *param);

private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void resetSlider(QWidget *control);

private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void setResetSliderEnabledState(void);
};
#endif	// Q_eq2_h

#pragma once
#include "ui_blackenBorders.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyBlackenBorders.h"
#include "blackenBorder.h"
class Ui_blackenWindow : public QDialog
{
    Q_OBJECT

protected:
    int                 lock;
    int                 inputWidth, inputHeight;
    flyBlacken          *myBlacken;
    ADM_QCanvas         *canvas;
    Ui_blackenDialog    ui;

public:
    Ui_blackenWindow(QWidget* parent, blackenBorder *param,ADM_coreVideoFilter *in);
    ~Ui_blackenWindow();

public slots:
    void gather(blackenBorder *param);

private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(bool f);
    void toggleRubber(int checkState);

private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};



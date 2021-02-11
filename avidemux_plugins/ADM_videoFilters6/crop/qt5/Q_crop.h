#ifndef Q_crop_h
#define Q_crop_h
#include <QPushButton>
#include "ui_crop.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyCrop.h"
#include "crop.h"



#if 0
    #define aprintf ADM_info
#else
    #define aprintf(...) {}
#endif

class Ui_cropWindow : public QDialog
{
    Q_OBJECT

private:
    int             lock;
    int             inputWidth,inputHeight;
    flyCrop         *myCrop;
    ADM_QCanvas     *canvas;
    Ui_cropDialog   ui;
    QPushButton     *pushButtonAutoCrop;

    void updateRightBottomSpinners(int foo, bool useHeightAsRef);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void applyAspectRatio(void);

public:
    Ui_cropWindow(QWidget* parent, crop *param, ADM_coreVideoFilter *in);
    ~Ui_cropWindow();

public slots:
    void gather(crop *param);

private slots:
    void sliderUpdate(int foo);
    void widthChanged(int foo);
    void heightChanged(int foo);
    void autoCrop(bool f);
    void reset(bool f);
    void toggleRubber(int checkState);
    void changeARSelect(int f);
};

#endif	// Q_crop_h

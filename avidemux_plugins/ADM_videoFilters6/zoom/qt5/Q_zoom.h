#ifndef Q_zoom_h
#define Q_zoom_h
#include <QPushButton>
#include "ui_zoom.h"
#include "ADM_image.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyZoom.h"
#include "zoom.h"



#if 0
    #define aprintf ADM_info
#else
    #define aprintf(...) {}
#endif

class Ui_zoomWindow : public QDialog
{
    Q_OBJECT

private:
    int             lock;
    int             inputWidth,inputHeight;
    flyZoom         *myFly;
    ADM_QCanvas     *canvas;
    Ui_zoomDialog   ui;

    void updateRightBottomSpinners(int foo, bool useHeightAsRef);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void applyAspectRatio(void);

public:
    Ui_zoomWindow(QWidget* parent, zoom *param, ADM_coreVideoFilter *in);
    ~Ui_zoomWindow();

public slots:
    void gather(zoom *param);

private slots:
    void sliderUpdate(int foo);
    void widthChanged(int foo);
    void heightChanged(int foo);
    void reset(bool f);
    void toggleRubber(int checkState);
    void changeARSelect(int f);
};

#endif	// Q_zoom_h

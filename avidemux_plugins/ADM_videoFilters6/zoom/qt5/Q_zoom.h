#ifndef Q_zoom_h
#define Q_zoom_h
#include <QPushButton>
#include "ui_zoom.h"
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
    bool            shown;
    int             lock;
    int             inputWidth,inputHeight;
    flyZoom         *myFly;
    ADM_QCanvas     *canvas;
    Ui_zoomDialog   ui;
    QPushButton     *preferencesButton;

    void updateRightBottomSpinners(int foo, bool useHeightAsRef);
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void applyAspectRatio(void);

public:
    Ui_zoomWindow(QWidget* parent, zoom *param, bool firstRun, ADM_coreVideoFilter *in);
    ~Ui_zoomWindow();

    bool rubberIsHidden(void);

public slots:
    void gather(zoom *param);

private slots:
    void sliderUpdate(int foo);
    void widthChanged(int foo);
    void heightChanged(int foo);
    void reset(bool f);
    void toggleRubber(int checkState);
    void changeARSelect(int f);
    void setPreferences(bool f);
};

#endif	// Q_zoom_h

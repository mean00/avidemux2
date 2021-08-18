#ifndef Q_fadeInOut_h
#define Q_fadeInOut_h
#include "ui_fadeInOut.h"
#include "fadeInOut.h"
#include "DIA_flyFadeInOut.h"

class Ui_fadeInOutWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    uint64_t markerA, markerB, duration;
    flyFadeInOut *myFly;
    ADM_QCanvas *canvas;
    Ui_fadeInOutDialog ui;

  public:
    Ui_fadeInOutWindow(QWidget *parent, fadeInOut *param,ADM_coreVideoFilter *in);
    ~Ui_fadeInOutWindow();

  public slots:
    void gather(fadeInOut *param);

  private slots:
    void sliderUpdate(int foo);
    void manualTimeEntry(bool f);
    void timesFromMarkers(bool f);
    void pushedColor();
    void reset(bool f);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_fadeInOut_h

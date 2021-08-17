#ifndef Q_fadeThrough_h
#define Q_fadeThrough_h
#include "ui_fadeThrough.h"
#include "fadeThrough.h"
#include "DIA_flyFadeThrough.h"
#include <QGraphicsScene>

class Ui_fadeThroughWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    uint64_t markerA, markerB, duration;
    flyFadeThrough *myFly;
    ADM_QCanvas *canvas;
    Ui_fadeThroughDialog ui;
    QGraphicsScene *scene;

  public:
    Ui_fadeThroughWindow(QWidget *parent, fadeThrough *param,ADM_coreVideoFilter *in);
    ~Ui_fadeThroughWindow();

  public slots:
    void gather(fadeThrough *param);

  private slots:
    void sliderUpdate(int foo);
    void manualTimeEntry(bool f);
    void timesFromMarkers(bool f);
    void centeredTimesFromMarkers(bool f);
    void tabChanged(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double f);
    void pushedColorBlend();
    void pushedColorVignette();
    void reset(bool f);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_fadeThrough_h

#ifndef Q_waveletSharp_h
#define Q_waveletSharp_h
#include "ui_waveletSharp.h"
#include "waveletSharp.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyWaveletSharp.h"

class Ui_waveletSharpWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyWaveletSharp *     myFly;
    ADM_QCanvas *      canvas;
    Ui_waveletSharpDialog ui;

  public:
    Ui_waveletSharpWindow(QWidget *parent, waveletSharp *param,ADM_coreVideoFilter *in);
    ~Ui_waveletSharpWindow();

  public slots:
    void gather(waveletSharp *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_waveletSharp_h

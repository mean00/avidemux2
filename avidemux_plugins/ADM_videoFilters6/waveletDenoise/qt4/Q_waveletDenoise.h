#ifndef Q_waveletDenoise_h
#define Q_waveletDenoise_h
#include "ui_waveletDenoise.h"
#include "waveletDenoise.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyWaveletDenoise.h"

class Ui_waveletDenoiseWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyWaveletDenoise *     myFly;
    ADM_QCanvas *      canvas;
    Ui_waveletDenoiseDialog ui;

  public:
    Ui_waveletDenoiseWindow(QWidget *parent, waveletDenoise *param,ADM_coreVideoFilter *in);
    ~Ui_waveletDenoiseWindow();

  public slots:
    void gather(waveletDenoise *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void valueChangedSpinBox(double foo);
    void reset(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_waveletDenoise_h

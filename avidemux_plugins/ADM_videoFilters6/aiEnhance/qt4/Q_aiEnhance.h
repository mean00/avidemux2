#ifndef Q_aiEnhance_h
#define Q_aiEnhance_h
#include "ui_aiEnhance.h"
#include "ADM_image.h"
#include "aiEnhance.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyAiEnhance.h"

class Ui_aiEnhanceWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    QPushButton *      peekOriginalBtn;

  public:
    flyAiEnhance *myFly;
    ADM_QCanvas *canvas;
    Ui_aiEnhanceWindow(QWidget *parent, aiEnhance *param,ADM_coreVideoFilter *in);
    ~Ui_aiEnhanceWindow();
    Ui_aiEnhanceDialog ui;

  public slots:
    void gather(aiEnhance *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void peekOriginalPressed(void);
    void peekOriginalReleased(void);
};
#endif    // Q_aiEnhance_h
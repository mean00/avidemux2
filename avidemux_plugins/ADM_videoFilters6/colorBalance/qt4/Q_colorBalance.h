#ifndef Q_colorBalance_h
#define Q_colorBalance_h
#include "ui_colorBalance.h"
#include "colorBalance.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyColorBalance.h"

class Ui_colorBalanceWindow : public QDialog
{
    Q_OBJECT

  protected:
    int lock;
    flyColorBalance *     myFly;
    ADM_QCanvas *      canvas;
    Ui_colorBalanceDialog ui;

  public:
    Ui_colorBalanceWindow(QWidget *parent, colorBalance *param,ADM_coreVideoFilter *in);
    ~Ui_colorBalanceWindow();
    static void rgb2yuv(int * yuv, int * rgb);
    static void yuv2rgb(int * rgb, int * yuv);
    static void setHueColor(QDial * d, int angle);
    static int  getHueColor(QDial * d);

  public slots:
    void gather(colorBalance *param);

  private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);
    void peekPressed(void);
    void peekReleased(void);
    void rangesPressed(void);
    void rangesReleased(void);

  private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif    // Q_colorBalance_h

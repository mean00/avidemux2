#ifndef Q_hue_h
#define Q_hue_h
#include "ui_hue.h"
#include "ADM_image.h"
#include "hue.h"
#include "DIA_flyDialogQt4.h"
#include "DIA_flyHue.h"

class Ui_hueWindow : public QDialog
{
    Q_OBJECT

protected:
    int lock;
    flyHue *myCrop;
    ADM_QCanvas *canvas;
    Ui_hueDialog ui;
public:
    Ui_hueWindow(QWidget *parent, hue *param,ADM_coreVideoFilter *in);
    ~Ui_hueWindow();

public slots:
    void gather(hue *param);

private slots:
    void sliderUpdate(int foo);
    void valueChanged(int foo);
    void reset(void);

private:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
};
#endif	// Q_hue_h

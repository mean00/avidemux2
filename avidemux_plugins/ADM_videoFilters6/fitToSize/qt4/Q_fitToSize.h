#ifndef Q_fitToSize_h
#define Q_fitToSize_h

#include "ui_fitToSize.h"
#include "ADM_default.h"
#include "fitToSize.h"

typedef struct resParam
{
    uint32_t originalWidth, originalHeight;
    fitToSize rsz;
} resParam;

class fitToSizeWindow : public QDialog
{
    Q_OBJECT

private:
    void connectDimensionControls();
    void disconnectDimensionControls();
    void roundUp();
    void printInfo();

protected:
    resParam *_param;

public:
    fitToSizeWindow(QWidget *parent, resParam *param);
    Ui_fitToSizeDialog ui;

public slots:
    void gather(void);
    void okButtonClicked();
    void sliderChanged(int value);
    void percentageSpinBoxChanged(int value);
    void dimensionSpinBoxChanged(int value);
    void roundupChanged(int index);
};
#endif // Q_fitToSize_h

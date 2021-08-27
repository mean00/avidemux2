#ifndef Q_reverse_h
#define Q_reverse_h

#include "ui_reverse.h"
#include "ADM_default.h"
#include "reverse.h"
#include "ADM_vidReverse.h"

class reverseWindow : public QDialog
{
    Q_OBJECT

private:
    void printInfo();
    void showEvent(QShowEvent *event);

protected:
    reverse * _param;
    uint64_t markerA, markerB, duration;
    double bytesPerSec;
    std::string filePath;

public:
    reverseWindow(QWidget *parent, reverse *param, ADM_coreVideoFilter *in);
    Ui_reverseDialog ui;

public slots:
    void gather(void);
    void okButtonClicked();
    void manualTimeEntry(bool f);
    void timesFromMarkers(bool f);
    void valueChanged(int f);
};
#endif // Q_reverse_h

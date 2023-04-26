#ifndef QT_TOOLKIT_H
#define QT_TOOLKIT_H
#include <QDialog>
#include <QSlider>
#include "ADM_UIQT46_export.h"
#include "ADM_inttype.h"

/* Since Qt 6.3.0, accepting or rejecting a video filter preview dialog
by pressing "OK" or "Cancel" button of the QDialogButtonBox crashes Avidemux.
macOS builds are not affected.

As a workaround, disconnect PMF-style connections to these slots and reconnect
them the old way.

Probably related to https://bugreports.qt.io/browse/QTBUG-33908 */
#if !(defined(__APPLE__) && QT_VERSION >= QT_VERSION_CHECK(6,3,0))
    #define QT6_CRASH_WORKAROUND(desc) \
    disconnect(ui.buttonBox, &QDialogButtonBox::accepted, this, &Ui_ ## desc ::accept); \
    disconnect(ui.buttonBox, &QDialogButtonBox::rejected, this, &Ui_ ## desc ::reject); \
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept())); \
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#else
    #define QT6_CRASH_WORKAROUND(desc) {}
#endif

ADM_UIQT46_EXPORT void qtRegisterDialog(QWidget *dialog);
ADM_UIQT46_EXPORT void qtUnregisterDialog(QWidget *dialog);
ADM_UIQT46_EXPORT QWidget* qtLastRegisteredDialog();
ADM_UIQT46_EXPORT uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);

/**
    \brief Override Qt-style-dependent behaviour of the navigation slider
*/
class ADM_UIQT46_EXPORT ADM_QSlider : public QSlider
{
public:
    ADM_QSlider(QWidget *parent);
private:
    void mousePressEvent(QMouseEvent *e);
};
/**
    \brief QSlider with value indicator in a tooltip
*/
class ADM_UIQT46_EXPORT ADM_SliderIndicator : public QSlider
{
public:
    ADM_SliderIndicator(QWidget *parent);
    void setScale(int num, int den, int precision = 0);
private:
    int _scaleNum;
    int _scaleDen;
    int _precision;
    virtual void sliderChange(SliderChange change);
};

/**
    \brief Specific navigation slider for flyDialogs
*/
class ADM_UIQT46_EXPORT ADM_flyNavSlider : public ADM_QSlider
{
protected:
    bool _invertWheel;
    uint64_t totalDuration, markerATime, markerBTime;

    void drawSelection(void);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void paintEvent(QPaintEvent *event);
            bool isDarkMode();

public:
    ADM_flyNavSlider(QWidget *parent);
    void setInvertedWheel(bool inverted);
    void setMarkers(uint64_t totalDuration, uint64_t markerATime, uint64_t markerBTime);
};

#endif

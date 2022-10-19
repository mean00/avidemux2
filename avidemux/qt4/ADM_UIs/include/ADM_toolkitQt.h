#ifndef QT_TOOLKIT_H
#define QT_TOOLKIT_H
#include <QWidget>
#include <QSlider>
#include "ADM_UIQT46_export.h"
#include "ADM_inttype.h"

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
    bool    isDarkMode();
private:
    void    mousePressEvent(QMouseEvent *e);
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
public:
    ADM_flyNavSlider(QWidget *parent);
    void wheelEvent(QWheelEvent *e);
    void setInvertedWheel(bool inverted);
    void setMarkers(uint64_t totalDuration, uint64_t markerATime, uint64_t markerBTime);
    void paintEvent(QPaintEvent *event);

};

#endif

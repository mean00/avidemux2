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
private:
    void mousePressEvent(QMouseEvent *e);
};
#endif

#ifndef QT_TOOLKIT_H
#define QT_TOOLKIT_H
#include <QWidget>
#include "ADM_UIQT46_export.h"
#include "ADM_inttype.h"

ADM_UIQT46_EXPORT void qtRegisterDialog(QWidget *dialog);
ADM_UIQT46_EXPORT void qtUnregisterDialog(QWidget *dialog);
ADM_UIQT46_EXPORT QWidget* qtLastRegisteredDialog();
ADM_UIQT46_EXPORT uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w,uint32_t *h);
#endif

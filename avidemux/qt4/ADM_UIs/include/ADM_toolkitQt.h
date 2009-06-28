#ifndef QT_TOOLKIT_H
#define QT_TOOLKIT_H
#include <QtGui/QWidget>

void qtRegisterDialog(QWidget *dialog);
void qtUnregisterDialog(QWidget *dialog);
QWidget* qtLastRegisteredDialog();
#endif

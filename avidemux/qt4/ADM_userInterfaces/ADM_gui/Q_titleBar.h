#ifndef Q_titleBar_h
#define Q_titleBar_h

#include <QStyle>
#include <QDockWidget>
#include "ui_titleBar.h"

/**
    \class TitleBar
*/
class TitleBar : public QWidget
{
    Q_OBJECT

public:
    TitleBar(QString title = "");
    virtual ~TitleBar();

protected:
    void showEvent(QShowEvent *e);

private:
    Ui_TitleBar ui;

private slots:
    void dockParent(void);
    void floatParent(void);
    void closeParent(void);
};
#endif	// Q_titleBar_h

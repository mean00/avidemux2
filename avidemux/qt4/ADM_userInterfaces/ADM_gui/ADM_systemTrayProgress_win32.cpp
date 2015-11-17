#pragma once
#include <QtWinExtras/QtWinExtras>
#include <QMainWindow>

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_systemTrayProgress.h"
/**
 */
class winTaskBarProgress : public admUITaskBarProgress
{
    public:
    winTaskBarProgress(QMainWindow *w)
    {
          button = new QWinTaskbarButton(w);
    }
    virtual ~winTaskBarProgress()
    {
        delete button;
        button=NULL;
    }
    bool onoff(bool v)
    {
        QWinTaskbarProgress *progress = button->progress();
        progress->setVisible(v);
        return true;

    }
    virtual bool enable() 
    {
        return onoff(true);
    }
    virtual bool disable() 
    {
        return onoff(false);
    }
    virtual bool setProgress(int percent) 
    {
        QWinTaskbarProgress *progress = button->progress();
        progress->setValue(percent);
    } 
    QWinTaskbarButton *button;
};

/**
 */
admUITaskBarProgress *createADMTaskBarProgress(void *parent)
{
        QMainWindow *win=( QMainWindow *)parent;
        return new winTaskBarProgress(win);
}
// EOF

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
          progress = button->progress();
    }
    virtual ~winTaskBarProgress()
    {
        progress=NULL;
        delete button;
        button=NULL;
    }    
    virtual bool enable() 
    {
        progress->show();
        progress->setVisible(true);
        return true;
    }
    virtual bool disable() 
    {
        progress->show();
        progress->setVisible(false);
        return true;
    }
    virtual bool setProgress(int percent) 
    {
        progress->setValue(percent);
    } 
    QWinTaskbarButton *button;
    QWinTaskbarProgress *progress;
};

/**
 */
admUITaskBarProgress *createADMTaskBarProgress(void *parent)
{
        QMainWindow *win=( QMainWindow *)parent;
        return new winTaskBarProgress(win);
}
// EOF
